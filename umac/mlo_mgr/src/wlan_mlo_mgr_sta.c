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
 * DOC: contains MLO manager STA related api's
 */

#include <wlan_cmn.h>
#include <wlan_mlo_mgr_sta.h>
#include <wlan_cm_public_struct.h>
#include <wlan_mlo_mgr_main.h>
#include <wlan_cm_api.h>
#include <wlan_mlo_mgr_cmn.h>
#include <wlan_scan_api.h>

#ifdef WLAN_FEATURE_11BE_MLO

/*
 * mlo_get_assoc_link_vdev - API to get assoc link vdev
 *
 * @mlo_dev_ctx: mlo dev ctx
 *
 * Return: MLD assoc link vdev
 */
static inline struct wlan_objmgr_vdev *
mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	uint8_t i = 0;

	if (!wlan_vdev_mlme_is_mlo_vdev(vdev))
		return NULL;

	if (!mlo_dev_ctx)
		return NULL;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (!wlan_vdev_mlme_is_mlo_link_vdev(mlo_dev_ctx->wlan_vdev_list[i]))
			return mlo_dev_ctx->wlan_vdev_list[i];
	}
	return NULL;
}

struct wlan_objmgr_vdev *
ucfg_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	return mlo_get_assoc_link_vdev(vdev);
}

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static QDF_STATUS
mlo_validate_connect_req(struct wlan_mlo_dev_context *mlo_dev_ctx,
			 struct wlan_cm_connect_req *req)
{
/* MCC: check back to back connect handling */
	return QDF_STATUS_SUCCESS;
}
#else
/**
 * mlo_is_mld_connected - Check whether MLD is connected
 *
 * @vdev: pointer to vdev
 *
 * Return: true if mld is connected, false otherwise
 */
static inline
bool mlo_is_mld_connected(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return true;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links)) {
			if (!wlan_cm_is_vdev_connected(mlo_dev_ctx->wlan_vdev_list[i]))
				return false;
		}
	}
	return true;
}

bool ucfg_mlo_is_mld_connected(struct wlan_objmgr_vdev *vdev)
{
	return mlo_is_mld_connected(vdev);
}

/**
 * mlo_is_mld_disconnected - Check whether MLD is disconnected
 *
 * @vdev: pointer to vdev
 *
 * Return: true if mld is disconnected, false otherwise
 */
static inline
bool mlo_is_mld_disconnected(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return true;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (!wlan_cm_is_vdev_disconnected(mlo_dev_ctx->wlan_vdev_list[i]))
			return false;
	}
	return true;
}

bool ucfg_mlo_is_mld_disconnected(struct wlan_objmgr_vdev *vdev)
{
	return mlo_is_mld_disconnected(vdev);
}

static void
mlo_cm_handle_connect_in_disconnection_state(struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_req *req)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = mlo_dev_ctx->sta_ctx;

	if (!sta_ctx->connect_req)
		sta_ctx->connect_req = qdf_mem_malloc(
					sizeof(struct wlan_cm_connect_req));

	if (sta_ctx->connect_req)
		qdf_mem_copy(sta_ctx->connect_req, req,
			     sizeof(struct wlan_cm_connect_req));
	else
		qdf_err("Failed to allocate connect req");
}

static void
mlo_cm_handle_connect_in_connection_state(struct wlan_objmgr_vdev *vdev,
					  struct wlan_cm_connect_req *req)
{
	mlo_disconnect_no_lock(vdev, CM_SB_DISCONNECT,
			       REASON_UNSPEC_FAILURE, NULL);
	mlo_cm_handle_connect_in_disconnection_state(vdev, req);
}

static QDF_STATUS mlo_osif_validate_connect_req(struct wlan_objmgr_vdev *vdev)
{
	//mlo_connect: add osif CB to perform operation
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
mlo_validate_connect_req(struct wlan_mlo_dev_context *mlo_dev_ctx,
			 struct wlan_cm_connect_req *req)
{
	uint8_t i = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	// Handle connect in various states
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if ((wlan_cm_is_vdev_connected(mlo_dev_ctx->wlan_vdev_list[i])) ||
		    (wlan_cm_is_vdev_connecting(mlo_dev_ctx->wlan_vdev_list[i])) ||
		    (wlan_cm_is_vdev_roaming(mlo_dev_ctx->wlan_vdev_list[i]))) {
			mlo_cm_handle_connect_in_connection_state(mlo_dev_ctx->wlan_vdev_list[i], req);
			return QDF_STATUS_E_BUSY;
		} else if (wlan_cm_is_vdev_disconnecting(mlo_dev_ctx->wlan_vdev_list[i])) {
			mlo_cm_handle_connect_in_disconnection_state(mlo_dev_ctx->wlan_vdev_list[i], req);
			return QDF_STATUS_E_BUSY;
		}

		/*
		 * mlo_connect: update wlan_connect_req_links in
		 * wlan_cfg80211_conect on osif_cm_connect,
		 * Validate pre checks for connection
		 */
		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connect_req_links)) {
			status = mlo_osif_validate_connect_req(mlo_dev_ctx->wlan_vdev_list[i]);
			if (status != QDF_STATUS_SUCCESS)
				return status;
		}
	}
	return status;
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (mlo_dev_ctx && wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		mlo_dev_lock_acquire(mlo_dev_ctx);
		status = mlo_validate_connect_req(mlo_dev_ctx, req);
		if (QDF_IS_STATUS_SUCCESS(status))
			status = wlan_cm_start_connect(vdev, req);

		mlo_dev_lock_release(mlo_dev_ctx);
		return status;
	}

	return wlan_cm_start_connect(vdev, req);
}

/**
 * mlo_prepare_and_send_connect- Prepare and send the connect req
 *
 * @vdev: vdev pointer
 * @ml_parnter_info: ml partner link info
 * @link_info: link info on which connect req will be sent
 * @ssid: ssid to connect
 *
 * Return: none
 */
static void
mlo_prepare_and_send_connect(struct wlan_objmgr_vdev *vdev,
			     struct mlo_partner_info ml_parnter_info,
			     struct mlo_link_info link_info,
			     struct wlan_ssid ssid)
{
	struct wlan_cm_connect_req req = {0};

	mlo_debug("Partner link connect mac:" QDF_MAC_ADDR_FMT "vdev_id:%d",
		  QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		  wlan_vdev_get_id(vdev));
	qdf_mem_copy(req.bssid.bytes,
		     link_info.link_addr.bytes,
		     QDF_MAC_ADDR_SIZE);

	qdf_mem_copy(&req.ml_parnter_info,
		     &ml_parnter_info,
		     sizeof(struct mlo_partner_info));

	req.ssid.length = ssid.length;
	qdf_mem_copy(req.ssid.ssid, ssid.ssid,
		     ssid.length);

	wlan_cm_start_connect(vdev, &req);
}

/**
 * mlo_send_link_connect- Create/Issue the connection on secondary link
 *
 * @vdev: vdev pointer
 * @mlo_dev_ctx: ml dev context
 * @assoc_rsp: assoc response
 * @ml_parnter_info: ml partner link info
 *
 * Return: none
 */
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static void
mlo_send_link_connect(struct wlan_objmgr_vdev *vdev,
		      struct wlan_mlo_dev_context *mlo_dev_ctx,
		      struct element_info *assoc_rsp,
		      struct mlo_partner_info ml_parnter_info)
{
	/* Create the secondary interface, Send keys if the last link */
	uint8_t i;
	struct wlan_ssid ssid = {0};

	mlo_debug("Sending link connect on partner interface");
	wlan_vdev_mlme_get_ssid(
			vdev, ssid.ssid,
			&ssid.length);

	if (!ml_parnter_info.num_partner_links) {
		mlo_err("No patner info in connect resp");
		return;
	}

	if(wlan_vdev_mlme_is_mlo_link_vdev(vdev))
		return;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i] ||
		    (mlo_dev_ctx->wlan_vdev_list[i] == vdev))
			continue;
		wlan_vdev_mlme_feat_ext2_cap_set(mlo_dev_ctx->wlan_vdev_list[i],
						 WLAN_VDEV_FEXT2_MLO_STA_LINK);
		mlo_prepare_and_send_connect(
				mlo_dev_ctx->wlan_vdev_list[i],
				ml_parnter_info,
				ml_parnter_info.partner_link_info[i],
				ssid);
	}
}
#else
static void
mlo_send_link_connect(struct wlan_objmgr_vdev *vdev,
		      struct wlan_mlo_dev_context *mlo_dev_ctx,
		      struct element_info *assoc_rsp,
		      struct mlo_partner_info ml_parnter_info)
{
	struct wlan_ssid ssid = {0};
	uint8_t i = 0;
	uint8_t j = 0;

	if (!ml_parnter_info.num_partner_links) {
		mlo_err("No patner info in connect resp");
		return;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	if (wlan_cm_is_vdev_connected(vdev)) {
		for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
			if (!mlo_dev_ctx->wlan_vdev_list[i])
				continue;
			/*
			 * mlo_connect: update wlan_connected_links bitmap from
			 * assoc resp parsing
			 */
			if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links)) {
				if (wlan_cm_is_vdev_disconnected(
					mlo_dev_ctx->wlan_vdev_list[i])) {
					for (j = 0; j < ml_parnter_info.num_partner_links; j++) {
						if (mlo_dev_ctx->wlan_vdev_list[i]->vdev_mlme.mlo_link_id ==
							ml_parnter_info.partner_link_info[j].link_id)
							break;
					}
					if (j < ml_parnter_info.num_partner_links) {
						wlan_vdev_mlme_get_ssid(
							vdev, ssid.ssid,
							&ssid.length);
						mlo_prepare_and_send_connect(
							mlo_dev_ctx->wlan_vdev_list[i],
							ml_parnter_info,
							ml_parnter_info.partner_link_info[j],
							ssid);
					}
					mlo_dev_lock_release(mlo_dev_ctx);
					return;
				}
			}
		}
	}
	mlo_dev_lock_release(mlo_dev_ctx);
}
#endif

void mlo_sta_link_connect_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_resp *rsp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;

	if (mlo_dev_ctx && wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		mlo_debug("Vdev: %d", wlan_vdev_get_id(vdev));
		if (wlan_cm_is_vdev_disconnected(vdev)) {
			// Connect Failure
			if (vdev == wlan_mlo_get_assoc_link_vdev(vdev)) {
				return;
			} else {
				if (rsp->reason == CM_NO_CANDIDATE_FOUND ||
				    rsp->reason == CM_HW_MODE_FAILURE ||
				    rsp->reason == CM_SER_FAILURE)
					mlo_disconnect_no_lock(
						vdev, CM_OSIF_MLO_DISCONNECT,
						REASON_UNSPEC_FAILURE, NULL);
				else
					mlo_disconnect(
						vdev, CM_OSIF_MLO_DISCONNECT,
						REASON_UNSPEC_FAILURE, NULL);
				return;
			}
		}
		if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
			if (mlo_dev_ctx->sta_ctx->assoc_rsp.ptr) {
				qdf_mem_free(mlo_dev_ctx->sta_ctx->assoc_rsp.ptr);
				mlo_dev_ctx->sta_ctx->assoc_rsp.ptr = NULL;
			}
			mlo_dev_ctx->sta_ctx->assoc_rsp.len =
				rsp->connect_ies.assoc_rsp.len;
			mlo_dev_ctx->sta_ctx->assoc_rsp.ptr =
				qdf_mem_malloc(rsp->connect_ies.assoc_rsp.len);
			if (!mlo_dev_ctx->sta_ctx->assoc_rsp.ptr) {
				QDF_ASSERT(0);
				return;
			}
			qdf_mem_copy(mlo_dev_ctx->sta_ctx->assoc_rsp.ptr,
				     rsp->connect_ies.assoc_rsp.ptr,
				     rsp->connect_ies.assoc_rsp.len);
		}
		mlo_send_link_connect(vdev, mlo_dev_ctx,
				      &rsp->connect_ies.assoc_rsp,
				      rsp->ml_parnter_info);
	}
}

/* STA disconnect */
QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid)
{
/* This API will be pass through if MLO manager/11be is disabled */
	return QDF_STATUS_SUCCESS;
}

void mlo_sta_link_disconn_notify(struct wlan_objmgr_vdev *vdev)
{
}

bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

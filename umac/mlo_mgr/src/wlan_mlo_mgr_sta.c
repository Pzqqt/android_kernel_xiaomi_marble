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
/**
 * mlo_disconnect_no_lock - Start the disconnection process without acquiring
 * ML dev lock
 *
 * @vdev: pointer to vdev
 * @source: source of the request (can be connect or disconnect request)
 * @reason_code: reason for disconnect
 * @bssid: BSSID
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mlo_disconnect_no_lock(struct wlan_objmgr_vdev *vdev,
					 enum wlan_cm_source source,
					 enum wlan_reason_code reason_code,
					 struct qdf_mac_addr *bssid);

static inline void
mlo_allocate_and_copy_ies(struct wlan_cm_connect_req *target,
			  struct wlan_cm_connect_req *source)
{
	target->assoc_ie.ptr = NULL;
	target->scan_ie.ptr = NULL;

	if (source->scan_ie.ptr) {
		target->scan_ie.ptr = qdf_mem_malloc(source->scan_ie.len);
		if (!target->scan_ie.ptr)
			target->scan_ie.len = 0;
		else
			qdf_mem_copy(target->scan_ie.ptr,
				     source->scan_ie.ptr, source->scan_ie.len);
	}

	if (source->assoc_ie.ptr) {
		target->assoc_ie.ptr = qdf_mem_malloc(source->assoc_ie.len);
		if (!target->assoc_ie.ptr)
			target->assoc_ie.len = 0;
		else
			qdf_mem_copy(target->assoc_ie.ptr, source->assoc_ie.ptr,
				     source->assoc_ie.len);
	}
}

static inline void
mlo_free_connect_ies(struct wlan_cm_connect_req *connect_req)
{
	if (connect_req->scan_ie.ptr) {
		qdf_mem_free(connect_req->scan_ie.ptr);
		connect_req->scan_ie.ptr = NULL;
	}

	if (connect_req->assoc_ie.ptr) {
		qdf_mem_free(connect_req->assoc_ie.ptr);
		connect_req->assoc_ie.ptr = NULL;
	}
}

/*
 * mlo_get_assoc_link_vdev - API to get assoc link vdev
 *
 * @mlo_dev_ctx: pointer to mlo dev context
 *
 * Return: MLD assoc link vdev
 */
static inline struct wlan_objmgr_vdev *
mlo_get_assoc_link_vdev(struct wlan_mlo_dev_context *mlo_dev_ctx)
{
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return NULL;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (wlan_vdev_mlme_is_mlo_vdev(mlo_dev_ctx->wlan_vdev_list[i]) &&
		    !wlan_vdev_mlme_is_mlo_link_vdev(mlo_dev_ctx->wlan_vdev_list[i]))
			return mlo_dev_ctx->wlan_vdev_list[i];
	}
	return NULL;
}

struct wlan_objmgr_vdev *
ucfg_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;

	if (!mlo_dev_ctx || !wlan_vdev_mlme_is_mlo_vdev(vdev))
		return NULL;

	return mlo_get_assoc_link_vdev(mlo_dev_ctx);
}

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static QDF_STATUS
mlo_validate_connect_req(struct wlan_objmgr_vdev *vdev,
			 struct wlan_mlo_dev_context *mlo_dev_ctx,
			 struct wlan_cm_connect_req *req)
{
/* check back to back connect handling */
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

	if (!mlo_dev_ctx || !wlan_vdev_mlme_is_mlo_vdev(vdev))
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

	if (!mlo_dev_ctx || !wlan_vdev_mlme_is_mlo_vdev(vdev))
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

	if (sta_ctx->connect_req) {
		qdf_mem_copy(sta_ctx->connect_req, req,
			     sizeof(struct wlan_cm_connect_req));
		mlo_allocate_and_copy_ies(sta_ctx->connect_req, req);
	} else {
		mlo_err("Failed to allocate connect req");
	}
}

static void
mlo_cm_handle_connect_in_connection_state(struct wlan_objmgr_vdev *vdev,
					  struct wlan_cm_connect_req *req)
{
	mlo_disconnect_no_lock(vdev, CM_OSIF_CFG_DISCONNECT,
			       REASON_UNSPEC_FAILURE, NULL);
	mlo_cm_handle_connect_in_disconnection_state(vdev, req);
}

static QDF_STATUS
mlo_validate_connect_req(struct wlan_objmgr_vdev *vdev,
			 struct wlan_mlo_dev_context *mlo_dev_ctx,
			 struct wlan_cm_connect_req *req)
{
	uint8_t i = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!wlan_vdev_mlme_is_mlo_vdev(vdev))
		return QDF_STATUS_SUCCESS;

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
			status = mlo_mlme_validate_conn_req(
					mlo_dev_ctx->wlan_vdev_list[i], NULL);
			if (status != QDF_STATUS_SUCCESS)
				return status;
		}
	}
	return status;
}
#endif

QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx) {
		mlo_dev_lock_acquire(mlo_dev_ctx);
		status = mlo_validate_connect_req(vdev, mlo_dev_ctx, req);

		if (!sta_ctx->orig_conn_req)
			sta_ctx->orig_conn_req = qdf_mem_malloc(
					sizeof(struct wlan_cm_connect_req));
		else
			mlo_free_connect_ies(sta_ctx->orig_conn_req);

		mlo_debug("storing orig connect req");
		if (sta_ctx->orig_conn_req) {
			qdf_mem_copy(sta_ctx->orig_conn_req, req,
				     sizeof(struct wlan_cm_connect_req));
			mlo_allocate_and_copy_ies(sta_ctx->orig_conn_req, req);
		} else {
			mlo_err("Failed to allocate orig connect req");
			return QDF_STATUS_E_NOMEM;
		}

		if (QDF_IS_STATUS_SUCCESS(status)) {
			mlo_clear_connected_links_bmap(vdev);
			status = wlan_cm_start_connect(vdev, req);
		}

		mlo_dev_lock_release(mlo_dev_ctx);
		return status;
	}

	return wlan_cm_start_connect(vdev, req);
}

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static inline void
mlo_update_connect_req_chan_info(struct wlan_cm_connect_req *req)
{ }
#else
static inline void
mlo_update_connect_req_chan_info(struct wlan_cm_connect_req *req)
{
	req->chan_freq = 0;
	req->chan_freq_hint = 0;
}
#endif

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
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = mlo_dev_ctx->sta_ctx;

	mlo_debug("Partner link connect mac:" QDF_MAC_ADDR_FMT " vdev_id:%d",
		  QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		  wlan_vdev_get_id(vdev));

	qdf_mem_copy(&req, sta_ctx->orig_conn_req,
		     sizeof(struct wlan_cm_connect_req));

	mlo_update_connect_req_chan_info(&req);

	qdf_mem_copy(req.bssid.bytes,
		     link_info.link_addr.bytes,
		     QDF_MAC_ADDR_SIZE);

	qdf_mem_copy(&req.ml_parnter_info,
		     &ml_parnter_info,
		     sizeof(struct mlo_partner_info));

	req.ssid.length = ssid.length;
	qdf_mem_copy(&req.ssid.ssid, &ssid.ssid, ssid.length);

	mlo_allocate_and_copy_ies(&req, sta_ctx->orig_conn_req);
	if (!req.assoc_ie.ptr)
		mlo_err("Failed to allocate assoc IEs");

	if (!req.scan_ie.ptr)
		mlo_err("Failed to allocate scan IEs");

	/* Reset crypto auth type for partner link.
	 * It will be set based on partner scan cache entry
	 */
	req.crypto.auth_type = 0;

	wlan_cm_start_connect(vdev, &req);
	mlo_free_connect_ies(&req);
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
						 WLAN_VDEV_FEXT2_MLO);
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

static inline void
mlo_update_connected_links_bmap(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct mlo_partner_info ml_parnter_info)
{
	uint8_t i = 0;
	uint8_t j = 0;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		for (j = 0; j < ml_parnter_info.num_partner_links; j++) {
			if (wlan_vdev_get_link_id(mlo_dev_ctx->wlan_vdev_list[i]) ==
			    ml_parnter_info.partner_link_info[j].link_id)
				mlo_update_connected_links(
					mlo_dev_ctx->wlan_vdev_list[i], 1);
		}
	}
}

void mlo_sta_link_connect_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_resp *rsp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (mlo_dev_ctx) {
		mlo_debug("Vdev: %d", wlan_vdev_get_id(vdev));
		if (wlan_cm_is_vdev_disconnected(vdev)) {
			// Connect Failure
			if (vdev == mlo_get_assoc_link_vdev(mlo_dev_ctx)) {
				sta_ctx = mlo_dev_ctx->sta_ctx;
				if (sta_ctx->orig_conn_req) {
					mlo_free_connect_ies(
							sta_ctx->orig_conn_req);
					qdf_mem_free(sta_ctx->orig_conn_req);
					sta_ctx->orig_conn_req = NULL;
				}
				return;
			} else {
				mlo_update_connected_links(vdev, 0);
				if (rsp->reason == CM_NO_CANDIDATE_FOUND ||
				    rsp->reason == CM_HW_MODE_FAILURE ||
				    rsp->reason == CM_SER_FAILURE)
					mlo_disconnect_no_lock(
						vdev, CM_OSIF_DISCONNECT,
						REASON_UNSPEC_FAILURE, NULL);
				else
					mlo_disconnect(
						vdev, CM_OSIF_DISCONNECT,
						REASON_UNSPEC_FAILURE, NULL);
				return;
			}
		}
		if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
			if (mlo_dev_ctx->sta_ctx->assoc_rsp.ptr) {
				qdf_mem_free(
					mlo_dev_ctx->sta_ctx->assoc_rsp.ptr);
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
			if (rsp->connect_ies.assoc_rsp.ptr)
				qdf_mem_copy(
					mlo_dev_ctx->sta_ctx->assoc_rsp.ptr,
					rsp->connect_ies.assoc_rsp.ptr,
					rsp->connect_ies.assoc_rsp.len);
			mlo_update_connected_links_bmap(mlo_dev_ctx,
							rsp->ml_parnter_info);
		}
		mlo_send_link_connect(vdev, mlo_dev_ctx,
				      &rsp->connect_ies.assoc_rsp,
				      rsp->ml_parnter_info);
	}
}

/*
 * mlo_get_connected_link_vdev - API to get 1st connected link vdev
 *
 * @mlo_dev_ctx: mlo dev ctx
 *
 * Return: 1st connected link vdev in ML dev, NULL if none of the link vdev
 * is connected
 */
static struct wlan_objmgr_vdev
*mlo_get_connected_link_vdev(struct wlan_mlo_dev_context *mlo_dev_ctx)
{
	uint8_t i = 0;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if ((mlo_dev_ctx->wlan_vdev_list[i] !=
		    mlo_get_assoc_link_vdev(mlo_dev_ctx)) &&
		    wlan_cm_is_vdev_connected(mlo_dev_ctx->wlan_vdev_list[i]))
			return mlo_dev_ctx->wlan_vdev_list[i];
	}

	return NULL;
}

/**
 * mlo_send_link_disconnect- Issue the disconnect request on MLD links
 *
 * @mlo_dev_ctx: pointer to mlo dev context
 * @source: disconnect source
 * @reason_code: disconnect reason
 * @bssid: bssid of AP to disconnect, can be null if not known
 *
 * Return: QDF_STATUS
 */
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static QDF_STATUS
mlo_send_link_disconnect(struct wlan_mlo_dev_context *mlo_dev_ctx,
			 enum wlan_cm_source source,
			 enum wlan_reason_code reason_code,
			 struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links) &&
		    mlo_dev_ctx->wlan_vdev_list[i] != mlo_get_assoc_link_vdev(mlo_dev_ctx))
			wlan_cm_disconnect(mlo_dev_ctx->wlan_vdev_list[i],
					   CM_MLO_DISCONNECT, reason_code,
					   NULL);
	}

	wlan_cm_disconnect(mlo_get_assoc_link_vdev(mlo_dev_ctx),
			   source, reason_code, NULL);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
mlo_send_link_disconnect_sync(struct wlan_mlo_dev_context *mlo_dev_ctx,
			      enum wlan_cm_source source,
			      enum wlan_reason_code reason_code,
			      struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (mlo_dev_ctx->wlan_vdev_list[i] !=
				mlo_get_assoc_link_vdev(mlo_dev_ctx))
			wlan_cm_disconnect_sync(mlo_dev_ctx->wlan_vdev_list[i],
						CM_MLO_DISCONNECT, reason_code);
	}

	wlan_cm_disconnect_sync(mlo_get_assoc_link_vdev(mlo_dev_ctx),
				source, reason_code);
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
mlo_send_link_disconnect(struct wlan_mlo_dev_context *mlo_dev_ctx,
			 enum wlan_cm_source source,
			 enum wlan_reason_code reason_code,
			 struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;
	QDF_STATUS status = QDF_STATUS_E_ALREADY;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links)) {
			if (wlan_cm_is_vdev_connecting(
					mlo_dev_ctx->wlan_vdev_list[i]) ||
			    wlan_cm_is_vdev_disconnecting(
					mlo_dev_ctx->wlan_vdev_list[i]) ||
			    wlan_cm_is_vdev_roaming(
					mlo_dev_ctx->wlan_vdev_list[i])) {
				wlan_cm_disconnect(mlo_dev_ctx->wlan_vdev_list[i],
						   source, reason_code, NULL);
				return QDF_STATUS_SUCCESS;
			}
		}
	}

	return status;
}

static QDF_STATUS
mlo_send_link_disconnect_sync(struct wlan_mlo_dev_context *mlo_dev_ctx,
			      enum wlan_cm_source source,
			      enum wlan_reason_code reason_code,
			      struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;
	QDF_STATUS status = QDF_STATUS_E_ALREADY;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links)) {
			if (wlan_cm_is_vdev_connecting(
					mlo_dev_ctx->wlan_vdev_list[i]) ||
			    wlan_cm_is_vdev_disconnecting(
					mlo_dev_ctx->wlan_vdev_list[i]) ||
			    wlan_cm_is_vdev_roaming(
				mlo_dev_ctx->wlan_vdev_list[i])) {
				wlan_cm_disconnect_sync(
					mlo_dev_ctx->wlan_vdev_list[i],
					source, reason_code);
				return QDF_STATUS_SUCCESS;
			}
		}
	}

	return status;
}
#endif

static QDF_STATUS mlo_disconnect_no_lock(struct wlan_objmgr_vdev *vdev,
					 enum wlan_cm_source source,
					 enum wlan_reason_code reason_code,
					 struct qdf_mac_addr *bssid)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_objmgr_vdev *tmp_vdev = vdev;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (mlo_dev_ctx) {
		sta_ctx = mlo_dev_ctx->sta_ctx;
		if (!sta_ctx)
			return QDF_STATUS_E_FAILURE;

		if (sta_ctx->connect_req) {
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}

		if (sta_ctx->orig_conn_req) {
			mlo_free_connect_ies(sta_ctx->orig_conn_req);
			qdf_mem_free(sta_ctx->orig_conn_req);
			sta_ctx->orig_conn_req = NULL;
		}

		status = mlo_send_link_disconnect(mlo_dev_ctx, source,
						  reason_code, bssid);

		if (QDF_IS_STATUS_SUCCESS(status))
			return status;

		tmp_vdev = mlo_get_connected_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect(tmp_vdev, source,
						    reason_code, NULL);
			return status;
		}
		tmp_vdev = mlo_get_assoc_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect(tmp_vdev, source,
						    reason_code, NULL);
			return status;
		}
	}

	return status;
}

QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_objmgr_vdev *tmp_vdev = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx && sta_ctx->orig_conn_req) {
		mlo_free_connect_ies(sta_ctx->orig_conn_req);
		qdf_mem_free(sta_ctx->orig_conn_req);
		sta_ctx->orig_conn_req = NULL;
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		mlo_dev_lock_acquire(mlo_dev_ctx);
		if (sta_ctx->connect_req) {
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}

		status = mlo_send_link_disconnect(mlo_dev_ctx, source,
						  reason_code, bssid);

		if (QDF_IS_STATUS_SUCCESS(status)) {
			mlo_dev_lock_release(mlo_dev_ctx);
			return status;
		}

		tmp_vdev = mlo_get_connected_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect(tmp_vdev, source,
						    reason_code, NULL);
			mlo_dev_lock_release(mlo_dev_ctx);
			return status;
		}
		tmp_vdev = mlo_get_assoc_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect(tmp_vdev, source,
						    reason_code, NULL);
			mlo_dev_lock_release(mlo_dev_ctx);
			return status;
		}
		mlo_dev_lock_release(mlo_dev_ctx);
		return status;
	}
	return wlan_cm_disconnect(vdev, source,
				  reason_code, NULL);
}

QDF_STATUS mlo_sync_disconnect(struct wlan_objmgr_vdev *vdev,
			       enum wlan_cm_source source,
			       enum wlan_reason_code reason_code,
			       struct qdf_mac_addr *bssid)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_objmgr_vdev *tmp_vdev = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx && sta_ctx->orig_conn_req) {
		mlo_free_connect_ies(sta_ctx->orig_conn_req);
		qdf_mem_free(sta_ctx->orig_conn_req);
		sta_ctx->orig_conn_req = NULL;
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		if (sta_ctx->connect_req) {
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}

		status = mlo_send_link_disconnect_sync(mlo_dev_ctx, source,
						       reason_code, bssid);

		if (QDF_IS_STATUS_SUCCESS(status))
			return status;

		tmp_vdev = mlo_get_connected_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect_sync(tmp_vdev, source,
							 reason_code);
			return status;
		}
		tmp_vdev = mlo_get_assoc_link_vdev(mlo_dev_ctx);
		if (tmp_vdev) {
			status = wlan_cm_disconnect_sync(tmp_vdev, source,
							 reason_code);
			return status;
		}
		return status;
	}
	return wlan_cm_disconnect_sync(vdev, source,
				       reason_code);
}

/**
 * mlo_handle_disconnect_resp- Issue desired actions on partner link vdev
 *
 * @mlo_dev_ctx: pointer to mlo dev context
 * @resp: disconnect resp
 *
 * Return: none
 */
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static
void mlo_handle_disconnect_resp(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct wlan_cm_discon_rsp *resp)
{
/* If it is secondary link then delete vdev object from mlo device. */
	enum wlan_cm_source source;
	enum wlan_reason_code reason_code;
	uint8_t i = 0;

	mlo_dev_lock_acquire(mlo_dev_ctx);
	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (wlan_cm_is_vdev_connected(mlo_dev_ctx->wlan_vdev_list[i])) {
			if (wlan_vdev_mlme_is_mlo_link_vdev(
					mlo_dev_ctx->wlan_vdev_list[i])) {
				source = resp->req.req.source;
				reason_code = resp->req.req.reason_code;
				wlan_cm_disconnect(
						mlo_dev_ctx->wlan_vdev_list[i],
						source, reason_code, NULL);
				mlo_dev_lock_release(mlo_dev_ctx);
				return;
			}
		}
	}
	mlo_dev_lock_release(mlo_dev_ctx);
}
#else
static
void mlo_handle_disconnect_resp(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct wlan_cm_discon_rsp *resp)
{
	struct wlan_objmgr_vdev *assoc_vdev = NULL;
	enum wlan_cm_source source;
	enum wlan_reason_code reason_code;
	uint8_t i = 0;

	mlo_dev_lock_acquire(mlo_dev_ctx);
	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links)) {
			if (wlan_cm_is_vdev_connected(
					mlo_dev_ctx->wlan_vdev_list[i])) {
				if (wlan_vdev_mlme_is_mlo_link_vdev(
					mlo_dev_ctx->wlan_vdev_list[i])) {
					source = resp->req.req.source;
					reason_code = resp->req.req.reason_code;
					wlan_cm_disconnect(mlo_dev_ctx->wlan_vdev_list[i],
							   source, reason_code, NULL);
					mlo_dev_lock_release(mlo_dev_ctx);
					return;
				}
			}
		}
	}
	assoc_vdev = mlo_get_assoc_link_vdev(mlo_dev_ctx);
	if (assoc_vdev && wlan_cm_is_vdev_connected(assoc_vdev)) {
		source = resp->req.req.source;
		reason_code = resp->req.req.reason_code;
		wlan_cm_disconnect(assoc_vdev,
				   source, reason_code, NULL);
		mlo_dev_lock_release(mlo_dev_ctx);
		return;
	}
	mlo_dev_lock_release(mlo_dev_ctx);
}
#endif

void mlo_sta_link_disconn_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_discon_rsp *resp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!mlo_dev_ctx || !(wlan_vdev_mlme_is_mlo_vdev(vdev)))
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	if (!wlan_cm_is_vdev_disconnected(vdev))
		return;

	mlo_update_connected_links(vdev, 0);
	if (vdev == mlo_get_assoc_link_vdev(mlo_dev_ctx)) {
		if (sta_ctx->connect_req) {
			mlo_connect(mlo_get_assoc_link_vdev(mlo_dev_ctx),
				    sta_ctx->connect_req);
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}
		return;
	}

	mlo_handle_disconnect_resp(mlo_dev_ctx, resp);
}

bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev)
{
	if ((wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE) &&
	    wlan_vdev_mlme_is_mlo_vdev(vdev))
		return true;

	return false;
}

#ifndef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
struct wlan_objmgr_vdev *
mlo_get_ml_vdev_by_mac(struct wlan_objmgr_vdev *vdev,
		       struct qdf_mac_addr *macaddr)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return NULL;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if(qdf_mem_cmp(macaddr,
			       wlan_vdev_mlme_get_macaddr(mlo_dev_ctx->wlan_vdev_list[i]),
			       QDF_MAC_ADDR_SIZE) == 0) {
			return mlo_dev_ctx->wlan_vdev_list[i];
		}
	}
	return NULL;
}
#endif

qdf_freq_t
mlo_get_chan_freq_by_bssid(struct wlan_objmgr_pdev *pdev,
			   struct qdf_mac_addr *bssid)
{
	struct scan_filter *scan_filter;
	int8_t ch_freq = 0;
	qdf_list_t *list = NULL;
	struct scan_cache_node *first_node = NULL;
	qdf_list_node_t *cur_node = NULL;

	scan_filter = qdf_mem_malloc(sizeof(*scan_filter));
	if (!scan_filter)
		return ch_freq;

	scan_filter->num_of_bssid = 1;
	qdf_mem_copy(scan_filter->bssid_list[0].bytes,
		     bssid, sizeof(struct qdf_mac_addr));
	list = wlan_scan_get_result(pdev, scan_filter);
	qdf_mem_free(scan_filter);

	if (!list || (list && !qdf_list_size(list))) {
		mlo_debug("scan list empty");
		goto error;
	}

	qdf_list_peek_front(list, &cur_node);
	first_node = qdf_container_of(cur_node,
				      struct scan_cache_node,
				      node);
	if (first_node && first_node->entry)
		ch_freq = first_node->entry->channel.chan_freq;
error:
	if (list)
		wlan_scan_purge_results(list);

	return ch_freq;
}

void mlo_get_assoc_rsp(struct wlan_objmgr_vdev *vdev,
		       struct element_info *assoc_rsp_frame)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = mlo_dev_ctx->sta_ctx;

	if (!mlo_dev_ctx || !mlo_dev_ctx->sta_ctx)
		return;

	if (!sta_ctx->assoc_rsp.len || !sta_ctx->assoc_rsp.ptr) {
		mlo_err("Assoc Resp info is empty");
		return;
	}

	*assoc_rsp_frame = sta_ctx->assoc_rsp;
}
#endif

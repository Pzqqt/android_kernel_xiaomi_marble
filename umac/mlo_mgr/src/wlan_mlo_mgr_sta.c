/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <scheduler_api.h>

#ifdef WLAN_FEATURE_11BE_MLO
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
wlan_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;

	if (!mlo_dev_ctx || !wlan_vdev_mlme_is_mlo_vdev(vdev))
		return NULL;

	return mlo_get_assoc_link_vdev(mlo_dev_ctx);
}

struct wlan_objmgr_vdev *
ucfg_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	return wlan_mlo_get_assoc_link_vdev(vdev);
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
static QDF_STATUS
mlo_send_link_disconnect(struct wlan_mlo_dev_context *mlo_dev_ctx,
			 enum wlan_cm_source source,
			 enum wlan_reason_code reason_code,
			 struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;
	enum wlan_cm_source link_source = source;
	struct wlan_objmgr_vdev *assoc_vdev =
			mlo_get_assoc_link_vdev(mlo_dev_ctx);

	if (!assoc_vdev)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Change the source for the link vdev to make sure it's handled as a
	 * Northbound disconnect in VDEV/PEER state machine.
	 */
	if (source != CM_OSIF_DISCONNECT)
		link_source = CM_MLO_LINK_VDEV_DISCONNECT;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i, mlo_dev_ctx->sta_ctx->wlan_connected_links) &&
		    mlo_dev_ctx->wlan_vdev_list[i] != mlo_get_assoc_link_vdev(mlo_dev_ctx))
			wlan_cm_disconnect(mlo_dev_ctx->wlan_vdev_list[i],
					   link_source, reason_code,
					   NULL);
	}

	wlan_cm_disconnect(assoc_vdev,
			   source, reason_code, NULL);
	return QDF_STATUS_SUCCESS;
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

static inline
void mlo_mld_clear_mlo_cap(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	uint8_t i = 0;

	if (!mlo_dev_ctx || !wlan_vdev_mlme_is_mlo_vdev(vdev))
		return;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;
		wlan_vdev_mlme_feat_ext2_cap_clear(
				mlo_dev_ctx->wlan_vdev_list[i],
				WLAN_VDEV_FEXT2_MLO);
	}
}

void ucfg_mlo_mld_clear_mlo_cap(struct wlan_objmgr_vdev *vdev)
{
	mlo_mld_clear_mlo_cap(vdev);
}

static void
mlo_cm_handle_connect_in_disconnection_state(struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_req *req)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx;

	if (!mlo_dev_ctx) {
		mlo_err("ML dev ctx is NULL");
		return;
	}

	sta_ctx = mlo_dev_ctx->sta_ctx;
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

static QDF_STATUS mlo_disconnect_no_lock(struct wlan_objmgr_vdev *vdev,
					 enum wlan_cm_source source,
					 enum wlan_reason_code reason_code,
					 struct qdf_mac_addr *bssid)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx) {
		copied_conn_req_lock_acquire(sta_ctx);
		if (sta_ctx->copied_conn_req) {
			mlo_free_connect_ies(sta_ctx->copied_conn_req);
			qdf_mem_free(sta_ctx->copied_conn_req);
			sta_ctx->copied_conn_req = NULL;
		}
		copied_conn_req_lock_release(sta_ctx);
	} else {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		sta_ctx = mlo_dev_ctx->sta_ctx;
		if (!sta_ctx)
			return QDF_STATUS_E_FAILURE;

		if (sta_ctx->connect_req) {
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}

		status = mlo_send_link_disconnect(mlo_dev_ctx, source,
						  reason_code, bssid);
	}

	return status;
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
			/*
			 * clone security params in all partner sta vaps
			 */
			mlo_mlme_clone_sta_security(
				mlo_dev_ctx->wlan_vdev_list[i], req);
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

		copied_conn_req_lock_acquire(sta_ctx);
		if (!sta_ctx->copied_conn_req)
			sta_ctx->copied_conn_req = qdf_mem_malloc(
					sizeof(struct wlan_cm_connect_req));
		else
			mlo_free_connect_ies(sta_ctx->copied_conn_req);

		mlo_debug("storing orig connect req");
		if (sta_ctx->copied_conn_req) {
			qdf_mem_copy(sta_ctx->copied_conn_req, req,
				     sizeof(struct wlan_cm_connect_req));
			mlo_allocate_and_copy_ies(sta_ctx->copied_conn_req,
						  req);
			copied_conn_req_lock_release(sta_ctx);
		} else {
			mlo_err("Failed to allocate orig connect req");
			copied_conn_req_lock_release(sta_ctx);
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
	struct wlan_mlo_sta *sta_ctx;

	if (!mlo_dev_ctx) {
		mlo_err("ML dev ctx is NULL");
		return;
	}

	sta_ctx = mlo_dev_ctx->sta_ctx;

	mlo_debug("Partner link connect mac:" QDF_MAC_ADDR_FMT " vdev_id:%d",
		  QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		  wlan_vdev_get_id(vdev));

	qdf_mem_copy(&req, sta_ctx->copied_conn_req,
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

	mlo_allocate_and_copy_ies(&req, sta_ctx->copied_conn_req);
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
		      struct mlo_partner_info *ml_parnter_info)
{
	/* Create the secondary interface, Send keys if the last link */
	uint8_t i, partner_idx = 0;
	struct wlan_ssid ssid = {0};

	mlo_debug("Sending link connect on partner interface");
	wlan_vdev_mlme_get_ssid(
			vdev, ssid.ssid,
			&ssid.length);

	if (!ml_parnter_info->num_partner_links) {
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
		wlan_vdev_set_link_id(
		      mlo_dev_ctx->wlan_vdev_list[i],
		      ml_parnter_info->partner_link_info[partner_idx].link_id);
		ml_parnter_info->partner_link_info[partner_idx].vdev_id =
			       wlan_vdev_get_id(mlo_dev_ctx->wlan_vdev_list[i]);
		mlo_prepare_and_send_connect(
				mlo_dev_ctx->wlan_vdev_list[i],
				*ml_parnter_info,
				ml_parnter_info->partner_link_info[partner_idx],
				ssid);
		mlo_update_connected_links(mlo_dev_ctx->wlan_vdev_list[i], 1);
		partner_idx++;
	}
}
#else
static void
mlo_send_link_connect(struct wlan_objmgr_vdev *vdev,
		      struct wlan_mlo_dev_context *mlo_dev_ctx,
		      struct element_info *assoc_rsp,
		      struct mlo_partner_info *ml_parnter_info)
{
	struct wlan_ssid ssid = {0};
	uint8_t i = 0;
	uint8_t j = 0;

	if (!ml_parnter_info->num_partner_links) {
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
					for (j = 0; j < ml_parnter_info->num_partner_links; j++) {
						if (mlo_dev_ctx->wlan_vdev_list[i]->vdev_mlme.mlo_link_id ==
							ml_parnter_info->partner_link_info[j].link_id)
							break;
					}
					if (j < ml_parnter_info->num_partner_links) {
						wlan_vdev_mlme_get_ssid(
							vdev, ssid.ssid,
							&ssid.length);
						mlo_prepare_and_send_connect(
							mlo_dev_ctx->wlan_vdev_list[i],
							*ml_parnter_info,
							ml_parnter_info->partner_link_info[j],
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

void
mlo_update_connected_links_bmap(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct mlo_partner_info ml_parnter_info)
{
	uint8_t i = 0;
	uint8_t j = 0;

	if (!mlo_dev_ctx) {
		mlo_err("ML dev ctx is NULL");
		return;
	}

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

static QDF_STATUS ml_activate_disconnect_req_sched_cb(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev = msg->bodyptr;

	if (!vdev) {
		mlme_err("Null input vdev");
		return QDF_STATUS_E_INVAL;
	}

	mlo_disconnect(vdev, CM_OSIF_DISCONNECT,
		       REASON_UNSPEC_FAILURE, NULL);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ml_activate_disconnect_req_flush_cb(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev = msg->bodyptr;

	if (!vdev) {
		mlme_err("Null input vdev");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static inline
QDF_STATUS mlo_post_disconnect_msg(struct scheduler_msg *msg)
{
	return scheduler_post_message(
			QDF_MODULE_ID_SYS,
			QDF_MODULE_ID_SYS,
			QDF_MODULE_ID_SYS,
			msg);
}
#else
static inline
QDF_STATUS mlo_post_disconnect_msg(struct scheduler_msg *msg)
{
	return scheduler_post_message(
			QDF_MODULE_ID_MLME,
			QDF_MODULE_ID_MLME,
			QDF_MODULE_ID_MLME,
			msg);
}
#endif

static inline
void mlo_handle_sta_link_connect_failure(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_resp *rsp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct scheduler_msg msg = {0};
	QDF_STATUS ret;

	if (!mlo_dev_ctx) {
		mlo_err("ML dev ctx is NULL");
		return;
	}

	if (vdev != mlo_get_assoc_link_vdev(mlo_dev_ctx)) {
		mlo_update_connected_links(vdev, 0);
		if (rsp->reason == CM_NO_CANDIDATE_FOUND ||
		    rsp->reason == CM_HW_MODE_FAILURE ||
		    rsp->reason == CM_SER_FAILURE) {
			ret = wlan_objmgr_vdev_try_get_ref(
					vdev, WLAN_MLO_MGR_ID);
			if (QDF_IS_STATUS_ERROR(ret)) {
				mlo_err("Failed to get ref vdev_id %d",
					wlan_vdev_get_id(vdev));
				return;
			}
			/* Since these failures happen in same context. use
			 * scheduler to avoid deadlock by deferring context
			 */
			msg.bodyptr = vdev;
			msg.callback = ml_activate_disconnect_req_sched_cb;
			msg.flush_callback =
				ml_activate_disconnect_req_flush_cb;
			mlo_post_disconnect_msg(&msg);
			if (QDF_IS_STATUS_ERROR(ret)) {
				wlan_objmgr_vdev_release_ref(
						vdev,
						WLAN_MLO_MGR_ID);
				return;
			}
		} else {
			mlo_disconnect(vdev, CM_OSIF_DISCONNECT,
				       REASON_UNSPEC_FAILURE, NULL);
		}
	}
}

void mlo_sta_link_connect_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_resp *rsp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (mlo_dev_ctx) {
		sta_ctx = mlo_dev_ctx->sta_ctx;
	} else {
		mlo_debug_rl("mlo_dev_ctx is NULL");
		return;
	}

	if (wlan_cm_is_vdev_disconnected(vdev)) {
		if (sta_ctx) {
			copied_conn_req_lock_acquire(sta_ctx);
			if (sta_ctx->copied_conn_req) {
				mlo_free_connect_ies(sta_ctx->copied_conn_req);
				qdf_mem_free(sta_ctx->copied_conn_req);
				sta_ctx->copied_conn_req = NULL;
			}
			copied_conn_req_lock_release(sta_ctx);
		}
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		mlo_debug("Vdev: %d", wlan_vdev_get_id(vdev));
		if (wlan_cm_is_vdev_disconnected(vdev)) {
			mlo_handle_sta_link_connect_failure(vdev, rsp);
			return;
		} else if (!wlan_cm_is_vdev_connected(vdev)) {
			/* If vdev is not in disconnected or connected state,
			 * then the event is received due to connect req being
			 * flushed. Hence, ignore this event
			 */
			return;
		}

		if (!wlan_vdev_mlme_is_mlo_link_vdev(vdev) && sta_ctx) {
			if (sta_ctx->assoc_rsp.ptr) {
				qdf_mem_free(sta_ctx->assoc_rsp.ptr);
				sta_ctx->assoc_rsp.ptr = NULL;
			}
			sta_ctx->assoc_rsp.len = rsp->connect_ies.assoc_rsp.len;
			sta_ctx->assoc_rsp.ptr =
				qdf_mem_malloc(rsp->connect_ies.assoc_rsp.len);
			if (!sta_ctx->assoc_rsp.ptr) {
				QDF_ASSERT(0);
				return;
			}
			if (rsp->connect_ies.assoc_rsp.ptr)
				qdf_mem_copy(sta_ctx->assoc_rsp.ptr,
					     rsp->connect_ies.assoc_rsp.ptr,
					     rsp->connect_ies.assoc_rsp.len);
			/* Update connected_links_bmap for all vdev taking
			 * part in association
			 */
			mlo_update_connected_links(vdev, 1);
			mlo_update_connected_links_bmap(mlo_dev_ctx,
							rsp->ml_parnter_info);
		}
		mlo_send_link_connect(vdev, mlo_dev_ctx,
				      &rsp->connect_ies.assoc_rsp,
				      &rsp->ml_parnter_info);
	}
}

/**
 * mlo_send_link_disconnect_sync- Issue sync the disconnect request on MLD links
 *
 * @mlo_dev_ctx: pointer to mlo dev context
 * @source: disconnect source
 * @reason_code: disconnect reason
 * @bssid: bssid of AP to disconnect, can be null if not known
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mlo_send_link_disconnect_sync(struct wlan_mlo_dev_context *mlo_dev_ctx,
			      enum wlan_cm_source source,
			      enum wlan_reason_code reason_code,
			      struct qdf_mac_addr *bssid)
{
	uint8_t i = 0;
	struct wlan_objmgr_vdev *assoc_vdev =
			mlo_get_assoc_link_vdev(mlo_dev_ctx);

	if (!assoc_vdev)
		return QDF_STATUS_E_FAILURE;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (mlo_dev_ctx->wlan_vdev_list[i] !=
				mlo_get_assoc_link_vdev(mlo_dev_ctx))
			wlan_cm_disconnect_sync(mlo_dev_ctx->wlan_vdev_list[i],
						source, reason_code);
	}

	wlan_cm_disconnect_sync(assoc_vdev,
				source, reason_code);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx) {
		copied_conn_req_lock_acquire(sta_ctx);
		if (sta_ctx->copied_conn_req) {
			mlo_free_connect_ies(sta_ctx->copied_conn_req);
			qdf_mem_free(sta_ctx->copied_conn_req);
			sta_ctx->copied_conn_req = NULL;
		}
		copied_conn_req_lock_release(sta_ctx);
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
	struct wlan_mlo_dev_context *mlo_dev_ctx = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (mlo_dev_ctx)
		sta_ctx = mlo_dev_ctx->sta_ctx;
	if (sta_ctx) {
		copied_conn_req_lock_acquire(sta_ctx);
		if (sta_ctx->copied_conn_req) {
			mlo_free_connect_ies(sta_ctx->copied_conn_req);
			qdf_mem_free(sta_ctx->copied_conn_req);
			sta_ctx->copied_conn_req = NULL;
		}
		copied_conn_req_lock_release(sta_ctx);
	}

	if (wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		if (sta_ctx->connect_req) {
			mlo_free_connect_ies(sta_ctx->connect_req);
			qdf_mem_free(sta_ctx->connect_req);
			sta_ctx->connect_req = NULL;
		}

		status = mlo_send_link_disconnect_sync(mlo_dev_ctx, source,
						       reason_code, bssid);

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
{ }

static QDF_STATUS ml_activate_connect_req_sched_cb(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev = msg->bodyptr;
	struct wlan_mlo_dev_context *mlo_dev_ctx = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!vdev) {
		mlme_err("Null input vdev");
		return QDF_STATUS_E_INVAL;
	}

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (!mlo_dev_ctx) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
		return QDF_STATUS_E_INVAL;
	}

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
		return QDF_STATUS_E_INVAL;
	}

	mlo_connect(vdev, sta_ctx->connect_req);
	mlo_free_connect_ies(sta_ctx->connect_req);
	qdf_mem_free(sta_ctx->connect_req);
	sta_ctx->connect_req = NULL;

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ml_activate_connect_req_flush_cb(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev = msg->bodyptr;

	if (!vdev) {
		mlme_err("Null input vdev");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static inline
void mlo_sta_link_handle_pending_connect(struct wlan_objmgr_vdev *vdev)
{ }
#else
static inline
void mlo_sta_link_handle_pending_connect(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_objmgr_vdev *tmp_vdev;
	struct scheduler_msg msg = {0};
	QDF_STATUS ret;
	struct wlan_mlo_sta *sta_ctx = NULL;
	uint8_t i = 0;
	struct mlo_partner_info partner_info;
	struct mlo_link_info partner_link_info;

	if (!mlo_dev_ctx) {
		mlo_err("ML dev ctx is null");
		return;
	}
	sta_ctx = mlo_dev_ctx->sta_ctx;
	ret = wlan_objmgr_vdev_try_get_ref(
			vdev,
			WLAN_MLO_MGR_ID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		mlo_free_connect_ies(sta_ctx->connect_req);
		qdf_mem_free(sta_ctx->connect_req);
		sta_ctx->connect_req = NULL;
		return;
	}
	msg.bodyptr = vdev;
	msg.callback = ml_activate_connect_req_sched_cb;
	msg.flush_callback = ml_activate_connect_req_flush_cb;

	ret = scheduler_post_message(QDF_MODULE_ID_MLME,
				     QDF_MODULE_ID_MLME,
				     QDF_MODULE_ID_MLME, &msg);
	if (QDF_IS_STATUS_ERROR(ret)) {
		mlo_free_connect_ies(sta_ctx->connect_req);
		qdf_mem_free(sta_ctx->connect_req);
		sta_ctx->connect_req = NULL;
		wlan_objmgr_vdev_release_ref(vdev,
					     WLAN_MLO_MGR_ID);
		return;
	}

	if (sta_ctx->connect_req->ml_parnter_info.num_partner_links) {
		partner_info = sta_ctx->connect_req->ml_parnter_info;
		wlan_vdev_mlme_feat_ext2_cap_set(vdev, WLAN_VDEV_FEXT2_MLO);
		mlo_clear_connect_req_links_bmap(vdev);
		mlo_update_connect_req_links(vdev, 1);
		for (i = 0; i < partner_info.num_partner_links; i++) {
			partner_link_info = partner_info.partner_link_info[i];
			tmp_vdev = mlo_get_ml_vdev_by_mac(
					vdev,
					&partner_link_info.link_addr);
			if (tmp_vdev) {
				mlo_update_connect_req_links(tmp_vdev, 1);
				wlan_vdev_mlme_feat_ext2_cap_set(
						tmp_vdev, WLAN_VDEV_FEXT2_MLO);
				wlan_vdev_mlme_feat_ext2_cap_set(
						tmp_vdev,
						WLAN_VDEV_FEXT2_MLO_STA_LINK);
				wlan_vdev_set_link_id(
					tmp_vdev,
					partner_link_info.link_id);
			}
		}
	}
}
#endif

void mlo_sta_link_disconn_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_discon_rsp *resp)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	struct wlan_objmgr_vdev *assoc_vdev = NULL;

	if (!mlo_dev_ctx || !(wlan_vdev_mlme_is_mlo_vdev(vdev)))
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	if (!wlan_cm_is_vdev_disconnected(vdev))
		return;

	mlo_update_connected_links(vdev, 0);
	if (mlo_is_mld_disconnected(vdev)) {
		if (sta_ctx->connect_req) {
			assoc_vdev = mlo_get_assoc_link_vdev(mlo_dev_ctx);
			mlo_sta_link_handle_pending_connect(assoc_vdev);
		}
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
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!mlo_dev_ctx || !mlo_dev_ctx->sta_ctx)
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;

	if (!sta_ctx->assoc_rsp.len || !sta_ctx->assoc_rsp.ptr) {
		mlo_err("Assoc Resp info is empty");
		return;
	}

	*assoc_rsp_frame = sta_ctx->assoc_rsp;
}

QDF_STATUS mlo_sta_save_quiet_status(struct wlan_mlo_dev_context *mlo_dev_ctx,
				     uint8_t link_id,
				     bool quiet_status)
{
	struct wlan_mlo_sta *sta_ctx;
	int i;
	bool find_free_buffer = false;
	int free_idx;

	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		return QDF_STATUS_E_INVAL;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		mlo_dev_lock_release(mlo_dev_ctx);
		return QDF_STATUS_E_INVAL;
	}
	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_quiet_status); i++) {
		if (!sta_ctx->mlo_quiet_status[i].valid_status) {
			if (!find_free_buffer) {
				free_idx = i;
				find_free_buffer = true;
			}
		} else if (link_id == sta_ctx->mlo_quiet_status[i].link_id) {
			sta_ctx->mlo_quiet_status[i].quiet_status =
							quiet_status;
			mlo_debug("mld mac " QDF_MAC_ADDR_FMT " link id %d quiet status update %d",
				  QDF_MAC_ADDR_REF(mlo_dev_ctx->mld_addr.bytes),
				  link_id, quiet_status);
			mlo_dev_lock_release(mlo_dev_ctx);
			return QDF_STATUS_SUCCESS;
		}
	}
	if (!find_free_buffer) {
		mlo_err("no free buffer for link id %d to save quiet_status",
			link_id);
		mlo_dev_lock_release(mlo_dev_ctx);
		return QDF_STATUS_E_INVAL;
	}
	sta_ctx->mlo_quiet_status[free_idx].quiet_status = quiet_status;
	sta_ctx->mlo_quiet_status[free_idx].link_id = link_id;
	sta_ctx->mlo_quiet_status[free_idx].valid_status = true;

	mlo_debug("mld mac " QDF_MAC_ADDR_FMT " link id %d in quiet status %d",
		  QDF_MAC_ADDR_REF(mlo_dev_ctx->mld_addr.bytes),
		  link_id, quiet_status);
	mlo_dev_lock_release(mlo_dev_ctx);

	return QDF_STATUS_SUCCESS;
}

bool mlo_is_sta_in_quiet_status(struct wlan_mlo_dev_context *mlo_dev_ctx,
				uint8_t link_id)
{
	struct wlan_mlo_sta *sta_ctx;
	int i;
	bool quiet_status = false;

	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		return quiet_status;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		mlo_dev_lock_release(mlo_dev_ctx);
		return quiet_status;
	}
	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_quiet_status); i++) {
		if (sta_ctx->mlo_quiet_status[i].valid_status &&
		    link_id == sta_ctx->mlo_quiet_status[i].link_id) {
			quiet_status =
				sta_ctx->mlo_quiet_status[i].quiet_status;
			break;
		}
	}
	mlo_dev_lock_release(mlo_dev_ctx);

	return quiet_status;
}

bool mlo_is_sta_inactivity_allowed_with_quiet(struct wlan_objmgr_psoc *psoc,
					      uint8_t *vdev_id_list,
					      uint8_t num_mlo, uint8_t *mlo_idx,
					      uint8_t affected_links,
					      uint8_t *affected_list)
{
	uint8_t i, j;
	struct wlan_objmgr_vdev *vdev;
	bool allowed = false;

	for (i = 0; i < num_mlo; i++) {
		for (j = 0; j < affected_links; j++) {
			if (vdev_id_list[mlo_idx[i]] == affected_list[j])
				break;
		}
		if (j != affected_links)
			continue;
		/* find vdev not in affected_list */
		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
				psoc, vdev_id_list[mlo_idx[i]],
				WLAN_IF_MGR_ID);
		if (!vdev) {
			mlo_err("invalid vdev for id %d",
				vdev_id_list[mlo_idx[i]]);
			continue;
		}

		/* for not affected vdev, check the vdev is in quiet or not*/
		allowed = !mlo_is_sta_in_quiet_status(
				vdev->mlo_dev_ctx, wlan_vdev_get_link_id(vdev));
		wlan_objmgr_vdev_release_ref(vdev, WLAN_IF_MGR_ID);
		if (allowed) {
			mlo_debug("vdev id %d link id %d is not in quiet, allow partner link to trigger inactivity",
				  wlan_vdev_get_id(vdev),
				  wlan_vdev_get_link_id(vdev));
			break;
		}
	}

	return allowed;
}

bool mlo_is_sta_csa_synced(struct wlan_mlo_dev_context *mlo_dev_ctx,
			   uint8_t link_id)
{
	struct wlan_mlo_sta *sta_ctx;
	int i;
	bool sta_csa_synced = false;

	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		return sta_csa_synced;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		mlo_dev_lock_release(mlo_dev_ctx);
		return sta_csa_synced;
	}
	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_csa_param); i++) {
		if (link_id == sta_ctx->mlo_csa_param[i].link_id &&
		    (sta_ctx->mlo_csa_param[i].valid_csa_param ||
		     sta_ctx->mlo_csa_param[i].mlo_csa_synced)) {
			mlo_dev_lock_release(mlo_dev_ctx);
			sta_csa_synced =
				sta_ctx->mlo_csa_param[i].mlo_csa_synced;
			break;
		}
	}
	mlo_dev_lock_release(mlo_dev_ctx);

	return sta_csa_synced;
}

QDF_STATUS mlo_sta_csa_save_params(struct wlan_mlo_dev_context *mlo_dev_ctx,
				   uint8_t link_id,
				   struct csa_offload_params *csa_param)
{
	struct wlan_mlo_sta *sta_ctx;
	int i;
	bool find_free_buffer = false;
	int free_idx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		status = QDF_STATUS_E_INVAL;
		goto done;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}
	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_csa_param); i++) {
		if (!sta_ctx->mlo_csa_param[i].valid_csa_param &&
		    !sta_ctx->mlo_csa_param[i].mlo_csa_synced) {
			if (!find_free_buffer) {
				free_idx = i;
				find_free_buffer = true;
			}
		} else if (link_id == sta_ctx->mlo_csa_param[i].link_id) {
			qdf_mem_copy(&sta_ctx->mlo_csa_param[i].csa_param,
				     csa_param, sizeof(*csa_param));
			mlo_debug("mld mac " QDF_MAC_ADDR_FMT " link id %d update csa",
				  QDF_MAC_ADDR_REF(mlo_dev_ctx->mld_addr.bytes),
				  link_id);
			goto rel_lock;
		}
	}
	if (!find_free_buffer) {
		mlo_err("no free buffer of csa param for link %d in sta_ctx",
			link_id);
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}
	qdf_mem_copy(&sta_ctx->mlo_csa_param[free_idx].csa_param,
		     csa_param, sizeof(*csa_param));
	sta_ctx->mlo_csa_param[free_idx].link_id = link_id;
	sta_ctx->mlo_csa_param[free_idx].valid_csa_param = true;
	mlo_debug("mld mac " QDF_MAC_ADDR_FMT " link id %d RX csa",
		  QDF_MAC_ADDR_REF(mlo_dev_ctx->mld_addr.bytes),
		  link_id);

rel_lock:
	mlo_dev_lock_release(mlo_dev_ctx);

done:

	return status;
}

QDF_STATUS mlo_sta_up_active_notify(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_sta *sta_ctx;
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	uint8_t link_id;
	int i;
	bool find_free_buffer = false;
	int free_idx;
	struct csa_offload_params csa_param;
	struct wlan_channel *chan;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!vdev) {
		mlo_err("invalid vdev");
		status = QDF_STATUS_E_INVAL;
		goto done;
	}
	link_id = wlan_vdev_get_link_id(vdev);
	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		status = QDF_STATUS_E_INVAL;
		goto done;
	}
	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}

	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_csa_param); i++) {
		if (!sta_ctx->mlo_csa_param[i].valid_csa_param &&
		    !sta_ctx->mlo_csa_param[i].mlo_csa_synced) {
			if (!find_free_buffer) {
				free_idx = i;
				find_free_buffer = true;
			}
		} else if (link_id == sta_ctx->mlo_csa_param[i].link_id) {
			if (sta_ctx->mlo_csa_param[i].valid_csa_param &&
			    !sta_ctx->mlo_csa_param[i].mlo_csa_synced) {
				mlo_debug("mld mac " QDF_MAC_ADDR_FMT " vdev id %d link id %d handle csa",
					  QDF_MAC_ADDR_REF(
						mlo_dev_ctx->mld_addr.bytes),
					  wlan_vdev_get_id(vdev), link_id);
				csa_param = sta_ctx->mlo_csa_param[i].csa_param;
				sta_ctx->mlo_csa_param[i].mlo_csa_synced = true;
				mlo_dev_lock_release(mlo_dev_ctx);
				chan = wlan_vdev_mlme_get_bss_chan(vdev);
				if (csa_param.csa_chan_freq && chan &&
				    csa_param.csa_chan_freq != chan->ch_freq)
					mlo_mlme_handle_sta_csa_param(
						vdev, &csa_param);
				goto done;
			}
			sta_ctx->mlo_csa_param[i].mlo_csa_synced = true;
			goto rel_lock;
		}
	}
	if (!find_free_buffer) {
		mlo_err("no free buffer of csa param for link %d in sta_ctx",
			link_id);
		goto rel_lock;
	}
	sta_ctx->mlo_csa_param[free_idx].mlo_csa_synced = true;
	sta_ctx->mlo_csa_param[free_idx].link_id = link_id;
	mlo_debug("mld mac " QDF_MAC_ADDR_FMT " link id %d UP Active",
		  QDF_MAC_ADDR_REF(mlo_dev_ctx->mld_addr.bytes),
		  link_id);

rel_lock:
	mlo_dev_lock_release(mlo_dev_ctx);

done:

	return status;
}

bool mlo_is_sta_csa_param_handled(struct wlan_objmgr_vdev *vdev,
				  struct csa_offload_params *csa_param)
{
	struct wlan_mlo_sta *sta_ctx;
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	uint8_t link_id;
	int i;
	bool handled = false;

	if (!vdev) {
		mlo_err("invalid vdev");
		goto done;
	}
	link_id = wlan_vdev_get_link_id(vdev);
	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (!mlo_dev_ctx) {
		mlo_err("invalid mlo_dev_ctx");
		goto done;
	}
	mlo_dev_lock_acquire(mlo_dev_ctx);
	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx) {
		mlo_err("invalid sta_ctx");
		goto rel_lock;
	}

	for (i = 0; i < QDF_ARRAY_SIZE(sta_ctx->mlo_csa_param); i++) {
		if (link_id == sta_ctx->mlo_csa_param[i].link_id &&
		    (sta_ctx->mlo_csa_param[i].valid_csa_param ||
		     sta_ctx->mlo_csa_param[i].mlo_csa_synced))
			break;
	}

	if (i >= QDF_ARRAY_SIZE(sta_ctx->mlo_csa_param)) {
		mlo_debug("mlo csa synced does not happen before csa FW event");
		goto rel_lock;
	}
	if (!sta_ctx->mlo_csa_param[i].csa_offload_event_recvd) {
		sta_ctx->mlo_csa_param[i].csa_offload_event_recvd = true;
		if (sta_ctx->mlo_csa_param[i].valid_csa_param &&
		    !qdf_mem_cmp(&sta_ctx->mlo_csa_param[i].csa_param,
				 csa_param, sizeof(*csa_param)))
			handled = true;
	}

rel_lock:
	mlo_dev_lock_release(mlo_dev_ctx);

done:

	return handled;
}

void mlo_internal_disconnect_links(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = NULL;
	struct wlan_mlo_sta *sta_ctx = NULL;
	uint8_t i;

	if (!vdev)
		return;

	if (!wlan_vdev_mlme_is_assoc_sta_vdev(vdev)) {
		mlo_debug("Not an assoc vdev, so ignore disconnect req");
		return;
	}

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	if (mlo_dev_ctx) {
		sta_ctx = mlo_dev_ctx->sta_ctx;
	} else {
		mlo_err("Invalid mlo_dev_ctx");
		return;
	}

	if (sta_ctx) {
		copied_conn_req_lock_acquire(sta_ctx);
		if (sta_ctx->copied_conn_req) {
			mlo_free_connect_ies(sta_ctx->copied_conn_req);
			qdf_mem_free(sta_ctx->copied_conn_req);
			sta_ctx->copied_conn_req = NULL;
		}
		copied_conn_req_lock_release(sta_ctx);
	} else {
		mlo_err("Invalid sta_ctx");
		return;
	}

	mlo_dev_lock_acquire(mlo_dev_ctx);
	if (sta_ctx->connect_req) {
		mlo_free_connect_ies(sta_ctx->connect_req);
		qdf_mem_free(sta_ctx->connect_req);
		sta_ctx->connect_req = NULL;
	}

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (qdf_test_bit(i,
				 mlo_dev_ctx->sta_ctx->wlan_connected_links) &&
		    mlo_dev_ctx->wlan_vdev_list[i] !=
		    mlo_get_assoc_link_vdev(mlo_dev_ctx))
			wlan_cm_disconnect(mlo_dev_ctx->wlan_vdev_list[i],
					   CM_INTERNAL_DISCONNECT,
					   REASON_UNSPEC_FAILURE,
					   NULL);
	}

	mlo_dev_lock_release(mlo_dev_ctx);
}
#endif

/*
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
 * DOC: contains MLO manager roaming related functionality
 */
#include <wlan_cmn.h>
#include <wlan_cm_public_struct.h>
#include <wlan_cm_roam_public_struct.h>
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"
#include "wlan_mlo_mgr_roam.h"
#include "wlan_mlo_mgr_public_structs.h"
#include "wlan_mlo_mgr_sta.h"
#include <../../core/src/wlan_cm_roam_i.h>
#include "wlan_cm_roam_api.h"

#ifdef WLAN_FEATURE_11BE_MLO
static bool
mlo_check_connect_req_bmap(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return false;

	sta_ctx = mlo_dev_ctx->sta_ctx;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (vdev == mlo_dev_ctx->wlan_vdev_list[i])
			return qdf_test_bit(i, sta_ctx->wlan_connect_req_links);
	}

	mlo_err("vdev not found in ml dev ctx list");
	return false;
}

static void
mlo_update_for_multi_link_roam(struct wlan_objmgr_psoc *psoc,
			       uint8_t vdev_id,
			       uint8_t ml_link_vdev_id)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    ml_link_vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlo_err("VDEV is null");
		return;
	}

	if (vdev_id == ml_link_vdev_id) {
		wlan_vdev_mlme_feat_ext2_cap_set(vdev,
						 WLAN_VDEV_FEXT2_MLO);
		goto end;
	}

	wlan_vdev_mlme_feat_ext2_cap_set(vdev,
					 WLAN_VDEV_FEXT2_MLO);
	wlan_vdev_mlme_feat_ext2_cap_set(vdev,
					 WLAN_VDEV_FEXT2_MLO_STA_LINK);

	mlo_update_connect_req_links(vdev, true);

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
}

static void
mlo_cleanup_link(struct wlan_objmgr_vdev *tmp_vdev,
		 struct wlan_objmgr_vdev *vdev)
{
	wlan_vdev_mlme_feat_ext2_cap_clear(vdev,
					   WLAN_VDEV_FEXT2_MLO);

	if (wlan_vdev_mlme_is_mlo_link_vdev(tmp_vdev)) {
		cm_cleanup_mlo_link(tmp_vdev);
		wlan_vdev_mlme_feat_ext2_cap_clear(tmp_vdev,
				WLAN_VDEV_FEXT2_MLO_STA_LINK);
	}
}

static void
mlo_update_for_legacy_roam(struct wlan_objmgr_psoc *psoc,
			   uint8_t vdev_id)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	uint8_t i;
	struct wlan_objmgr_vdev *vdev, *tmp_vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlo_err("VDEV is null");
		return;
	}

	if (!vdev->mlo_dev_ctx)
		goto end;

	mlo_dev_ctx = vdev->mlo_dev_ctx;
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		tmp_vdev = mlo_dev_ctx->wlan_vdev_list[i];
		mlo_cleanup_link(tmp_vdev, vdev);
	}

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
}

static void
mlo_clear_link_bmap(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlo_err("VDEV is null");
		return;
	}

	mlo_clear_connect_req_links_bmap(vdev);
	wlan_vdev_mlme_feat_ext2_cap_clear(vdev,
					   WLAN_VDEV_FEXT2_MLO);
	if (wlan_vdev_mlme_is_mlo_link_vdev(vdev))
		wlan_vdev_mlme_feat_ext2_cap_clear(vdev,
				WLAN_VDEV_FEXT2_MLO_STA_LINK);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
}
#else
static inline void
mlo_clear_link_bmap(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{}

static inline void
mlo_update_for_legacy_roam(struct wlan_objmgr_psoc *psoc,
			   uint8_t vdev_id)
{}

static inline void
mlo_cleanup_link(struct wlan_objmgr_vdev *tmp_vdev,
		 struct wlan_objmgr_vdev *vdev)
{}

static inline void
mlo_update_for_multi_link_roam(struct wlan_objmgr_psoc *psoc,
			       uint8_t vdev_id,
			       uint8_t ml_link_vdev_id)
{}

static inline bool
mlo_check_connect_req_bmap(struct wlan_objmgr_vdev *vdev)
{}
#endif
QDF_STATUS mlo_fw_roam_sync_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				void *event, uint32_t event_data_len)
{
	struct roam_offload_synch_ind *sync_ind;
	QDF_STATUS status;
	uint8_t i;

	sync_ind = (struct roam_offload_synch_ind *)event;
	if (!sync_ind)
		return QDF_STATUS_E_FAILURE;

	for (i = 0; i < sync_ind->num_setup_links; i++)
		mlo_update_for_multi_link_roam(psoc, vdev_id,
					       sync_ind->ml_link[i].vdev_id);

	if (!sync_ind->num_setup_links)
		mlo_debug("MLO_ROAM: Roamed to Legacy");
	else
		mlo_debug("MLO_ROAM: Roamed to MLO");

	status = cm_fw_roam_sync_req(psoc, vdev_id, event, event_data_len);

	if (QDF_IS_STATUS_ERROR(status))
		mlo_clear_link_bmap(psoc, vdev_id);

	return status;
}

#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
void mlo_cm_roam_sync_cb(struct wlan_objmgr_vdev *vdev,
			 void *event, uint32_t event_data_len)
{
	QDF_STATUS status;
	struct roam_offload_synch_ind *sync_ind;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *link_vdev = NULL;
	uint8_t i;
	uint8_t vdev_id;

	sync_ind = (struct roam_offload_synch_ind *)event;
	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);

	if (!sync_ind->num_setup_links)
		mlo_update_for_legacy_roam(psoc, vdev_id);

	for (i = 0; i < sync_ind->num_setup_links; i++) {
		if (vdev_id == sync_ind->ml_link[i].vdev_id)
			continue;

		link_vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
								 sync_ind->ml_link[i].vdev_id,
								 WLAN_MLME_SB_ID);

		if (!link_vdev) {
			mlo_err("Link vdev is null");
			return;
		}

		if (mlo_check_connect_req_bmap(link_vdev)) {
			mlo_update_connect_req_links(link_vdev, false);

			status = cm_fw_roam_sync_req(psoc,
						     sync_ind->ml_link[i].vdev_id,
						     event, event_data_len);
			if (QDF_IS_STATUS_ERROR(status)) {
				mlo_clear_connect_req_links_bmap(link_vdev);
				wlan_objmgr_vdev_release_ref(link_vdev,
							     WLAN_MLME_SB_ID);
				return;
			}
		}
		wlan_objmgr_vdev_release_ref(link_vdev,
					     WLAN_MLME_SB_ID);
	}
}
#endif

void
mlo_fw_ho_fail_req(struct wlan_objmgr_psoc *psoc,
		   uint8_t vdev_id, struct qdf_mac_addr bssid)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_mlo_dev_context *mlo_dev_ctx;
	uint8_t i;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlo_err("vdev is null");
		return;
	}

	if (!vdev->mlo_dev_ctx)
		goto end;

	mlo_dev_ctx = vdev->mlo_dev_ctx;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i] ||
		    mlo_dev_ctx->wlan_vdev_list[i] == vdev)
			continue;
		cm_fw_ho_fail_req(psoc,
				  wlan_vdev_get_id(mlo_dev_ctx->wlan_vdev_list[i]),
				  bssid);
	}

end:
	cm_fw_ho_fail_req(psoc, vdev_id, bssid);
	wlan_objmgr_vdev_release_ref(vdev,
				     WLAN_MLME_SB_ID);
}

QDF_STATUS
mlo_get_sta_link_mac_addr(uint8_t vdev_id,
			  struct roam_offload_synch_ind *sync_ind,
			  struct qdf_mac_addr *link_mac_addr)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i;

	if (!sync_ind || !sync_ind->num_setup_links)
		return QDF_STATUS_E_FAILURE;

	for (i = 0; i < sync_ind->num_setup_links; i++) {
		if (sync_ind->ml_link[i].vdev_id == vdev_id) {
			qdf_copy_macaddr(link_mac_addr,
					 &sync_ind->ml_link[i].link_addr);
			return status;
		}
	}

	if (i == sync_ind->num_setup_links) {
		mlo_err("Link mac addr not found");
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

uint32_t
mlo_roam_get_chan_freq(uint8_t vdev_id,
		       struct roam_offload_synch_ind *sync_ind)
{
	uint8_t i;

	if (!sync_ind || !sync_ind->num_setup_links)
		return 0;

	for (i = 0; i < sync_ind->num_setup_links; i++) {
		if (sync_ind->ml_link[i].vdev_id == vdev_id)
			return sync_ind->ml_link[i].channel.mhz;
	}

	return 0;
}

uint32_t
mlo_roam_get_link_id(uint8_t vdev_id,
		     struct roam_offload_synch_ind *sync_ind)
{
	uint8_t i;

	if (!sync_ind || !sync_ind->num_setup_links)
		return 0;

	for (i = 0; i < sync_ind->num_setup_links; i++) {
		if (sync_ind->ml_link[i].vdev_id == vdev_id)
			return sync_ind->ml_link[i].link_id;
	}

	return 0;
}

bool is_multi_link_roam(struct roam_offload_synch_ind *sync_ind)
{
	if (!sync_ind)
		return false;

	if (sync_ind->num_setup_links)
		return true;

	return false;
}

QDF_STATUS mlo_enable_rso(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_vdev *assoc_vdev;

	if (wlan_vdev_mlme_is_mlo_link_vdev(vdev)) {
		assoc_vdev = wlan_mlo_get_assoc_link_vdev(vdev);
		if (!assoc_vdev) {
			mlo_err("Assoc vdev is null");
			return QDF_STATUS_E_NULL_VALUE;
		}

		cm_roam_start_init_on_connect(pdev,
					      wlan_vdev_get_id(assoc_vdev));
	}

	return QDF_STATUS_SUCCESS;
}

void
mlo_roam_copy_partner_info(struct wlan_cm_connect_resp *connect_rsp,
			   struct roam_offload_synch_ind *sync_ind)
{
	uint8_t i;
	struct mlo_partner_info *partner_info;

	if (!sync_ind)
		return;

	partner_info = &connect_rsp->ml_parnter_info;

	for (i = 0; i < sync_ind->num_setup_links; i++) {
		partner_info->partner_link_info[i].link_id =
			sync_ind->ml_link[i].link_id;
		qdf_copy_macaddr(
			&partner_info->partner_link_info[i].link_addr,
			&sync_ind->ml_link[i].link_addr);
	}
}

void
mlo_roam_update_connected_links(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_resp *connect_rsp)
{
	mlo_update_connected_links_bmap(vdev->mlo_dev_ctx,
					connect_rsp->ml_parnter_info);
}

QDF_STATUS
wlan_mlo_roam_abort_on_link(struct wlan_objmgr_psoc *psoc,
			    struct roam_offload_synch_ind *sync_ind)
{
	uint8_t i;
	QDF_STATUS status;

	for (i = 0; i < sync_ind->num_setup_links; i++)
		if (sync_ind->ml_link[i].vdev_id != sync_ind->roamed_vdev_id) {
			status = cm_fw_roam_abort_req(psoc,
						      sync_ind->roamed_vdev_id);
			if (QDF_IS_STATUS_ERROR(status))
				mlme_err("LFR3: Fail to abort roam on vdev: %u",
					 sync_ind->ml_link[i].vdev_id);
		}

	return QDF_STATUS_SUCCESS;
}

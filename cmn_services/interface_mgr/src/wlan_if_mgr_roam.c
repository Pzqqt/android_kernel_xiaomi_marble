/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: contains interface manager roam public api
 */
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_policy_mgr_i.h"
#include "wlan_if_mgr_roam.h"
#include "wlan_if_mgr_public_struct.h"
#include "wlan_cm_roam_api.h"
#include "wlan_if_mgr_main.h"
#include "wlan_p2p_ucfg_api.h"
#include "cds_api.h"
#include "sme_api.h"

static void if_mgr_enable_roaming_on_vdev(struct wlan_objmgr_pdev *pdev,
					  void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	struct change_roam_state_arg *roam_arg = arg;
	uint8_t vdev_id, curr_vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);
	curr_vdev_id = roam_arg->curr_vdev_id;

	if (curr_vdev_id != vdev_id &&
	    vdev->vdev_mlme.vdev_opmode == QDF_STA_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		ifmgr_debug("Roaming enabled on vdev_id %d", vdev_id);
		wlan_cm_enable_rso(pdev, vdev_id,
				   roam_arg->requestor,
				   REASON_DRIVER_ENABLED);
	}
}

QDF_STATUS if_mgr_enable_roaming(struct wlan_objmgr_vdev *vdev,
				 struct wlan_objmgr_pdev *pdev,
				 enum wlan_cm_rso_control_requestor requestor)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct change_roam_state_arg roam_arg;
	uint8_t vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);

	roam_arg.requestor = requestor;
	roam_arg.curr_vdev_id = vdev_id;

	status = wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
						if_mgr_enable_roaming_on_vdev,
						&roam_arg, 0,
						WLAN_IF_MGR_ID);

	return status;
}

static void if_mgr_disable_roaming_on_vdev(struct wlan_objmgr_pdev *pdev,
					   void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	struct change_roam_state_arg *roam_arg = arg;
	uint8_t vdev_id, curr_vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);
	curr_vdev_id = roam_arg->curr_vdev_id;

	if (curr_vdev_id != vdev_id &&
	    wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE &&
	    vdev->vdev_mlme.mlme_state == WLAN_VDEV_S_UP) {
		/* IFMGR Verification: Temporary call to sme_stop_roaming api,
		 * will be replaced by converged roaming api
		 * once roaming testing is complete.
		 */
		ifmgr_debug("Roaming disabled on vdev_id %d", vdev_id);
		wlan_cm_disable_rso(pdev, vdev_id,
				    roam_arg->requestor,
				    REASON_DRIVER_DISABLED);
	}
}

QDF_STATUS if_mgr_disable_roaming(struct wlan_objmgr_vdev *vdev,
				  struct wlan_objmgr_pdev *pdev,
				  enum wlan_cm_rso_control_requestor requestor)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct change_roam_state_arg roam_arg;
	uint8_t vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);

	roam_arg.requestor = requestor;
	roam_arg.curr_vdev_id = vdev_id;

	status = wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
						if_mgr_disable_roaming_on_vdev,
						&roam_arg, 0,
						WLAN_IF_MGR_ID);

	return status;
}

QDF_STATUS
if_mgr_enable_roaming_on_connected_sta(struct wlan_objmgr_vdev *vdev,
				       struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	vdev_id = wlan_vdev_get_id(vdev);

	if (policy_mgr_is_sta_active_connection_exists(psoc) &&
	    wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE) {
		ifmgr_debug("Enable roaming on connected sta for vdev_id %d", vdev_id);
		wlan_cm_enable_roaming_on_connected_sta(pdev, vdev_id);
		policy_mgr_set_pcl_for_connected_vdev(psoc, vdev_id, true);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS if_mgr_enable_roaming_after_p2p_disconnect(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_objmgr_pdev *pdev,
				enum wlan_cm_rso_control_requestor requestor)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc;
	struct change_roam_state_arg roam_arg;
	uint8_t vdev_id;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	vdev_id = wlan_vdev_get_id(vdev);

	roam_arg.requestor = requestor;
	roam_arg.curr_vdev_id = vdev_id;

	/*
	 * Due to audio share glitch with P2P clients due
	 * to roam scan on concurrent interface, disable
	 * roaming if "p2p_disable_roam" ini is enabled.
	 * Re-enable roaming again once the p2p client
	 * gets disconnected.
	 */
	if (ucfg_p2p_is_roam_config_disabled(psoc) &&
	    wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_CLIENT_MODE) {
		ifmgr_debug("P2P client disconnected, enable roam");
		status = wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					      if_mgr_enable_roaming_on_vdev,
					      &roam_arg, 0,
					      WLAN_IF_MGR_ID);
	}

	return status;
}

static void if_mgr_get_vdev_id_from_bssid(struct wlan_objmgr_pdev *pdev,
					  void *object, void *arg)
{
	struct bssid_search_arg *bssid_arg = arg;
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	struct wlan_objmgr_peer *peer;

	if (!(wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE ||
	      wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_CLIENT_MODE))
		return;

	/* Need to check the connection manager state when that becomes
	 * available
	 */
	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_UP)
		return;

	peer = wlan_objmgr_vdev_try_get_bsspeer(vdev, WLAN_IF_MGR_ID);
	if (!peer)
		return;

	if (WLAN_ADDR_EQ(bssid_arg->peer_addr.bytes,
			 wlan_peer_get_macaddr(peer)) == QDF_STATUS_SUCCESS)
		bssid_arg->vdev_id = wlan_vdev_get_id(vdev);

	wlan_objmgr_peer_release_ref(peer, WLAN_IF_MGR_ID);
}

QDF_STATUS if_mgr_validate_candidate(struct wlan_objmgr_vdev *vdev,
				     struct if_mgr_event_data *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	enum QDF_OPMODE op_mode;
	enum policy_mgr_con_mode mode;
	struct bssid_search_arg bssid_arg;
	uint32_t chan_freq = event_data->validate_bss_info.chan_freq;

	op_mode = wlan_vdev_mlme_get_opmode(vdev);

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Ignore the BSS if any other vdev is already connected to it.
	 */
	qdf_copy_macaddr(&bssid_arg.peer_addr,
			 &event_data->validate_bss_info.peer_addr);
	bssid_arg.vdev_id = WLAN_INVALID_VDEV_ID;
	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  if_mgr_get_vdev_id_from_bssid,
					  &bssid_arg, 0,
					  WLAN_IF_MGR_ID);

	if (bssid_arg.vdev_id != WLAN_INVALID_VDEV_ID) {
		ifmgr_info("vdev_id %d already connected to "QDF_MAC_ADDR_FMT". select next bss for vdev_id %d",
			   bssid_arg.vdev_id,
			   QDF_MAC_ADDR_REF(bssid_arg.peer_addr.bytes),
			   wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * If concurrency enabled take the concurrent connected channel first.
	 * Valid multichannel concurrent sessions exempted
	 */
	mode = policy_mgr_convert_device_mode_to_qdf_type(op_mode);
	/* If concurrency is not allowed select next bss */
	if (!policy_mgr_is_concurrency_allowed(psoc, mode, chan_freq,
					       HW_MODE_20_MHZ)) {
		ifmgr_info("Concurrency not allowed for this channel freq %d bssid "QDF_MAC_ADDR_FMT", selecting next",
			   chan_freq, QDF_MAC_ADDR_REF(bssid_arg.peer_addr.bytes));
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * check if channel is allowed for current hw mode, if not fetch
	 * next BSS.
	 */
	if (!policy_mgr_is_hwmode_set_for_given_chnl(psoc, chan_freq)) {
		ifmgr_info("HW mode isn't properly set, freq %d BSSID "QDF_MAC_ADDR_FMT,
			   chan_freq,
			   QDF_MAC_ADDR_REF(bssid_arg.peer_addr.bytes));
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

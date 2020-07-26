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
 * DOC: wlan_cm_roam_ucfg_api.c
 *
 * Implementation for roaming ucfg public functionality.
 */

#include "wlan_cm_roam_ucfg_api.h"

#ifdef ROAM_OFFLOAD_V1
QDF_STATUS
ucfg_user_space_enable_disable_rso(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   const bool is_fast_roam_enabled)
{
	bool supplicant_disabled_roaming;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	/*
	 * If the ini "FastRoamEnabled" is disabled, don't allow the
	 * userspace to enable roam offload
	 */
	ucfg_mlme_is_lfr_enabled(psoc, &lfr_enabled);
	if (!lfr_enabled) {
		mlme_legacy_debug("ROAM_CONFIG: Fast roam ini is disabled");
		if (!is_fast_roam_enabled)
			return QDF_STATUS_SUCCESS;

		return  QDF_STATUS_E_FAILURE;
	}

	/*
	 * Supplicant_disabled_roaming flag is the global flag to control
	 * roam offload from supplicant. Driver cannot enable roaming if
	 * supplicant disabled roaming is set.
	 * is_fast_roam_enabled: true - enable RSO if not disabled by driver
	 *                       false - Disable RSO. Send RSO stop if false
	 *                       is set.
	 */
	supplicant_disabled_roaming =
		mlme_get_supplicant_disabled_roaming(psoc, vdev_id);
	if (!is_fast_roam_enabled && supplicant_disabled_roaming) {
		mlme_legacy_debug("ROAM_CONFIG: RSO already disabled by supplicant");
		return QDF_STATUS_E_ALREADY;
	}

	mlme_set_supplicant_disabled_roaming(psoc, vdev_id,
					     !is_fast_roam_enabled);

	state = (is_fast_roam_enabled) ?
		WLAN_ROAM_RSO_ENABLED : WLAN_ROAM_RSO_STOPPED;
	status = cm_roam_state_change(pdev, vdev_id, state,
				      REASON_SUPPLICANT_DISABLED_ROAMING);

	return status;
}

/*
 * Driver internally invoked RSO operation/configuration APIs.
 */
QDF_STATUS
ucfg_cm_rso_init_deinit(struct wlan_objmgr_pdev *pdev,
			uint8_t vdev_id, bool enable)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	uint8_t reason = REASON_SUPPLICANT_DE_INIT_ROAMING;
	QDF_STATUS status;

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	if (enable)
		reason = REASON_SUPPLICANT_INIT_ROAMING;

	status = cm_roam_state_change(
			pdev, vdev_id,
			enable ? WLAN_ROAM_RSO_ENABLED : WLAN_ROAM_DEINIT,
			reason);
	cm_roam_release_lock();

	return status;
}

QDF_STATUS ucfg_cm_disable_rso(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id,
			       enum wlan_cm_rso_control_requestor requestor,
			       uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	if (reason == REASON_DRIVER_DISABLED && requestor)
		mlme_set_operations_bitmap(psoc, vdev_id, requestor, false);

	mlme_legacy_debug("ROAM_CONFIG: vdev[%d] Disable roaming - requestor:%s",
			  vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STOPPED,
				      REASON_DRIVER_DISABLED);
	cm_roam_release_lock();

	return status;
}

QDF_STATUS ucfg_cm_enable_rso(struct wlan_objmgr_pdev *pdev, uint32_t vdev_id,
			      enum wlan_cm_rso_control_requestor requestor,
			      uint8_t reason)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	QDF_STATUS status;

	if (reason == REASON_DRIVER_DISABLED && requestor)
		mlme_set_operations_bitmap(mac->psoc, vdev_id, requestor, true);

	status = cm_roam_acquire_lock();
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	mlme_legacy_debug("ROAM_CONFIG: vdev[%d] Enable roaming - requestor:%s",
			  vdev_id, cm_roam_get_requestor_string(requestor));

	status = cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STARTED,
				      REASON_DRIVER_ENABLED);
	cm_roam_release_lock();

	return status;
}
#endif

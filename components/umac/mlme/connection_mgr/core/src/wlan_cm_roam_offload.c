/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_roam_offload.c
 *
 * Implementation for the common roaming offload api interfaces.
 */

#include "wlan_cm_roam_offload.h"
#include "wlan_cm_tgt_if_tx_api.h"
#include "wlan_cm_roam_api.h"

/**
 * cm_roam_init_req() - roam init request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_init_req(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   bool enable)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_start_req() - roam start request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_start_req(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id,
				    uint8_t reason)
{
	struct wlan_roam_start_config *start_req;
	QDF_STATUS status;

	start_req = qdf_mem_malloc(sizeof(*start_req));
	if (!start_req)
		return QDF_STATUS_E_NOMEM;

	/* fill from legacy through this API */
	wlan_cm_roam_fill_start_req(psoc, vdev_id, start_req, reason);

	status = wlan_cm_tgt_send_roam_start_req(psoc, vdev_id, start_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("fail to send roam start");

	qdf_mem_free(start_req);

	return status;
}

/**
 * cm_roam_update_config_req() - roam update config request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_update_config_req(struct wlan_objmgr_psoc *psoc,
					    uint8_t vdev_id,
					    uint8_t reason)
{
	return QDF_STATUS_SUCCESS;
}

/*
 * similar to csr_roam_offload_scan, will be used from many legacy
 * process directly, generate a new function wlan_cm_roam_send_rso_cmd
 * for external usage.
 */
QDF_STATUS cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id,
				uint8_t rso_command,
				uint8_t reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = wlan_cm_roam_cmd_allowed(psoc, vdev_id, rso_command, reason);

	if (status == QDF_STATUS_E_NOSUPPORT)
		return QDF_STATUS_SUCCESS;
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug("ROAM: not allowed");
		return status;
	}
	if (rso_command == ROAM_SCAN_OFFLOAD_START)
		status = cm_roam_start_req(psoc, vdev_id, reason);
	else if (rso_command == ROAM_SCAN_OFFLOAD_UPDATE_CFG)
		status = cm_roam_update_config_req(psoc, vdev_id, reason);
//	else if (rso_command == ROAM_SCAN_OFFLOAD_RESTART)
		/* RESTART API */
//	else
		/* ABORT SCAN API */

	return status;
}

/**
 * cm_roam_stop_req() - roam stop request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_stop_req(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   uint8_t reason)
{
	/* do the filling as csr_post_rso_stop */
	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_rso_stop() - roam state handling for rso stop
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_RSO_STOPPED roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_switch_to_rso_stop(struct wlan_objmgr_pdev *pdev,
					     uint8_t vdev_id,
					     uint8_t reason)
{
	enum roam_offload_state cur_state;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cur_state = mlme_get_roam_state(psoc, vdev_id);
	switch (cur_state) {
	case WLAN_ROAM_RSO_ENABLED:
		status = cm_roam_stop_req(psoc, vdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err("ROAM: Unable to switch to RSO STOP State");
			return QDF_STATUS_E_FAILURE;
		}
		break;

	case WLAN_ROAM_DEINIT:
	case WLAN_ROAM_RSO_STOPPED:
	case WLAN_ROAM_INIT:
	/*
	 * Already the roaming module is initialized at fw,
	 * nothing to do here
	 */
	default:
		return QDF_STATUS_SUCCESS;
	}
	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_RSO_STOPPED);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_deinit() - roam state handling for roam deinit
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_DEINIT roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_switch_to_deinit(struct wlan_objmgr_pdev *pdev,
					   uint8_t vdev_id,
					   uint8_t reason)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);
	enum roam_offload_state cur_state = mlme_get_roam_state(psoc, vdev_id);

	switch (cur_state) {
	case WLAN_ROAM_RSO_ENABLED:
		cm_roam_switch_to_rso_stop(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_RSO_STOPPED:
	case WLAN_ROAM_INIT:
		break;

	case WLAN_ROAM_DEINIT:
	/*
	 * Already the roaming module is de-initialized at fw,
	 * do nothing here
	 */
	default:
		return QDF_STATUS_SUCCESS;
	}

	status = cm_roam_init_req(psoc, vdev_id, false);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_DEINIT);

	if (reason != REASON_SUPPLICANT_INIT_ROAMING)
		wlan_cm_enable_roaming_on_connected_sta(pdev, vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_init() - roam state handling for roam init
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_INIT roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_switch_to_init(struct wlan_objmgr_pdev *pdev,
					 uint8_t vdev_id,
					 uint8_t reason)
{
	enum roam_offload_state cur_state;
	uint8_t temp_vdev_id, roam_enabled_vdev_id;
	uint32_t roaming_bitmap;
	bool dual_sta_roam_active;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	dual_sta_roam_active =
		wlan_mlme_get_dual_sta_roaming_enabled(psoc);

	cur_state = mlme_get_roam_state(psoc, vdev_id);
	switch (cur_state) {
	case WLAN_ROAM_DEINIT:
		roaming_bitmap = mlme_get_roam_trigger_bitmap(psoc, vdev_id);
		if (!roaming_bitmap) {
			mlme_info("ROAM: Cannot change to INIT state for vdev[%d]",
				  vdev_id);
			return QDF_STATUS_E_FAILURE;
		}

		if (dual_sta_roam_active)
			break;
		/*
		 * Disable roaming on the enabled sta if supplicant wants to
		 * enable roaming on this vdev id
		 */
		temp_vdev_id = policy_mgr_get_roam_enabled_sta_session_id(
								psoc, vdev_id);
		if (temp_vdev_id != WLAN_UMAC_VDEV_ID_MAX) {
			/*
			 * Roam init state can be requested as part of
			 * initial connection or due to enable from
			 * supplicant via vendor command. This check will
			 * ensure roaming does not get enabled on this STA
			 * vdev id if it is not an explicit enable from
			 * supplicant.
			 */
			if (reason != REASON_SUPPLICANT_INIT_ROAMING) {
				mlme_info("ROAM: Roam module already initialized on vdev:[%d]",
					  temp_vdev_id);
				return QDF_STATUS_E_FAILURE;
			}
			cm_roam_state_change(pdev, temp_vdev_id,
					     WLAN_ROAM_DEINIT, reason);
		}
		break;

	case WLAN_ROAM_INIT:
	case WLAN_ROAM_RSO_STOPPED:
	case WLAN_ROAM_RSO_ENABLED:
	/*
	 * Already the roaming module is initialized at fw,
	 * just return from here
	 */
	default:
		return QDF_STATUS_SUCCESS;
	}

	status = cm_roam_init_req(psoc, vdev_id, true);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_INIT);

	roam_enabled_vdev_id =
		policy_mgr_get_roam_enabled_sta_session_id(psoc, vdev_id);

	/* Send PDEV pcl command if only one STA is in connected state
	 * If there is another STA connection exist, then set the
	 * PCL type to vdev level
	 */
	if (roam_enabled_vdev_id != WLAN_UMAC_VDEV_ID_MAX &&
	    dual_sta_roam_active)
		wlan_cm_roam_activate_pcl_per_vdev(psoc, vdev_id, true);

	/* Set PCL before sending RSO start */
	policy_mgr_set_pcl_for_existing_combo(psoc, PM_STA_MODE, vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_roam_switch_to_rso_start() - roam state handling for rso started
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function is used for WLAN_ROAM_RSO_ENABLED roam state handling
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS cm_roam_switch_to_rso_start(struct wlan_objmgr_pdev *pdev,
					      uint8_t vdev_id,
					      uint8_t reason)
{
	enum roam_offload_state cur_state;
	QDF_STATUS status;
	uint8_t control_bitmap;
	bool sup_disabled_roaming;
	bool rso_allowed;
	uint8_t rso_command = ROAM_SCAN_OFFLOAD_START;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	wlan_mlme_get_roam_scan_offload_enabled(psoc, &rso_allowed);

	cur_state = mlme_get_roam_state(psoc, vdev_id);
	switch (cur_state) {
	case WLAN_ROAM_INIT:
	case WLAN_ROAM_RSO_STOPPED:
		break;

	case WLAN_ROAM_DEINIT:
		status = cm_roam_switch_to_init(pdev, vdev_id, reason);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		break;
	case WLAN_ROAM_RSO_ENABLED:
		/*
		 * Send RSO update config if roaming already enabled
		 */
		rso_command = ROAM_SCAN_OFFLOAD_UPDATE_CFG;
		break;
	default:
		return QDF_STATUS_SUCCESS;
	}

	if (!rso_allowed) {
		mlme_debug("ROAM: RSO disabled via INI");
		return QDF_STATUS_E_FAILURE;
	}

	control_bitmap = mlme_get_operations_bitmap(psoc, vdev_id);
	if (control_bitmap) {
		mlme_debug("ROAM: RSO Disabled internaly: vdev[%d] bitmap[0x%x]",
			   vdev_id, control_bitmap);
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_roam_send_rso_cmd(psoc, vdev_id, rso_command, reason);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_debug("ROAM: RSO start failed");
		return status;
	}
	mlme_set_roam_state(psoc, vdev_id, WLAN_ROAM_RSO_ENABLED);

	/*
	 * If supplicant disabled roaming, driver does not send
	 * RSO cmd to fw. This causes roam invoke to fail in FW
	 * since RSO start never happened at least once to
	 * configure roaming engine in FW.
	 */
	sup_disabled_roaming = mlme_get_supplicant_disabled_roaming(psoc,
								    vdev_id);
	if (!sup_disabled_roaming)
		return QDF_STATUS_SUCCESS;

	mlme_debug("ROAM: RSO disabled by Supplicant on vdev[%d]", vdev_id);
	return cm_roam_state_change(pdev, vdev_id, WLAN_ROAM_RSO_STOPPED,
				    REASON_SUPPLICANT_DISABLED_ROAMING);
}

QDF_STATUS cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
				uint8_t vdev_id,
				enum roam_offload_state requested_state,
				uint8_t reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	bool is_up;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return status;

	is_up = QDF_IS_STATUS_SUCCESS(wlan_vdev_is_up(vdev));
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	if (requested_state != WLAN_ROAM_DEINIT && !is_up) {
		mlme_debug("ROAM: roam state change requested in disconnected state");
		return status;
	}

	switch (requested_state) {
	case WLAN_ROAM_DEINIT:
		status = cm_roam_switch_to_deinit(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_INIT:
		status = cm_roam_switch_to_init(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_RSO_ENABLED:
		status = cm_roam_switch_to_rso_start(pdev, vdev_id, reason);
		break;
	case WLAN_ROAM_RSO_STOPPED:
		status = cm_roam_switch_to_rso_stop(pdev, vdev_id, reason);
		break;
	default:
		mlme_debug("ROAM: Invalid roam state %d", requested_state);
		break;
	}

	return status;
}

/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_vdev_mgr_wake_lock.c
 *
 * This file provide definition for APIs related to wake lock
 */
#include "wlan_objmgr_vdev_obj.h"
#include "include/wlan_vdev_mlme.h"
#include "qdf_lock.h"
#include "target_if_vdev_mgr_wake_lock.h"
#include "wlan_lmac_if_def.h"
#include "host_diag_core_event.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if.h>
#include "target_if_vdev_mgr_rx_ops.h"

void target_if_wake_lock_init(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_wakelock *vdev_wakelock;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (!vdev) {
		target_if_err(" VDEV is NULL");
		return;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("PSOC is NULL");
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_wakelock_info) {
		target_if_err("No Rx Ops");
		return;
	}

	vdev_wakelock = rx_ops->vdev_mgr_get_wakelock_info(vdev);

	qdf_wake_lock_create(&vdev_wakelock->start_wakelock, "vdev_start");
	qdf_wake_lock_create(&vdev_wakelock->stop_wakelock, "vdev_stop");
	qdf_wake_lock_create(&vdev_wakelock->delete_wakelock, "vdev_delete");

	qdf_runtime_lock_init(&vdev_wakelock->wmi_cmd_rsp_runtime_lock);
}

void target_if_wake_lock_deinit(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_wakelock *vdev_wakelock;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (!vdev) {
		target_if_err(" VDEV is NULL");
		return;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("PSOC is NULL");
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_wakelock_info) {
		target_if_err("No Rx Ops");
		return;
	}

	vdev_wakelock = rx_ops->vdev_mgr_get_wakelock_info(vdev);

	qdf_wake_lock_destroy(&vdev_wakelock->start_wakelock);
	qdf_wake_lock_destroy(&vdev_wakelock->stop_wakelock);
	qdf_wake_lock_destroy(&vdev_wakelock->delete_wakelock);

	qdf_runtime_lock_deinit(&vdev_wakelock->wmi_cmd_rsp_runtime_lock);
}

QDF_STATUS target_if_wake_lock_timeout_acquire(
				struct wlan_objmgr_vdev *vdev,
				enum wakelock_mode mode)
{
	struct vdev_mlme_wakelock *vdev_wakelock;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (!vdev) {
		target_if_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("PSOC is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_wakelock_info) {
		target_if_err("No Rx Ops");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_wakelock = rx_ops->vdev_mgr_get_wakelock_info(vdev);

	switch (mode) {
	case START_WAKELOCK:
		qdf_wake_lock_timeout_acquire(&vdev_wakelock->start_wakelock,
					      START_RESPONSE_TIMER);
		break;
	case STOP_WAKELOCK:
		qdf_wake_lock_timeout_acquire(&vdev_wakelock->stop_wakelock,
					      STOP_RESPONSE_TIMER);
		break;
	case DELETE_WAKELOCK:
		qdf_wake_lock_timeout_acquire(&vdev_wakelock->delete_wakelock,
					      DELETE_RESPONSE_TIMER);
		break;
	default:
		target_if_err("operation mode is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_runtime_pm_prevent_suspend(
				&vdev_wakelock->wmi_cmd_rsp_runtime_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_wake_lock_timeout_release(
				struct wlan_objmgr_vdev *vdev,
				enum wakelock_mode mode)
{
	struct vdev_mlme_wakelock *vdev_wakelock;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (!vdev) {
		target_if_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("PSOC is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->vdev_mgr_get_wakelock_info) {
		target_if_err("No Rx Ops");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_wakelock = rx_ops->vdev_mgr_get_wakelock_info(vdev);

	switch (mode) {
	case START_WAKELOCK:
		qdf_wake_lock_release(&vdev_wakelock->start_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	case STOP_WAKELOCK:
		qdf_wake_lock_release(&vdev_wakelock->stop_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	case DELETE_WAKELOCK:
		qdf_wake_lock_release(&vdev_wakelock->delete_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	default:
		target_if_err("operation mode is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_runtime_pm_allow_suspend(&vdev_wakelock->wmi_cmd_rsp_runtime_lock);

	return QDF_STATUS_SUCCESS;
}


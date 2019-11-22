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
 * DOC: target_if_psoc_wake_lock.c
 *
 * This file provide definition for APIs related to wake lock
 */

#include "qdf_lock.h"
#include <target_if_psoc_wake_lock.h>
#include <wlan_lmac_if_def.h>
#include <host_diag_core_event.h>
#include <wlan_objmgr_psoc_obj.h>
#include <target_if.h>

void target_if_wake_lock_init(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_wakelock_info) {
		mlme_err("vdev_id:%d psoc_id:%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);

	qdf_wake_lock_create(&psoc_wakelock->start_wakelock, "vdev_start");
	qdf_wake_lock_create(&psoc_wakelock->stop_wakelock, "vdev_stop");
	qdf_wake_lock_create(&psoc_wakelock->delete_wakelock, "vdev_delete");

	qdf_runtime_lock_init(&psoc_wakelock->wmi_cmd_rsp_runtime_lock);
}

void target_if_wake_lock_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_wakelock_info) {
		mlme_err("vdev_id:%d psoc_id:%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);

	qdf_wake_lock_destroy(&psoc_wakelock->start_wakelock);
	qdf_wake_lock_destroy(&psoc_wakelock->stop_wakelock);
	qdf_wake_lock_destroy(&psoc_wakelock->delete_wakelock);

	qdf_runtime_lock_deinit(&vdev_wakelock->wmi_cmd_rsp_runtime_lock);
}

QDF_STATUS target_if_wake_lock_timeout_acquire(
				struct wlan_objmgr_psoc *psoc,
				enum wakelock_mode mode)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_wakelock_info) {
		mlme_err("vdev_id:%d psoc_id:%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);
	switch (mode) {
	case START_WAKELOCK:
		qdf_wake_lock_timeout_acquire(&psoc_wakelock->start_wakelock,
					      START_RESPONSE_TIMER);
		break;
	case STOP_WAKELOCK:
		qdf_wake_lock_timeout_acquire(&psoc_wakelock->stop_wakelock,
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
				&psoc_wakelock->wmi_cmd_rsp_runtime_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_wake_lock_timeout_release(
				struct wlan_objmgr_psoc *psoc,
				enum wakelock_mode mode)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_wakelock_info) {
		mlme_err("vdev_id:%d psoc_id:%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);
	switch (mode) {
	case START_WAKELOCK:
		qdf_wake_lock_release(&psoc_wakelock->start_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	case STOP_WAKELOCK:
		qdf_wake_lock_release(&psoc_wakelock->stop_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	case DELETE_WAKELOCK:
		qdf_wake_lock_release(&psoc_wakelock->delete_wakelock,
				      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
		break;
	default:
		target_if_err("operation mode is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_runtime_pm_allow_suspend(&psoc_wakelock->wmi_cmd_rsp_runtime_lock);

	return QDF_STATUS_SUCCESS;
}


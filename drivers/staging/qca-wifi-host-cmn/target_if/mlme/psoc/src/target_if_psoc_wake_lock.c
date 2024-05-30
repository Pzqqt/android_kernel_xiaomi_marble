/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
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
#include <target_if_vdev_mgr_rx_ops.h>
#include <wlan_reg_services_api.h>
#include "init_deinit_lmac.h"

void target_if_wake_lock_init(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops",
			 wlan_psoc_get_id(psoc));
		return;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);

	qdf_wake_lock_create(&psoc_wakelock->start_wakelock, "vdev_start");
	qdf_wake_lock_create(&psoc_wakelock->stop_wakelock, "vdev_stop");
	qdf_wake_lock_create(&psoc_wakelock->delete_wakelock, "vdev_delete");

	qdf_runtime_lock_init(&psoc_wakelock->wmi_cmd_rsp_runtime_lock);
	qdf_runtime_lock_init(&psoc_wakelock->prevent_runtime_lock);
	psoc_wakelock->is_link_up = false;
}

void target_if_wake_lock_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops",
			 wlan_psoc_get_id(psoc));
		return;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);

	qdf_wake_lock_destroy(&psoc_wakelock->start_wakelock);
	qdf_wake_lock_destroy(&psoc_wakelock->stop_wakelock);
	qdf_wake_lock_destroy(&psoc_wakelock->delete_wakelock);

	qdf_runtime_lock_deinit(&psoc_wakelock->wmi_cmd_rsp_runtime_lock);
	qdf_runtime_lock_deinit(&psoc_wakelock->prevent_runtime_lock);
}

QDF_STATUS target_if_wake_lock_timeout_acquire(
				struct wlan_objmgr_psoc *psoc,
				enum wakelock_mode mode)
{
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops",
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
		qdf_wake_lock_timeout_acquire(&psoc_wakelock->delete_wakelock,
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
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops", wlan_psoc_get_id(psoc));
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

static void
target_if_vote_for_link_down(struct wlan_objmgr_psoc *psoc,
			     struct psoc_mlme_wakelock *psoc_wakelock)
{
	void *htc_handle;

	htc_handle = lmac_get_htc_hdl(psoc);
	if (!htc_handle) {
		mlme_err("HTC handle is NULL");
		return;
	}

	if (psoc_wakelock->is_link_up) {
		htc_vote_link_down(htc_handle, HTC_LINK_VOTE_SAP_DFS_USER_ID);
		qdf_runtime_pm_allow_suspend(&psoc_wakelock->prevent_runtime_lock);
		psoc_wakelock->is_link_up = false;
	}
}

static void
target_if_vote_for_link_up(struct wlan_objmgr_psoc *psoc,
			   struct psoc_mlme_wakelock *psoc_wakelock)
{
	void *htc_handle;

	htc_handle = lmac_get_htc_hdl(psoc);
	if (!htc_handle) {
		mlme_err("HTC handle is NULL");
		return;
	}

	if (!psoc_wakelock->is_link_up) {
		htc_vote_link_up(htc_handle, HTC_LINK_VOTE_SAP_DFS_USER_ID);
		qdf_runtime_pm_prevent_suspend(&psoc_wakelock->prevent_runtime_lock);
		psoc_wakelock->is_link_up = true;
	}
}

void target_if_vdev_start_link_handler(struct wlan_objmgr_vdev *vdev,
				       bool is_restart)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct wlan_channel *curr_channel, *prev_channel;
	uint32_t ch_freq, prev_ch_freq;
	enum phy_ch_width ch_width, prev_ch_width;
	uint32_t is_dfs, prev_ch_is_dfs;
	enum channel_state ch_state, prev_ch_state;

	psoc = wlan_vdev_get_psoc(vdev);
	pdev = wlan_vdev_get_pdev(vdev);

	if (!pdev) {
		mlme_err("pdev is NULL");
		return;
	}

	curr_channel = wlan_vdev_mlme_get_des_chan(vdev);
	ch_freq = curr_channel->ch_freq;
	ch_width = curr_channel->ch_width;
	is_dfs = wlan_reg_is_dfs_for_freq(pdev, ch_freq);
	ch_state = wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, ch_freq,
								 ch_width);
	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops",
			 wlan_psoc_get_id(psoc));
		return;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE) {
		if (is_restart) {
			prev_channel = wlan_vdev_mlme_get_bss_chan(vdev);
			prev_ch_freq = prev_channel->ch_freq;
			prev_ch_width = prev_channel->ch_width;
			prev_ch_is_dfs = wlan_reg_is_dfs_for_freq(pdev,
								  prev_ch_freq);
			prev_ch_state =
				wlan_reg_get_5g_bonded_channel_state_for_freq(pdev,
						prev_ch_freq, prev_ch_width);
			/*
			 * In restart case, if SAP is on non DFS channel and
			 * previously it was on DFS channel then vote for link
			 * down.
			 */
			if ((prev_ch_is_dfs ||
			     prev_ch_state == CHANNEL_STATE_DFS) &&
			     !(is_dfs || ch_state == CHANNEL_STATE_DFS))
				target_if_vote_for_link_down(psoc,
							     psoc_wakelock);

			/*
			 * If SAP is on DFS channel and previously it was on
			 * non DFS channel then vote for link up
			 */
			if (!(prev_ch_is_dfs ||
			      prev_ch_state == CHANNEL_STATE_DFS) &&
			     (is_dfs || ch_state == CHANNEL_STATE_DFS))
				target_if_vote_for_link_up(psoc, psoc_wakelock);
		} else if (is_dfs || ch_state == CHANNEL_STATE_DFS)
			target_if_vote_for_link_up(psoc, psoc_wakelock);
	}
}

void target_if_vdev_stop_link_handler(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct psoc_mlme_wakelock *psoc_wakelock;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct wlan_channel *curr_channel;
	uint32_t ch_freq;
	enum phy_ch_width ch_width;
	uint32_t is_dfs;

	psoc = wlan_vdev_get_psoc(vdev);
	pdev = wlan_vdev_get_pdev(vdev);

	if (!pdev) {
		mlme_err("pdev is NULL");
		return;
	}

	curr_channel = wlan_vdev_mlme_get_bss_chan(vdev);
	ch_freq = curr_channel->ch_freq;
	ch_width = curr_channel->ch_width;
	is_dfs = wlan_reg_is_dfs_for_freq(pdev, ch_freq);

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->psoc_get_wakelock_info) {
		mlme_err("psoc_id:%d No Rx Ops",
			 wlan_psoc_get_id(psoc));
		return;
	}

	psoc_wakelock = rx_ops->psoc_get_wakelock_info(psoc);
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE)
		if (is_dfs ||
		    (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev,
			ch_freq, ch_width) == CHANNEL_STATE_DFS))
			target_if_vote_for_link_down(psoc, psoc_wakelock);
}


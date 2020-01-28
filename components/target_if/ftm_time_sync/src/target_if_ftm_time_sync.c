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

/**
 * DOC: Target interface file for ftm time sync component to
 * Implement api's which shall be used by ftm time sync component
 * in target_if internally.
 */

#include "target_if.h"
#include "target_if_ftm_time_sync.h"
#include "wlan_ftm_time_sync_public_struct.h"
#include <wmi_unified_api.h>

static QDF_STATUS
target_if_ftm_time_sync_send_qtime(struct wlan_objmgr_psoc *psoc,
				   uint32_t vdev_id, uint64_t lpass_ts)
{
	wmi_unified_t wmi_hdl;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_send_wlan_time_sync_qtime(wmi_hdl, vdev_id,
						     lpass_ts);
}

static QDF_STATUS
target_if_ftm_time_sync_send_trigger(struct wlan_objmgr_psoc *psoc,
				     uint32_t vdev_id, bool mode)
{
	wmi_unified_t wmi_hdl;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_send_wlan_time_sync_ftm_trigger(wmi_hdl, vdev_id,
							   mode);
}

static int
target_if_time_sync_ftm_start_stop_event_handler(ol_scn_t scn_handle,
						 uint8_t *data, uint32_t len)
{
	struct ftm_time_sync_start_stop_params param;
	struct wlan_objmgr_psoc *psoc;
	wmi_unified_t wmi_handle;

	if (!data) {
		target_if_err("%s: invalid pointer", __func__);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return -EINVAL;
	}

	if (wmi_unified_extract_time_sync_ftm_start_stop_params(
			wmi_handle, data, &param) != QDF_STATUS_SUCCESS) {
		target_if_err("Extraction of time sync ftm start stop failed");
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS
target_if_ftm_time_sync_start_stop_event(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	wmi_unified_t wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event(
			wmi_handle, wmi_wlan_time_sync_ftm_start_stop_event_id,
			target_if_time_sync_ftm_start_stop_event_handler);
	if (status) {
		target_if_err("Ftm timesync start stop event register failed");
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

static int
target_if_time_sync_master_slave_offset_event_handler(ol_scn_t scn_handle,
						      uint8_t *data,
						      uint32_t len)
{
	struct ftm_time_sync_offset param;
	struct wlan_objmgr_psoc *psoc;
	wmi_unified_t wmi_handle;

	if (!data) {
		target_if_err("%s: invalid pointer", __func__);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return -EINVAL;
	}

	if (wmi_unified_extract_time_sync_ftm_offset(
			wmi_handle, data, &param) != QDF_STATUS_SUCCESS) {
		target_if_err("Extraction of timesync ftm offset param failed");
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS
target_if_ftm_time_sync_master_slave_offset(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	wmi_unified_t wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event(
			wmi_handle,
			wmi_wlan_time_sync_q_master_slave_offset_eventid,
			target_if_time_sync_master_slave_offset_event_handler);
	if (status) {
		target_if_err("Ftm timesync offset event register failed");
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

void
target_if_ftm_time_sync_register_rx_ops(struct wlan_ftm_timesync_rx_ops *rx_ops)
{
	if (!rx_ops) {
		target_if_err("FTM timesync rx_ops is null");
		return;
	}

	rx_ops->ftm_timesync_register_start_stop =
				target_if_ftm_time_sync_start_stop_event;
	rx_ops->ftm_timesync_regiser_master_slave_offset =
				target_if_ftm_time_sync_master_slave_offset;
}

void
target_if_ftm_time_sync_register_tx_ops(struct wlan_ftm_timesync_tx_ops *tx_ops)
{
	if (!tx_ops) {
		target_if_err("FTM timesync tx_ops is null");
		return;
	}

	tx_ops->ftm_time_sync_send_qtime = target_if_ftm_time_sync_send_qtime;
	tx_ops->ftm_time_sync_send_trigger =
				target_if_ftm_time_sync_send_trigger;
}


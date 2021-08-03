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

/**
 *  DOC: target_if_mgmt_txrx_rx_reo.c
 *  This file contains definitions of management rx re-ordering related APIs.
 */

#include <wlan_objmgr_psoc_obj.h>
#include <qdf_status.h>
#include <target_if.h>
#include <wlan_mgmt_txrx_rx_reo_public_structs.h>
#include <target_if_mgmt_txrx_rx_reo.h>

/**
 * target_if_mgmt_rx_reo_fw_consumed_event_handler() - WMI event handler to
 * process MGMT Rx FW consumed event handler
 * @scn: Pointer to scn object
 * @data_buf: Pointer to event buffer
 * @data_len: Length of event buffer
 *
 * Return: 0 for success, else failure
 */
static int
target_if_mgmt_rx_reo_fw_consumed_event_handler(
	ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	QDF_STATUS status;
	struct mgmt_rx_reo_params params;
	struct wlan_lmac_if_mgmt_rx_reo_rx_ops *mgmt_rx_reo_rx_ops;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		mgmt_rx_reo_err("null psoc");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mgmt_rx_reo_err("wmi_handle is NULL");
		return -EINVAL;
	}

	status = wmi_extract_mgmt_rx_fw_consumed(wmi_handle, data, &params);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to extract mgmt rx params");
		return -EINVAL;
	}

	mgmt_rx_reo_rx_ops = target_if_mgmt_rx_reo_get_rx_ops(psoc);
	if (!mgmt_rx_reo_rx_ops) {
		mgmt_rx_reo_err("rx_ops of MGMT Rx REO module is NULL");
		return -EINVAL;
	}

	if (!mgmt_rx_reo_rx_ops->fw_consumed_event_handler) {
		mgmt_rx_reo_err("FW consumed event handler is NULL");
		return -EINVAL;
	}

	status = mgmt_rx_reo_rx_ops->fw_consumed_event_handler(psoc, &params);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("FW consumed event handling failed");
		return -EINVAL;
	}

	return 0;
}

QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;
	QDF_STATUS status;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mgmt_rx_reo_err("Invalid WMI handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_mgmt_rx_fw_consumed_eventid,
			target_if_mgmt_rx_reo_fw_consumed_event_handler,
			WMI_RX_UMAC_CTX);

	if (QDF_IS_STATUS_ERROR(status))
		mgmt_rx_reo_err("Registering for MGMT Rx FW consumed event failed");

	return status;
}

QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;
	QDF_STATUS status;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mgmt_rx_reo_err("Invalid WMI handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_unregister_event_handler(
			wmi_handle,
			wmi_mgmt_rx_fw_consumed_eventid);

	if (QDF_IS_STATUS_ERROR(status))
		mgmt_rx_reo_err("Unregistering for MGMT Rx FW consumed event failed");

	return status;
}

/**
 * target_if_mgmt_rx_reo_read_snapshot() - Read management rx-reorder snapshot
 * @snapshot_address: Snapshot address
 * @id: Snapshot ID
 * @snapshot_value: Pointer to snapshot value
 *
 * Read management rx-reorder snapshots from target.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_mgmt_rx_reo_read_snapshot(
			struct mgmt_rx_reo_snapshot *snapshot_address,
			enum mgmt_rx_reo_snapshot_id id,
			struct mgmt_rx_reo_snapshot_params *snapshot_value)
{
	bool snapshot_valid;
	uint16_t mgmt_pkt_ctr;
	uint16_t redundant_mgmt_pkt_ctr;
	uint32_t global_timestamp;
	uint32_t mgmt_rx_reo_snapshot_low;
	uint32_t mgmt_rx_reo_snapshot_high;
	uint8_t retry_count;
	QDF_STATUS status;

	if (!snapshot_address) {
		mgmt_rx_reo_err("Mgmt Rx REO snapshot address null");
		return QDF_STATUS_E_INVAL;
	}

	if (!snapshot_value) {
		mgmt_rx_reo_err("Mgmt Rx REO snapshot null");
		return QDF_STATUS_E_INVAL;
	}

	switch (id) {
	case MGMT_RX_REO_SNAPSHOT_MAC_HW:
	case MGMT_RX_REO_SNAPSHOT_FW_CONSUMED:
	case MGMT_RX_REO_SNAPSHOT_FW_FORWADED:
		retry_count = 0;
		for (; retry_count < MGMT_RX_REO_SNAPSHOT_READ_RETRY_LIMIT;
		     retry_count++) {
			mgmt_rx_reo_snapshot_low =
				snapshot_address->mgmt_rx_reo_snapshot_low;
			mgmt_rx_reo_snapshot_high =
				snapshot_address->mgmt_rx_reo_snapshot_high;

			snapshot_valid = REO_SNAPSHOT_GET_VALID(
						mgmt_rx_reo_snapshot_low);
			if (!snapshot_valid) {
				mgmt_rx_reo_debug("Invalid REO snapshot value");
				snapshot_value->valid = false;
				return QDF_STATUS_SUCCESS;
			}

			global_timestamp = REO_SNAPSHOT_GET_GLOBAL_TIMESTAMP(
						mgmt_rx_reo_snapshot_low,
						mgmt_rx_reo_snapshot_high);
			mgmt_pkt_ctr = REO_SNAPSHOT_GET_MGMT_PKT_CTR(
						mgmt_rx_reo_snapshot_low);
			redundant_mgmt_pkt_ctr =
					REO_SNAPSHOT_GET_REDUNDANT_MGMT_PKT_CTR(
					mgmt_rx_reo_snapshot_high);
			if (REO_SNAPSHOT_IS_CONSISTENT(
			    mgmt_pkt_ctr, redundant_mgmt_pkt_ctr))
				break;

			mgmt_rx_reo_info("Inconsistent snapshot value low=0x%x high=0x%x",
					 mgmt_rx_reo_snapshot_low,
					 mgmt_rx_reo_snapshot_high);
		}

		if (retry_count == MGMT_RX_REO_SNAPSHOT_READ_RETRY_LIMIT) {
			mgmt_rx_reo_debug("Read retry limit reached");
			snapshot_value->valid = false;
			return QDF_STATUS_SUCCESS;
		}

		snapshot_value->valid = true;
		snapshot_value->mgmt_pkt_ctr = mgmt_pkt_ctr;
		snapshot_value->global_timestamp = global_timestamp;
		status = QDF_STATUS_SUCCESS;
		break;

	default:
		mgmt_rx_reo_err("Invalid snapshot id %d", id);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

QDF_STATUS
target_if_mgmt_rx_reo_tx_ops_register(
			struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_tx_ops;

	if (!mgmt_txrx_tx_ops) {
		mgmt_rx_reo_err("mgmt_txrx txops NULL");
		return QDF_STATUS_E_FAILURE;
	}
	mgmt_rx_reo_tx_ops = &mgmt_txrx_tx_ops->mgmt_rx_reo_tx_ops;
	mgmt_rx_reo_tx_ops->read_mgmt_rx_reo_snapshot =
				target_if_mgmt_rx_reo_read_snapshot;

	return QDF_STATUS_SUCCESS;
}

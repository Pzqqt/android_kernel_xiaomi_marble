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
 *  DOC: wlan_mgmt_txrx_rx_reo_tgt_api.h
 *  This file contains mgmt rx re-ordering tgt layer related APIs
 */

#ifndef _WLAN_MGMT_TXRX_RX_REO_TGT_API_H
#define _WLAN_MGMT_TXRX_RX_REO_TGT_API_H
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_types.h>
#include <wlan_mgmt_txrx_rx_reo_public_structs.h>
#include <wlan_mgmt_txrx_rx_reo_utils_api.h>

#ifdef WLAN_MGMT_RX_REO_SUPPORT
/**
 * tgt_mgmt_rx_reo_read_snapshot() - Read management rx-reorder snapshot
 * @pdev: Pointer to pdev object
 * @address: Snapshot address
 * @id: Snapshot ID
 * @value: Pointer to the snapshot value where the snapshot
 * should be written
 *
 * Read management rx-reorder snapshots from target.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_mgmt_rx_reo_read_snapshot(struct wlan_objmgr_pdev *pdev,
			      struct mgmt_rx_reo_snapshot *address,
			      enum mgmt_rx_reo_snapshot_id id,
			      struct mgmt_rx_reo_snapshot_params *value);
/**
 * tgt_mgmt_rx_reo_fw_consumed_event_handler() - MGMT Rx REO FW consumed
 * event handler
 * @psoc: Pointer to psoc objmgr
 * @params: Pointer to MGMT Rx REO parameters
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
tgt_mgmt_rx_reo_fw_consumed_event_handler(struct wlan_objmgr_psoc *psoc,
					  struct mgmt_rx_reo_params *params);

/**
 * tgt_mgmt_rx_reo_filter_config() - Configure MGMT Rx REO filter
 * @pdev: Pointer to pdev object
 * @filter: Pointer to MGMT Rx REO filter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS tgt_mgmt_rx_reo_filter_config(struct wlan_objmgr_pdev *pdev,
					 struct mgmt_rx_reo_filter *filter);
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_TGT_API_H */

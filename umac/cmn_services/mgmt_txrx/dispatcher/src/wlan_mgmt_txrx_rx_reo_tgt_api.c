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
 *  DOC: wlan_mgmt_txrx_rx_re_tgt_api.c
 *  This file contains mgmt rx re-ordering tgt layer related function
 *  definitions
 */
#include <wlan_mgmt_txrx_rx_reo_tgt_api.h>
#include "../../core/src/wlan_mgmt_txrx_rx_reo_i.h"

QDF_STATUS
tgt_mgmt_rx_reo_read_snapshot(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_rx_reo_snapshot *address,
			enum mgmt_rx_reo_shared_snapshot_id id,
			struct mgmt_rx_reo_snapshot_params *value)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_txops;

	mgmt_rx_reo_txops = wlan_pdev_get_mgmt_rx_reo_txops(pdev);
	if (!mgmt_rx_reo_txops) {
		mgmt_rx_reo_err("mgmt rx reo txops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!mgmt_rx_reo_txops->read_mgmt_rx_reo_snapshot) {
		mgmt_rx_reo_err("mgmt rx reo read snapshot txops is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mgmt_rx_reo_txops->read_mgmt_rx_reo_snapshot(pdev, address, id,
							    value);
}

/**
 * tgt_mgmt_rx_reo_enter_algo_without_buffer() - Entry point to the MGMT Rx REO
 * algorithm when there is no frame buffer
 * @pdev: pdev for which this frame/event is intended
 * @reo_params: MGMT Rx REO parameters corresponding to this frame/event
 * @type: Type of the MGMT Rx REO frame/event descriptor
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
tgt_mgmt_rx_reo_enter_algo_without_buffer(
				struct wlan_objmgr_pdev *pdev,
				struct mgmt_rx_reo_params *reo_params,
				enum mgmt_rx_reo_frame_descriptor_type type)
{
	struct mgmt_rx_event_params mgmt_rx_params;
	struct mgmt_rx_reo_frame_descriptor desc;
	bool is_frm_queued;
	QDF_STATUS status;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!reo_params) {
		mgmt_rx_reo_err("mgmt rx reo params are null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mgmt_rx_params.reo_params = reo_params;

	desc.nbuf = NULL; /* No frame buffer */
	desc.rx_params = &mgmt_rx_params;
	desc.type = type;

	/** If REO is not required for this descriptor,
	 *  no need to proceed further
	 */
	if (!is_mgmt_rx_reo_required(pdev, &desc))
		return  QDF_STATUS_SUCCESS;

	/* Enter the REO algorithm */
	status = wlan_mgmt_rx_reo_algo_entry(pdev, &desc, &is_frm_queued);

	qdf_assert_always(!is_frm_queued);

	return status;
}

QDF_STATUS
tgt_mgmt_rx_reo_fw_consumed_event_handler(struct wlan_objmgr_pdev *pdev,
					  struct mgmt_rx_reo_params *params)
{
	return tgt_mgmt_rx_reo_enter_algo_without_buffer(
			pdev, params, MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME);
}

QDF_STATUS
tgt_mgmt_rx_reo_host_drop_handler(struct wlan_objmgr_pdev *pdev,
				  struct mgmt_rx_reo_params *params)
{
	return tgt_mgmt_rx_reo_enter_algo_without_buffer(
			pdev, params, MGMT_RX_REO_FRAME_DESC_ERROR_FRAME);
}

QDF_STATUS tgt_mgmt_rx_reo_filter_config(struct wlan_objmgr_pdev *pdev,
					 struct mgmt_rx_reo_filter *filter)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_txops;

	mgmt_rx_reo_txops = wlan_pdev_get_mgmt_rx_reo_txops(pdev);
	if (!mgmt_rx_reo_txops) {
		mgmt_rx_reo_err("MGMT Rx REO txops is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mgmt_rx_reo_txops->mgmt_rx_reo_filter_config) {
		mgmt_rx_reo_err("mgmt_rx_reo_filter_config is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mgmt_rx_reo_txops->mgmt_rx_reo_filter_config(pdev, filter);
}

QDF_STATUS
tgt_mgmt_rx_reo_get_snapshot_address(
			struct wlan_objmgr_pdev *pdev,
			enum mgmt_rx_reo_shared_snapshot_id id,
			struct mgmt_rx_reo_snapshot **address)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_txops;

	mgmt_rx_reo_txops = wlan_pdev_get_mgmt_rx_reo_txops(pdev);
	if (!mgmt_rx_reo_txops) {
		mgmt_rx_reo_err("mgmt rx reo txops is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mgmt_rx_reo_txops->get_mgmt_rx_reo_snapshot_address) {
		mgmt_rx_reo_err("txops entry for get snapshot address is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mgmt_rx_reo_txops->get_mgmt_rx_reo_snapshot_address(pdev, id,
								   address);
}

QDF_STATUS tgt_mgmt_rx_reo_frame_handler(
				struct wlan_objmgr_pdev *pdev,
				qdf_nbuf_t buf,
				struct mgmt_rx_event_params *mgmt_rx_params)
{
	QDF_STATUS status;
	struct mgmt_rx_reo_frame_descriptor desc;
	bool is_queued;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto cleanup;
	}

	if (!buf) {
		mgmt_rx_reo_err("nbuf is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto cleanup;
	}

	if (!mgmt_rx_params) {
		mgmt_rx_reo_err("MGMT rx params is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto cleanup;
	}

	/* Populate frame descriptor */
	desc.type = MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME;
	desc.nbuf = buf;
	desc.rx_params = mgmt_rx_params;

	/* If REO is not required for this frame, process it right away */
	if (!is_mgmt_rx_reo_required(pdev, &desc)) {
		return tgt_mgmt_txrx_process_rx_frame(pdev, buf,
						      mgmt_rx_params);
	}

	status = wlan_mgmt_rx_reo_algo_entry(pdev, &desc, &is_queued);

	/* If frame is queued, we shouldn't free up params and buf pointers */
	if (is_queued)
		return status;
cleanup:
	qdf_nbuf_free(buf);
	free_mgmt_rx_event_params(mgmt_rx_params);

	return status;
}

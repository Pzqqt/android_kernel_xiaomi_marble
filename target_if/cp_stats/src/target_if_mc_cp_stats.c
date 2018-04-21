/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: target_if_cp_stats.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <qdf_mem.h>
#include <qdf_status.h>
#include <target_if_cp_stats.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_param.h>
#include <target_if.h>
#include <wlan_tgt_def_config.h>
#include <wmi_unified_api.h>
#include <wlan_osif_priv.h>
#include <wlan_cp_stats_utils_api.h>
#include <wlan_cp_stats_mc_tgt_api.h>

#define TGT_INVALID_SNR         (0)
#define TGT_NOISE_FLOOR_DBM     (-96)
#define TGT_MAX_SNR             (TGT_NOISE_FLOOR_DBM * (-1))
#define TGT_IS_VALID_SNR(x)     ((x) >= 0 && (x) < TGT_MAX_SNR)

static void target_if_cp_stats_free_stats_event(struct stats_event *ev)
{
	qdf_mem_free(ev->pdev_stats);
	ev->pdev_stats = NULL;
}

static QDF_STATUS target_if_cp_stats_extract_pdev_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev,
					uint8_t *data)
{
	uint32_t i;
	QDF_STATUS status;
	wmi_host_pdev_stats pdev_stats;

	ev->num_pdev_stats = stats_param->num_pdev_stats;
	if (!ev->num_pdev_stats)
		return QDF_STATUS_SUCCESS;

	/*
	 * num_pdev_stats is validated within function wmi_extract_stats_param
	 * which is called to populated wmi_host_stats_event stats_param
	 */
	ev->pdev_stats = qdf_mem_malloc(sizeof(*ev->pdev_stats) *
						ev->num_pdev_stats);
	if (!ev->pdev_stats) {
		cp_stats_err("malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	for (i = 0; i < ev->num_pdev_stats; i++) {
		status = wmi_extract_pdev_stats(wmi_hdl, data, i, &pdev_stats);
		if (QDF_IS_STATUS_ERROR(status)) {
			cp_stats_err("wmi_extract_pdev_stats failed");
			return status;
		}
		ev->pdev_stats[i].max_pwr = pdev_stats.chan_tx_pwr;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_cp_stats_extract_event(struct wmi_unified *wmi_hdl,
						   struct stats_event *ev,
						   uint8_t *data)
{
	QDF_STATUS status;
	wmi_host_stats_event stats_param = {0};

	status = wmi_extract_stats_param(wmi_hdl, data, &stats_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_debug("stats param extract failed: %d", status);
		return status;
	}
	cp_stats_debug("num: pdev: %d, vdev: %d, peer: %d, rssi: %d",
		       stats_param.num_pdev_stats, stats_param.num_vdev_stats,
		       stats_param.num_peer_stats, stats_param.num_rssi_stats);

	status = target_if_cp_stats_extract_pdev_stats(wmi_hdl, &stats_param,
						       ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mc_cp_stats_stats_event_handler() - function to handle stats event
 * from firmware.
 * @scn: scn handle
 * @data: data buffer for event
 * @datalen: data length
 *
 * Return: status of operation.
 */
static int target_if_mc_cp_stats_stats_event_handler(ol_scn_t scn,
						     uint8_t *data,
						     uint32_t datalen)
{
	QDF_STATUS status;
	struct stats_event ev = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_cp_stats_rx_ops *rx_ops;

	if (!scn || !data) {
		cp_stats_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		cp_stats_err("null psoc");
		return -EINVAL;
	}
	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	rx_ops = target_if_cp_stats_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->process_stats_event) {
		cp_stats_err("callback not registered");
		return -EINVAL;
	}

	status = target_if_cp_stats_extract_event(wmi_handle, &ev, data);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("extract event failed");
		goto end;
	}

	status = rx_ops->process_stats_event(psoc, &ev);

end:
	target_if_cp_stats_free_stats_event(&ev);
	return qdf_status_to_os_return(status);
}

static void target_if_cp_stats_inc_wake_lock_stats(uint32_t reason,
					struct wake_lock_stats *stats,
					uint32_t *unspecified_wake_count)
{
	switch (reason) {
	case WOW_REASON_UNSPECIFIED:
		(*unspecified_wake_count)++;
		break;
	case WOW_REASON_RA_MATCH:
		stats->ipv6_mcast_wake_up_count++;
		stats->ipv6_mcast_ra_stats++;
		stats->icmpv6_count++;
		break;
	case WOW_REASON_NLOD:
		stats->pno_match_wake_up_count++;
		break;
	case WOW_REASON_NLO_SCAN_COMPLETE:
		stats->pno_complete_wake_up_count++;
		break;
	case WOW_REASON_LOW_RSSI:
		stats->low_rssi_wake_up_count++;
		break;
	case WOW_REASON_EXTSCAN:
		stats->gscan_wake_up_count++;
		break;
	case WOW_REASON_RSSI_BREACH_EVENT:
		stats->rssi_breach_wake_up_count++;
		break;
	case WOW_REASON_OEM_RESPONSE_EVENT:
		stats->oem_response_wake_up_count++;
	case WOW_REASON_11D_SCAN:
		stats->scan_11d++;
		break;
	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
		stats->pwr_save_fail_detected++;
		break;
	default:
		break;
	}
}

static QDF_STATUS
target_if_cp_stats_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_register_event_handler(GET_WMI_HDL_FROM_PSOC(psoc),
			wmi_update_stats_event_id,
			target_if_mc_cp_stats_stats_event_handler,
			WMI_RX_WORK_CTX);
	if (status) {
		cp_stats_err("Failed to register stats event cb");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
target_if_cp_stats_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(GET_WMI_HDL_FROM_PSOC(psoc),
					     wmi_update_stats_event_id);

	return QDF_STATUS_SUCCESS;
}

static uint32_t get_stats_id(enum stats_req_type type)
{
	switch (type) {
	default:
		break;
	case TYPE_CONNECTION_TX_POWER:
		return WMI_REQUEST_PDEV_STAT;
	}
	return 0;
}

/**
 * target_if_cp_stats_send_stats_req() - API to send stats request to wmi
 * @psoc: pointer to psoc object
 * @req: pointer to object containing stats request parameters
 *
 * Return: status of operation.
 */
static QDF_STATUS target_if_cp_stats_send_stats_req(
					struct wlan_objmgr_psoc *psoc,
					enum stats_req_type type,
					struct request_info *req)

{
	struct wmi_unified *wmi_handle;
	struct stats_request_params param = {0};

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}
	/* refer  (WMI_REQUEST_STATS_CMDID) */
	param.stats_id = get_stats_id(type);
	param.vdev_id = req->vdev_id;
	param.pdev_id = req->pdev_id;
	return wmi_unified_stats_request_send(wmi_handle, req->peer_mac_addr,
					      &param);
}

QDF_STATUS
target_if_cp_stats_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_cp_stats_tx_ops *cp_stats_tx_ops;

	if (!tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	cp_stats_tx_ops = &tx_ops->cp_stats_tx_ops;
	if (!cp_stats_tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	cp_stats_tx_ops->cp_stats_attach =
		target_if_cp_stats_register_event_handler;
	cp_stats_tx_ops->cp_stats_detach =
		target_if_cp_stats_unregister_event_handler;
	cp_stats_tx_ops->inc_wake_lock_stats =
		target_if_cp_stats_inc_wake_lock_stats;
	cp_stats_tx_ops->send_req_stats = target_if_cp_stats_send_stats_req;
	return QDF_STATUS_SUCCESS;
}


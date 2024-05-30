/*
 * Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <wlan_objmgr_peer_obj.h>
#ifdef WLAN_FEATURE_MIB_STATS
#include <wlan_cp_stats_mc_defs.h>
#endif
#include "wlan_cp_stats_defs.h"
#include "cdp_txrx_cmn_struct.h"
#include "cdp_txrx_ctrl.h"
#include "wlan_cp_stats_comp_handler.h"

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS

uint32_t get_infra_cp_stats_id(enum infra_cp_stats_id type)
{
	switch (type) {
	case TYPE_REQ_CTRL_PATH_PDEV_TX_STAT:
		return WMI_REQUEST_CTRL_PATH_PDEV_TX_STAT;
	case TYPE_REQ_CTRL_PATH_VDEV_EXTD_STAT:
		return WMI_REQUEST_CTRL_PATH_VDEV_EXTD_STAT;
	case TYPE_REQ_CTRL_PATH_MEM_STAT:
		return WMI_REQUEST_CTRL_PATH_MEM_STAT;
	case TYPE_REQ_CTRL_PATH_TWT_STAT:
		return WMI_REQUEST_CTRL_PATH_TWT_STAT;
	case TYPE_REQ_CTRL_PATH_BMISS_STAT:
		return WMI_REQUEST_CTRL_PATH_BMISS_STAT;
	default:
		return -EINVAL;
	}
}

uint32_t get_infra_cp_stats_action(enum infra_cp_stats_action action)
{
	switch (action) {
	case ACTION_REQ_CTRL_PATH_STAT_GET:
		return WMI_REQUEST_CTRL_PATH_STAT_GET;
	case ACTION_REQ_CTRL_PATH_STAT_RESET:
		return WMI_REQUEST_CTRL_PATH_STAT_RESET;
	case ACTION_REQ_CTRL_PATH_STAT_START:
		return WMI_REQUEST_CTRL_PATH_STAT_START;
	case ACTION_REQ_CTRL_PATH_STAT_STOP:
		return WMI_REQUEST_CTRL_PATH_STAT_STOP;
	default:
		return -EINVAL;
	}
}

#ifdef WLAN_SUPPORT_TWT
/**
 * target_if_infra_cp_stats_twt_event_free() - Free event buffer
 * @ev: pointer to infra cp stats event structure
 *
 * Return: None
 */
static
void target_if_infra_cp_stats_twt_event_free(struct infra_cp_stats_event *ev)
{
	qdf_mem_free(ev->twt_infra_cp_stats);
	ev->twt_infra_cp_stats = NULL;
}

/**
 * target_if_infra_cp_stats_twt_event_alloc() - Allocate event buffer for TWT
 * parameters
 * @ev: pointer to infra cp stats event structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
static QDF_STATUS
target_if_infra_cp_stats_twt_event_alloc(struct infra_cp_stats_event *ev)
{
	ev->twt_infra_cp_stats =
			qdf_mem_malloc(sizeof(*ev->twt_infra_cp_stats) *
			INFRA_CP_STATS_MAX_RESP_TWT_DIALOG_ID);
	if (!ev->twt_infra_cp_stats) {
		cp_stats_err("mem alloc failed for ev.twt_infra_cp_stats");
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

#else
static inline
void target_if_infra_cp_stats_twt_event_free(struct infra_cp_stats_event *ev)
{
}

static inline QDF_STATUS
target_if_infra_cp_stats_twt_event_alloc(struct infra_cp_stats_event *ev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void target_if_infra_cp_stats_free_stats_event(struct infra_cp_stats_event *ev)
{
}
#endif /* WLAN_SUPPORT_TWT */

#ifdef CONFIG_WLAN_BMISS

/**
 * target_if_infra_cp_stats_bmiss_event_free() - Free event buffer
 * @ev: pointer to infra cp stats event structure
 *
 * Return: None
 */
static
void target_if_infra_cp_stats_bmiss_event_free(struct infra_cp_stats_event *ev)
{
	qdf_mem_free(ev->bmiss_infra_cp_stats);
	ev->bmiss_infra_cp_stats = NULL;
}

/**
 * target_if_infra_cp_stats_bmiss_event_alloc() - Allocate buffer for bmiss
 * parameters
 * @ev: pointer to infra cp stats event structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
static QDF_STATUS
target_if_infra_cp_stats_bmiss_event_alloc(struct infra_cp_stats_event *ev)
{
	ev->bmiss_infra_cp_stats =
	qdf_mem_malloc(sizeof(*ev->bmiss_infra_cp_stats));
	if (!ev->bmiss_infra_cp_stats) {
		cp_stats_err("mem alloc failed for ev.bmiss_infra_cp_stats");
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}
#else

static inline
void target_if_infra_cp_stats_bmiss_event_free(struct infra_cp_stats_event *ev)
{
}

static inline QDF_STATUS
target_if_infra_cp_stats_bmiss_event_alloc(struct infra_cp_stats_event *ev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* CONFIG_WLAN_BMISS */

/**
 * target_if_infra_cp_stats_event_free() - Free event buffer
 * @ev: pointer to infra cp stats event structure
 *
 * Return : None
 */
static
void target_if_infra_cp_stats_event_free(struct infra_cp_stats_event *ev)
{
	target_if_infra_cp_stats_twt_event_free(ev);
	target_if_infra_cp_stats_bmiss_event_free(ev);
}

/**
 * target_if_infra_cp_stats_event_alloc() - Allocate buffer for event
 * parameters
 * @ev: pointer to infra cp stats event structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes on
 * failure
 */
static QDF_STATUS
target_if_infra_cp_stats_event_alloc(struct infra_cp_stats_event *ev)
{
	QDF_STATUS status;

	status = target_if_infra_cp_stats_twt_event_alloc(ev);
	if (status)
		return QDF_STATUS_E_NOMEM;

	status = target_if_infra_cp_stats_bmiss_event_alloc(ev);
	if (status)
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_extract_infra_cp_stats_event() - Extract data from stats event
 * @wmi_hdl: WMI Handle
 * @data: pointer to event data buffer from firmware
 * @data_len: length of the data buffer
 * @ev: pointer of output structure to be filled with extracted values
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
static QDF_STATUS
target_if_extract_infra_cp_stats_event(struct wmi_unified *wmi_hdl,
				       uint8_t *data, uint32_t data_len,
				       struct infra_cp_stats_event *ev)
{
	QDF_STATUS status;
	uint32_t more_flag = 0;

	status = wmi_unified_extract_cp_stats_more_pending(wmi_hdl, data,
							   &more_flag);

	status = wmi_unified_extract_infra_cp_stats(wmi_hdl, data,
						    data_len, ev);

	cp_stats_debug("request_id %d", ev->request_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_infra_cp_stats_event_handler() - Handle
 * wmi_pdev_cp_fwstats_eventid
 * @scn: opaque scn handle
 * @data: event buffer received from fw
 * @datalen: length of event buffer
 *
 * Return: 0 for success or non zero error codes for failure
 */
static
int target_if_infra_cp_stats_event_handler(ol_scn_t scn, uint8_t *data,
					   uint32_t datalen)
{
	QDF_STATUS status;
	struct infra_cp_stats_event ev = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_cp_stats_rx_ops *rx_ops;

	cp_stats_debug("Enter");

	if (!scn || !data) {
		cp_stats_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		cp_stats_err("null psoc");
		return -EINVAL;
	}

	rx_ops = target_if_cp_stats_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->process_stats_event) {
		cp_stats_err("callback not registered");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return -EINVAL;
	}
	status = target_if_infra_cp_stats_event_alloc(&ev);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Alloc event mem failed");
		goto end;
	}

	status = target_if_extract_infra_cp_stats_event(wmi_handle, data,
							datalen, &ev);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("extract event failed");
		goto end;
	}

	status = rx_ops->process_infra_stats_event(psoc, &ev);

end:
	target_if_infra_cp_stats_event_free(&ev);
	return qdf_status_to_os_return(status);
}
#else
static
int target_if_infra_cp_stats_event_handler(ol_scn_t scn, uint8_t *data,
					   uint32_t datalen)
{
	return 0;
}
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */

#if defined(WLAN_SUPPORT_INFRA_CTRL_PATH_STATS) && defined(WLAN_SUPPORT_TWT) && \
	defined(WLAN_TWT_CONV_SUPPORTED)
static int
target_if_twt_session_params_event_handler(ol_scn_t scn,
					   uint8_t *evt_buf,
					   uint32_t evt_data_len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_peer *peer_obj;
	struct wmi_unified *wmi_hdl;
	struct twt_session_stats_info twt_params;
	struct twt_session_stats_event_param params = {0};
	struct peer_cp_stats *peer_cp_stats;
	uint32_t expected_len;
	int i;
	QDF_STATUS status;
	uint32_t ev;
	cdp_config_param_type val = {0};
	ol_txrx_soc_handle soc_txrx_handle;
	struct wlan_lmac_if_rx_ops *rx_ops;
	wmi_pdev_twt_session_stats_event_fixed_param stats_param;

	TARGET_IF_ENTER();

	if (!scn || !evt_buf) {
		target_if_err("scn: 0x%pK, evt_buf: 0x%pK", scn, evt_buf);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc object is null!");
		return -EINVAL;
	}

	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	if (!soc_txrx_handle)
		return -EINVAL;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl) {
		target_if_err("wmi_handle is null!");
		return -EINVAL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		target_if_err("No valid twt session stats rx ops");
		return -EINVAL;
	}

	status = wmi_extract_twt_session_stats_event(wmi_hdl, evt_buf, &params);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Could not extract twt session stats event");
		return qdf_status_to_os_return(status);
	}

	if (params.num_sessions > TWT_PEER_MAX_SESSIONS) {
		target_if_err("Number of twt sessions exceeded, num:%d max:%d",
			      params.num_sessions, TWT_PEER_MAX_SESSIONS);
		return -EINVAL;
	}
	expected_len = (sizeof(wmi_pdev_twt_session_stats_event_fixed_param) +
			sizeof(stats_param.tlv_header) + (params.num_sessions *
			sizeof(wmi_twt_session_stats_info)));

	if (evt_data_len < expected_len) {
		target_if_err("Got invalid len of data from FW %d expected %d",
			      evt_data_len, expected_len);
		return -EINVAL;
	}

	for (i = 0; i < params.num_sessions; i++) {
		status = wmi_extract_twt_session_stats_data(wmi_hdl, evt_buf,
							    &params,
							    &twt_params, i);

		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("Unable to extract twt params for idx %d",
				      i);
			return -EINVAL;
		}
		peer_obj = wlan_objmgr_get_peer_by_mac(psoc,
						twt_params.peer_mac.bytes,
						WLAN_CP_STATS_ID);
		if (!peer_obj) {
			target_if_err("peer obj not found for "QDF_MAC_ADDR_FMT,
				      QDF_MAC_ADDR_REF(twt_params.peer_mac.bytes));
			continue;
		}

		ev = twt_params.event_type;
		if (ev == HOST_TWT_SESSION_SETUP)
			val.cdp_peer_param_in_twt = 1;
		else if (ev == HOST_TWT_SESSION_TEARDOWN)
			val.cdp_peer_param_in_twt = 0;

		cdp_txrx_set_peer_param(soc_txrx_handle, twt_params.vdev_id,
					twt_params.peer_mac.bytes,
					CDP_CONFIG_IN_TWT, val);

		peer_cp_stats = wlan_cp_stats_get_peer_stats_obj(peer_obj);
		if (!peer_cp_stats) {
			target_if_err("peer_cp_stats is null");
			continue;
		}

		wlan_cp_stats_peer_obj_lock(peer_cp_stats);

		rx_ops->cp_stats_rx_ops.twt_get_session_param_resp(psoc,
								 &twt_params);

		wlan_cp_stats_peer_obj_unlock(peer_cp_stats);
		wlan_objmgr_peer_release_ref(peer_obj, WLAN_CP_STATS_ID);
	}
	return 0;
}

static QDF_STATUS
target_if_cp_stats_register_twt_session_event(struct wmi_unified *wmi_handle)
{
	QDF_STATUS ret_val;

	ret_val = wmi_unified_register_event_handler(wmi_handle,
				wmi_twt_session_stats_event_id,
				target_if_twt_session_params_event_handler,
				WMI_RX_WORK_CTX);

	return ret_val;
}

static void
target_if_cp_stats_unregister_twt_session_event(struct wmi_unified *wmi_handle)
{
	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_twt_session_stats_event_id);
}
#else
static QDF_STATUS
target_if_cp_stats_register_twt_session_event(struct wmi_unified *wmi_handle)
{
	return QDF_STATUS_SUCCESS;
}

static void
target_if_cp_stats_unregister_twt_session_event(struct wmi_unified *wmi_handle)
{
}
#endif /*WLAN_SUPPORT_INFRA_CTRL_PATH_STATS && WLAN_SUPPORT_TWT && WLAN_TWT_CONV_SUPPORTED*/

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
static QDF_STATUS
target_if_cp_stats_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;
	QDF_STATUS ret_val;

	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	ret_val = wmi_unified_register_event_handler(wmi_handle,
			    wmi_pdev_cp_fwstats_eventid,
			    target_if_infra_cp_stats_event_handler,
			    WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(ret_val)) {
		cp_stats_err("Failed to register for pdev_cp_fwstats_event");
		return ret_val;
	}

	ret_val = target_if_cp_stats_register_twt_session_event(wmi_handle);
	if (QDF_IS_STATUS_ERROR(ret_val)) {
		cp_stats_err("Failed to register twt session stats event");
		return ret_val;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
target_if_cp_stats_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_pdev_cp_fwstats_eventid);
	target_if_cp_stats_unregister_twt_session_event(wmi_handle);
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
target_if_cp_stats_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
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

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
/**
 * target_if_infra_cp_stats_req() - API to send stats request to wmi
 * @psoc: pointer to psoc object
 * @req: pointer to object containing stats request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
static
QDF_STATUS target_if_infra_cp_stats_req(struct wlan_objmgr_psoc *psoc,
					struct infra_cp_stats_cmd_info *req)

{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_infra_cp_stats_request_send(wmi_handle, req);
}

static void target_if_register_infra_cp_stats_txops(
				struct wlan_lmac_if_cp_stats_tx_ops *tx_ops)
{
	tx_ops->send_req_infra_cp_stats = target_if_infra_cp_stats_req;
}
#else
static void target_if_register_infra_cp_stats_txops(
				struct wlan_lmac_if_cp_stats_tx_ops *tx_ops)
{
}
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */

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
	target_if_register_infra_cp_stats_txops(cp_stats_tx_ops);

	cp_stats_tx_ops->cp_stats_attach =
		target_if_cp_stats_register_event_handler;
	cp_stats_tx_ops->cp_stats_detach =
		target_if_cp_stats_unregister_event_handler;
	cp_stats_tx_ops->cp_stats_legacy_attach =
		target_if_cp_stats_register_legacy_event_handler;
	cp_stats_tx_ops->cp_stats_legacy_detach =
		target_if_cp_stats_unregister_legacy_event_handler;
	return QDF_STATUS_SUCCESS;
}


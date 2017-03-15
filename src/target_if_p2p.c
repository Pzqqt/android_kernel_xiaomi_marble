/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: offload lmac interface APIs definitions for P2P
 */

#include <wmi_unified_api.h>
#include <wlan_p2p_public_struct.h>
#include "target_if.h"
#include "target_if_p2p.h"

static inline struct wlan_lmac_if_p2p_rx_ops *
target_if_psoc_get_p2p_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &(psoc->soc_cb.rx_ops.p2p);
}

/**
 * target_p2p_lo_event_handler() - WMI callback for lo stop event
 * @scn:
 * @event_buf: event buffer
 * @len: buffer length
 *
 * This function gets called from WMI when triggered wmi event
 * wmi_p2p_lo_stop_event_id.
 *
 * Return: 0 - success
 * others - failure
 */
static int target_p2p_lo_event_handler(ol_scn_t scn, uint8_t *data,
	uint32_t datalen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct p2p_lo_event *event_info;
	struct wlan_lmac_if_p2p_rx_ops *p2p_rx_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	target_if_info("scn:%p, data:%p, datalen:%d", scn, data, datalen);

	if (!scn || !data) {
		target_if_err("scn: 0x%p, data: 0x%p", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		target_if_err("null wmi handle");
		return -EINVAL;
	}

	event_info = qdf_mem_malloc(sizeof(*event_info));
	if (!event_info) {
		target_if_err("Failed to allocate p2p lo event");
		return -ENOMEM;
	}

	/*TODO, extract wmi lo stop event */

	p2p_rx_ops = target_if_psoc_get_p2p_rx_ops(psoc);
	if (p2p_rx_ops->lo_ev_handler) {
		status = p2p_rx_ops->lo_ev_handler(psoc, event_info);
		target_if_debug("call lo event handler, status:%d",
			status);
	}

	return qdf_status_to_os_return(status);
}

/**
 * target_p2p_noa_event_handler() - WMI callback for noa event
 * @scn:
 * @event_buf: event buffer
 * @len: buffer length
 *
 * This function gets called from WMI when triggered WMI event
 * wmi_p2p_noa_event_id.
 *
 * Return: 0 - success
 * others - failure
 */
static int target_p2p_noa_event_handler(ol_scn_t scn, uint8_t *data,
	uint32_t datalen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct p2p_noa_info *event_info;
	struct wlan_lmac_if_p2p_rx_ops *p2p_rx_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	target_if_info("scn:%p, data:%p, datalen:%d", scn, data, datalen);

	if (!scn || !data) {
		target_if_err("scn: 0x%p, data: 0x%p", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		target_if_err("null wmi handle");
		return -EINVAL;
	}

	event_info = qdf_mem_malloc(sizeof(*event_info));
	if (!event_info) {
		target_if_err("Failed to allocate p2p noa information");
		return -ENOMEM;
	}

	/*TODO, extract wmi noa event */

	p2p_rx_ops = target_if_psoc_get_p2p_rx_ops(psoc);
	if (p2p_rx_ops->noa_ev_handler) {
		status = p2p_rx_ops->noa_ev_handler(psoc, event_info);
		target_if_debug("call noa event handler, status:%d",
			status);
	}

	return qdf_status_to_os_return(status);
}

QDF_STATUS target_if_p2p_register_lo_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	int status;
	wmi_unified_t wmi_handle = wlan_psoc_get_tgt_if_handle(psoc);

	target_if_info("psoc:%p, arg:%p", psoc, arg);

	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event_handler(wmi_handle,
		wmi_p2p_lo_stop_event_id, target_p2p_lo_event_handler,
		WMI_RX_UMAC_CTX);

	target_if_info("wmi register lo event handle, status:%d",
		status);

	return status == 0 ? QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

QDF_STATUS target_if_p2p_register_noa_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	int status;
	wmi_unified_t wmi_handle = wlan_psoc_get_tgt_if_handle(psoc);

	target_if_info("psoc:%p, arg:%p", psoc, arg);

	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event_handler(wmi_handle,
		wmi_p2p_noa_event_id, target_p2p_noa_event_handler,
		WMI_RX_UMAC_CTX);

	target_if_info("wmi register noa event handle, status:%d",
		status);

	return status == 0 ? QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

QDF_STATUS target_if_p2p_unregister_lo_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	int status;
	wmi_unified_t wmi_handle = wlan_psoc_get_tgt_if_handle(psoc);

	target_if_info("psoc:%p, arg:%p", psoc, arg);

	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
		wmi_p2p_lo_stop_event_id);

	target_if_info("wmi unregister lo event handle, status:%d",
		status);

	return status == 0 ? QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

QDF_STATUS target_if_p2p_unregister_noa_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	int status;
	wmi_unified_t wmi_handle = wlan_psoc_get_tgt_if_handle(psoc);

	target_if_info("psoc:%p, arg:%p", psoc, arg);

	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
		wmi_p2p_noa_event_id);

	target_if_info("wmi unregister noa event handle, status:%d",
		status);

	return status == 0 ? QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

QDF_STATUS target_if_p2p_set_ps(struct wlan_objmgr_psoc *psoc,
	struct p2p_ps_config *ps_config)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_p2p_lo_start(struct wlan_objmgr_psoc *psoc,
	struct p2p_lo_start *lo_start)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_p2p_lo_stop(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

void target_if_p2p_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_tx_ops;

	if (!tx_ops) {
		target_if_err("lmac tx_ops is null");
		return;
	}

	p2p_tx_ops = &tx_ops->p2p;
	p2p_tx_ops->set_ps = target_if_p2p_set_ps;
	p2p_tx_ops->lo_start = target_if_p2p_lo_start;
	p2p_tx_ops->lo_stop = target_if_p2p_lo_stop;
	p2p_tx_ops->reg_lo_ev_handler =
			target_if_p2p_register_lo_event_handler;
	p2p_tx_ops->reg_noa_ev_handler =
			target_if_p2p_register_noa_event_handler;
	p2p_tx_ops->unreg_lo_ev_handler =
			target_if_p2p_unregister_lo_event_handler;
	p2p_tx_ops->unreg_noa_ev_handler =
			target_if_p2p_unregister_noa_event_handler;
}

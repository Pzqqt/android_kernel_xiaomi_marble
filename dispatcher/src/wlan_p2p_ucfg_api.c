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
 * DOC: This file contains p2p north bound interface definitions
 */

#include <wmi_unified_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <scheduler_api.h>
#include "wlan_p2p_ucfg_api.h"
#include "wlan_p2p_public_struct.h"
#include "../../core/src/wlan_p2p_main.h"

static inline struct wlan_lmac_if_p2p_tx_ops *
ucfg_p2p_psoc_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &(psoc->soc_cb.tx_ops.p2p);
}

QDF_STATUS ucfg_p2p_init(void)
{
	return p2p_component_init();
}

QDF_STATUS ucfg_p2p_deinit(void)
{
	return p2p_component_deinit();
}

QDF_STATUS ucfg_p2p_psoc_open(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_object_open(soc);
}

QDF_STATUS ucfg_p2p_psoc_close(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_object_close(soc);
}

QDF_STATUS ucfg_p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req)
{
	return p2p_psoc_start(soc, req);
}

QDF_STATUS ucfg_p2p_psoc_stop(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_stop(soc);
}

QDF_STATUS ucfg_p2p_roc_req(struct wlan_objmgr_psoc *soc,
	struct p2p_roc_req *roc_req, uint64_t *cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_roc_cancel_req(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_mgmt_tx(struct wlan_objmgr_psoc *soc,
	struct p2p_mgmt_tx *mgmt_frm, uint64_t *cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_mgmt_tx_cancel(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_set_ps(struct wlan_objmgr_psoc *soc,
	struct p2p_ps_config *ps_config)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_debug("soc:%p, vdev_id:%d, opp_ps:%d, ct_window:%d, count:%d, duration:%d, duration:%d, ps_selection:%d",
		soc, ps_config->vdev_id, ps_config->opp_ps,
		ps_config->ct_window, ps_config->count,
		ps_config->duration, ps_config->single_noa_duration,
		ps_config->ps_selection);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->set_ps) {
		status = p2p_ops->set_ps(soc, ps_config);
		p2p_debug("p2p set ps, status:%d", status);
	}

	return status;
}

QDF_STATUS ucfg_p2p_lo_start(struct wlan_objmgr_psoc *soc,
	struct p2p_lo_start *p2p_lo_start)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_debug("soc:%p, vdev_id:%d, ctl_flags:%d, freq:%d, period:%d, interval:%d, count:%d, dev_types_len:%d, probe_resp_len:%d, device_types:%p, probe_resp_tmplt:%p",
		soc, p2p_lo_start->vdev_id, p2p_lo_start->ctl_flags,
		p2p_lo_start->freq, p2p_lo_start->period,
		p2p_lo_start->interval, p2p_lo_start->count,
		p2p_lo_start->dev_types_len, p2p_lo_start->probe_resp_len,
		p2p_lo_start->device_types, p2p_lo_start->probe_resp_tmplt);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->lo_start) {
		status = p2p_ops->lo_start(soc, p2p_lo_start);
		p2p_debug("p2p lo start, status:%d", status);
	}

	return status;
}

QDF_STATUS ucfg_p2p_lo_stop(struct wlan_objmgr_psoc *soc,
	uint32_t vdev_id)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_debug("soc:%p, vdev_id:%d", soc, vdev_id);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->lo_stop) {
		status = p2p_ops->lo_stop(soc, vdev_id);
		p2p_debug("p2p lo stop, status:%d", status);
	}

	return status;
}

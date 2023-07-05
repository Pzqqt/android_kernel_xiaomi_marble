/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Add host 11d scan utility functions
 */

#include <wlan_scan_public_structs.h>
#include <wlan_scan_ucfg_api.h>

#include "reg_priv_objs.h"

#include "reg_host_11d.h"
#include "reg_services_common.h"

static QDF_STATUS reg_11d_scan_trigger_handler(
	struct wlan_regulatory_psoc_priv_obj *soc_reg)
{
	struct scan_start_request *req;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
		soc_reg->psoc_ptr,
		soc_reg->vdev_id_for_11d_scan,
		WLAN_REGULATORY_SB_ID);
	if (!vdev) {
		reg_err("vdev object is NULL id %d",
			soc_reg->vdev_id_for_11d_scan);
		qdf_mem_free(req);
		return QDF_STATUS_E_FAILURE;
	}

	ucfg_scan_init_default_params(vdev, req);

	req->scan_req.scan_id = ucfg_scan_get_scan_id(soc_reg->psoc_ptr);
	if (!req->scan_req.scan_id) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_REGULATORY_SB_ID);
		qdf_mem_free(req);
		reg_err("Invalid scan ID");
		return QDF_STATUS_E_FAILURE;
	}
	soc_reg->scan_id = req->scan_req.scan_id;
	req->scan_req.vdev_id = soc_reg->vdev_id_for_11d_scan;
	req->scan_req.scan_req_id = soc_reg->scan_req_id;
	req->scan_req.scan_priority = SCAN_PRIORITY_LOW;
	req->scan_req.scan_f_passive = false;

	status = ucfg_scan_start(req);
	reg_nofl_debug("11d scan trigger vdev %d scan_id %d req_id %d status %d",
		       soc_reg->vdev_id_for_11d_scan, soc_reg->scan_id,
		       soc_reg->scan_req_id, status);

	if (status != QDF_STATUS_SUCCESS)
		/* Don't free req here, ucfg_scan_start will do free */
		reg_err("11d scan req failed vdev %d",
			soc_reg->vdev_id_for_11d_scan);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_REGULATORY_SB_ID);

	return status;
}

static void reg_11d_scan_event_cb(
	struct wlan_objmgr_vdev *vdev,
	struct scan_event *event, void *arg)
{
};

QDF_STATUS reg_11d_host_scan(
	struct wlan_regulatory_psoc_priv_obj *soc_reg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	reg_debug("host 11d enabled %d, inited: %d", soc_reg->enable_11d_supp,
		  soc_reg->is_host_11d_inited);
	if (!soc_reg->is_host_11d_inited)
		return QDF_STATUS_E_FAILURE;

	if (soc_reg->enable_11d_supp) {
		qdf_mc_timer_stop(&soc_reg->timer);
		status = reg_11d_scan_trigger_handler(soc_reg);
		if (status != QDF_STATUS_SUCCESS)
			return status;

		qdf_mc_timer_start(&soc_reg->timer, soc_reg->scan_11d_interval);
	} else {
		qdf_mc_timer_stop(&soc_reg->timer);
	}
	return status;
}

static void reg_11d_scan_timer(void *context)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg = context;

	reg_debug("11d scan timeout");

	if (!soc_reg)
		return;

	reg_11d_host_scan(soc_reg);
}

QDF_STATUS reg_11d_host_scan_init(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);
	if (!soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if (soc_reg->is_host_11d_inited) {
		reg_debug("host 11d scan are already inited");
		return QDF_STATUS_SUCCESS;
	}
	soc_reg->scan_req_id =
		ucfg_scan_register_requester(psoc, "11d",
					     reg_11d_scan_event_cb,
					     soc_reg);
	qdf_mc_timer_init(&soc_reg->timer, QDF_TIMER_TYPE_SW,
			  reg_11d_scan_timer, soc_reg);

	soc_reg->is_host_11d_inited = true;
	reg_debug("reg 11d scan inited");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_11d_host_scan_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);
	if (!soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if (!soc_reg->is_host_11d_inited) {
		reg_debug("host 11d scan are not inited");
		return QDF_STATUS_SUCCESS;
	}
	qdf_mc_timer_stop(&soc_reg->timer);
	qdf_mc_timer_destroy(&soc_reg->timer);
	ucfg_scan_unregister_requester(psoc, soc_reg->scan_req_id);
	soc_reg->is_host_11d_inited = false;
	reg_debug("reg 11d scan deinit");

	return QDF_STATUS_SUCCESS;
}

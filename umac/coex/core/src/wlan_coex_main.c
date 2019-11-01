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

/*
 * DOC: contains definitions for coex core functions
 */

#include <wlan_coex_ucfg_api.h>
#include <wlan_coex_tgt_api.h>
#include <wlan_coex_main.h>

QDF_STATUS wlan_coex_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
					       void *arg_list)
{
	struct coex_psoc_obj *psoc_obj;
	QDF_STATUS status;

	psoc_obj = qdf_mem_malloc(sizeof(*psoc_obj));
	if (!psoc_obj)
		return QDF_STATUS_E_NOMEM;

	/* Attach scan private date to psoc */
	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_COEX,
						       psoc_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		coex_err("Failed to attach psoc coex component");
		qdf_mem_free(psoc_obj);
	} else {
		coex_debug("Coex object attach to psoc successful");
	}

	return status;
}

QDF_STATUS wlan_coex_psoc_destroyed_notification(struct wlan_objmgr_psoc *psoc,
						 void *arg_list)
{
	void *psoc_obj;
	QDF_STATUS status;

	psoc_obj = wlan_psoc_get_coex_obj(psoc);
	if (!psoc_obj)
		return QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_COEX,
						       psoc_obj);
	if (QDF_IS_STATUS_ERROR(status))
		coex_err("Failed to detach psoc coex component");

	qdf_mem_free(psoc_obj);

	return status;
}

QDF_STATUS
wlan_coex_psoc_init(struct wlan_objmgr_psoc *psoc)
{
	struct coex_psoc_obj *coex_obj;

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return QDF_STATUS_E_INVAL;

	coex_obj->btc_chain_mode = WLAN_COEX_BTC_CHAIN_MODE_UNSETTLED;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_coex_psoc_deinit(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_coex_config_send(struct wlan_objmgr_vdev *vdev,
				 struct coex_config_params *param)
{
	QDF_STATUS status;

	status = tgt_send_coex_config(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		coex_err("failed to send coex config");

	return status;
}

QDF_STATUS
wlan_coex_config_updated(struct wlan_objmgr_vdev *vdev, uint8_t type)
{
	struct wlan_objmgr_psoc *psoc;
	struct coex_psoc_obj *coex_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!vdev) {
		coex_err("NULL vdev");
		return QDF_STATUS_E_INVAL;
	}

	if (type >= COEX_CONFIG_TYPE_MAX) {
		coex_err("config type out of range: %d", type);
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		coex_err("NULL psoc");
		return QDF_STATUS_E_INVAL;
	}

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return QDF_STATUS_E_INVAL;

	if (coex_obj->coex_config_updated[type])
		status = coex_obj->coex_config_updated[type](vdev);

	return status;
}

QDF_STATUS
wlan_coex_psoc_set_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	struct coex_psoc_obj *coex_obj;

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return QDF_STATUS_E_INVAL;

	coex_obj->btc_chain_mode = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_coex_psoc_get_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t *val)
{
	struct coex_psoc_obj *coex_obj;

	if (!val) {
		coex_err("invalid param for getting btc chain mode");
		return QDF_STATUS_E_INVAL;
	}

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return QDF_STATUS_E_INVAL;

	*val = coex_obj->btc_chain_mode;
	return QDF_STATUS_SUCCESS;
}

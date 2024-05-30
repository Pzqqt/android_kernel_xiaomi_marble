/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cp_stats_utils_api.c
 *
 * This file provide public API definitions for other accessing other UMAC
 * components
 */
#include "../../core/src/wlan_cp_stats_defs.h"
#include "../../core/src/wlan_cp_stats_obj_mgr_handler.h"
#include "../../core/src/wlan_cp_stats_comp_handler.h"
#include <wlan_cp_stats_utils_api.h>
#include <wlan_cp_stats_ucfg_api.h>

QDF_STATUS wlan_cp_stats_init(void)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_register_psoc_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_psoc_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register psoc create handler");
		goto wlan_cp_stats_psoc_init_fail1;
	}

	status = wlan_objmgr_register_psoc_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_psoc_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register psoc destroy handler");
		goto wlan_cp_stats_psoc_init_fail2;
	}

	status = wlan_objmgr_register_pdev_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_pdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register pdev create handler");
		goto wlan_cp_stats_pdev_init_fail1;
	}

	status = wlan_objmgr_register_pdev_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_pdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register pdev destroy handler");
		goto wlan_cp_stats_pdev_init_fail2;
	}

	status = wlan_objmgr_register_vdev_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_vdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register vdev create handler");
		goto wlan_cp_stats_vdev_init_fail1;
	}

	status = wlan_objmgr_register_vdev_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_vdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register vdev destroy handler");
		goto wlan_cp_stats_vdev_init_fail2;
	}

	status = wlan_objmgr_register_peer_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_peer_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register peer create handler");
		goto wlan_cp_stats_peer_init_fail1;
	}

	status = wlan_objmgr_register_peer_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_peer_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("Failed to register peer destroy handler");
		goto wlan_cp_stats_peer_init_fail2;
	}

	return QDF_STATUS_SUCCESS;

wlan_cp_stats_peer_init_fail2:
	wlan_objmgr_unregister_peer_create_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_peer_obj_create_handler,
		 NULL);
wlan_cp_stats_peer_init_fail1:
	wlan_objmgr_unregister_vdev_destroy_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_vdev_obj_destroy_handler,
		 NULL);
wlan_cp_stats_vdev_init_fail2:
	wlan_objmgr_unregister_vdev_create_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_vdev_obj_create_handler,
		 NULL);
wlan_cp_stats_vdev_init_fail1:
	wlan_objmgr_unregister_pdev_destroy_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_pdev_obj_destroy_handler,
		 NULL);
wlan_cp_stats_pdev_init_fail2:
	wlan_objmgr_unregister_pdev_create_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_pdev_obj_create_handler,
		 NULL);
wlan_cp_stats_pdev_init_fail1:
	wlan_objmgr_unregister_psoc_destroy_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_psoc_obj_destroy_handler,
		 NULL);
wlan_cp_stats_psoc_init_fail2:
	wlan_objmgr_unregister_psoc_create_handler
		(WLAN_UMAC_COMP_CP_STATS,
		 wlan_cp_stats_psoc_obj_create_handler,
		 NULL);
wlan_cp_stats_psoc_init_fail1:
	return status;
}

QDF_STATUS wlan_cp_stats_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_unregister_psoc_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_psoc_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister psoc create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_psoc_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister psoc destroy handler");

	status = wlan_objmgr_unregister_pdev_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_pdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister pdev create handler");

	status = wlan_objmgr_unregister_pdev_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_pdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister pdev destroy handler");

	status = wlan_objmgr_unregister_vdev_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_vdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_vdev_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_vdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister vdev destroy handler");

	status = wlan_objmgr_unregister_peer_create_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_peer_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister peer create handler");

	status = wlan_objmgr_unregister_peer_destroy_handler
				(WLAN_UMAC_COMP_CP_STATS,
				 wlan_cp_stats_peer_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cp_stats_err("Failed to unregister peer destroy handler");

	return status;
}

/* DA/OL specific call back initialization */
QDF_STATUS wlan_cp_stats_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}
	csc =
	wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_CP_STATS);
	if (!csc) {
		cp_stats_err("cp_stats_context is null!");
		return QDF_STATUS_E_FAILURE;
	}

	if (csc->cp_stats_open)
		status = csc->cp_stats_open(psoc);

	qdf_spinlock_create(&csc->csc_lock);
	return status;
}

QDF_STATUS wlan_cp_stats_close(struct wlan_objmgr_psoc *psoc)
{
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}
	csc =
	wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_CP_STATS);
	if (csc && csc->cp_stats_close) {
		csc->cp_stats_close(psoc);
		qdf_spinlock_destroy(&csc->csc_lock);
	}

	return QDF_STATUS_SUCCESS;
}

/* WMI registrations stage */
QDF_STATUS wlan_cp_stats_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is null!");
		return QDF_STATUS_E_INVAL;
	}
	csc =
	wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_CP_STATS);
	if (!csc) {
		cp_stats_err("cp_stats_context is null!");
		return QDF_STATUS_E_FAILURE;
	}

	if (csc->cp_stats_enable)
		status = csc->cp_stats_enable(psoc);

	return status;
}

QDF_STATUS wlan_cp_stats_disable(struct wlan_objmgr_psoc *psoc)
{
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is null!\n");
		return QDF_STATUS_E_INVAL;
	}
	csc =
	wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_CP_STATS);
	if (csc && csc->cp_stats_disable)
		csc->cp_stats_disable(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_comp_obj_cfg(enum wlan_objmgr_obj_type obj_type,
			   enum wlan_cp_stats_cfg_state cfg_state,
			   enum wlan_umac_comp_id comp_id,
			   void *cmn_obj, void *data)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct cp_stats_context *csc;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	enum wlan_cp_stats_comp_id cp_stats_comp_id;

	if (!cmn_obj) {
		cp_stats_err("common object is null!");
		return QDF_STATUS_E_INVAL;
	}

	cp_stats_comp_id = wlan_cp_stats_get_comp_id(comp_id);
	if (cp_stats_comp_id >= WLAN_CP_STATS_MAX_COMPONENTS) {
		cp_stats_err("Invalid UMAC id provided to cp_stats");
		return QDF_STATUS_E_INVAL;
	}

	switch (obj_type) {
	case WLAN_PSOC_OP:
		psoc = (struct wlan_objmgr_psoc *)cmn_obj;
		csc =
		wlan_objmgr_psoc_get_comp_private_obj
				(psoc, WLAN_UMAC_COMP_CP_STATS);
		break;
	case WLAN_PDEV_OP:
		pdev = (struct wlan_objmgr_pdev *)cmn_obj;
		csc = wlan_cp_stats_ctx_get_from_pdev(pdev);
		break;
	case WLAN_VDEV_OP:
		vdev = (struct wlan_objmgr_vdev *)cmn_obj;
		csc = wlan_cp_stats_ctx_get_from_vdev(vdev);
		break;
	case WLAN_PEER_OP:
		peer = (struct wlan_objmgr_peer *)cmn_obj;
		csc = wlan_cp_stats_ctx_get_from_peer(peer);
		break;
	default:
		cp_stats_err("Invalid common object type");
		return QDF_STATUS_E_INVAL;
	}

	if (!csc) {
		cp_stats_err("cp_stats_context is null!");
		return QDF_STATUS_E_FAILURE;
	}

	if (csc->cp_stats_comp_obj_config)
		status = csc->cp_stats_comp_obj_config(obj_type, cfg_state,
							cp_stats_comp_id,
							cmn_obj, data);

	return status;
}

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
QDF_STATUS
tgt_cp_stats_twt_get_session_evt_handler(
				struct wlan_objmgr_psoc *psoc,
				struct twt_session_stats_info *twt_params)
{
	return wlan_cp_stats_twt_get_session_evt_handler(psoc, twt_params);
}
#endif

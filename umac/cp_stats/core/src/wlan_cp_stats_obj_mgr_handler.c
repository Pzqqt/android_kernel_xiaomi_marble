/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
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
 * Doc: wlan_cp_stats_om_handler.c
 *
 * This file provide definitions to APIs invoked on receiving common object
 * repective create/destroy event notifications, which further
 * (de)allocate cp specific objects and (de)attach to specific
 * common object
 */
#include "wlan_cp_stats_obj_mgr_handler.h"
#include "wlan_cp_stats_defs.h"
#include "wlan_cp_stats_ol_api.h"
#include <wlan_cp_stats_ucfg_api.h>
#include "wlan_cp_stats_utils_api.h"
#include <target_if_cp_stats.h>

QDF_STATUS
wlan_cp_stats_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	WLAN_DEV_TYPE dev_type;
	struct cp_stats_context *csc = NULL;
	struct psoc_cp_stats *psoc_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!psoc) {
		cp_stats_err("PSOC is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	csc = qdf_mem_malloc(sizeof(*csc));
	if (!csc) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	csc->psoc_obj = psoc;
	dev_type = wlan_objmgr_psoc_get_dev_type(csc->psoc_obj);
	if (dev_type == WLAN_DEV_INVALID) {
		cp_stats_err("Failed to init cp stats ctx, bad device type");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	} else if (WLAN_DEV_OL == dev_type) {
		csc->cp_stats_ctx_init = wlan_cp_stats_ctx_init_ol;
		csc->cp_stats_ctx_deinit = wlan_cp_stats_ctx_deinit_ol;
	}

	if (QDF_STATUS_SUCCESS != csc->cp_stats_ctx_init(csc)) {
		cp_stats_err("Failed to init global ctx call back handlers");
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	psoc_cs = qdf_mem_malloc(sizeof(*psoc_cs));
	if (!psoc_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_psoc_obj_create_handler_return;
	}

	psoc_cs->psoc_obj = psoc;
	csc->psoc_cs = psoc_cs;
	if (csc->cp_stats_psoc_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_psoc_obj_init(psoc_cs)) {
			cp_stats_err("Failed to initialize psoc handlers");
			goto wlan_cp_stats_psoc_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_CP_STATS,
						       csc,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_psoc_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_psoc_obj_deinit && psoc_cs)
				csc->cp_stats_psoc_obj_deinit(psoc_cs);

			if (csc->psoc_cs) {
				qdf_mem_free(csc->psoc_cs);
				csc->psoc_cs = NULL;
			}

			if (csc->cp_stats_ctx_deinit)
				csc->cp_stats_ctx_deinit(csc);

			qdf_mem_free(csc);
			csc = NULL;
		}
		return status;
	}

	cp_stats_debug("cp stats context attach at psoc");
	return status;
}

QDF_STATUS
wlan_cp_stats_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct cp_stats_context *csc;

	if (!psoc) {
		cp_stats_err("PSOC is NULL");
		return QDF_STATUS_E_NOMEM;
	}
	csc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_CP_STATS);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_psoc_component_obj_detach(psoc,
					      WLAN_UMAC_COMP_CP_STATS, csc);
	if (csc->cp_stats_psoc_obj_deinit)
		csc->cp_stats_psoc_obj_deinit(csc->psoc_cs);
	qdf_mem_free(csc->psoc_cs);
	if (csc->cp_stats_ctx_deinit)
		csc->cp_stats_ctx_deinit(csc);
	qdf_mem_free(csc);

	cp_stats_debug("cp stats context dettached at psoc");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct pdev_cp_stats *pdev_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!pdev) {
		cp_stats_err("PDEV is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}

	pdev_cs = qdf_mem_malloc(sizeof(*pdev_cs));
	if (!pdev_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_pdev(pdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_pdev_obj_create_handler_return;
	}
	pdev_cs->pdev_obj = pdev;
	if (csc->cp_stats_pdev_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_pdev_obj_init(pdev_cs)) {
			cp_stats_err("Failed to initialize pdev handlers");
			goto wlan_cp_stats_pdev_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
						       WLAN_UMAC_COMP_CP_STATS,
						       pdev_cs,
						       QDF_STATUS_SUCCESS);

	cp_stats_debug("pdev cp stats object attached");
wlan_cp_stats_pdev_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_pdev_obj_deinit)
				csc->cp_stats_pdev_obj_deinit(pdev_cs);
		}

		if (pdev_cs)
			qdf_mem_free(pdev_cs);
	}

	return status;
}

QDF_STATUS
wlan_cp_stats_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cp_stats *pdev_cs;
	struct cp_stats_context *csc;

	if (!pdev) {
		cp_stats_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev_cs = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						WLAN_UMAC_COMP_CP_STATS);
	if (!pdev_cs) {
		cp_stats_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_pdev(pdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_pdev_obj_deinit)
		csc->cp_stats_pdev_obj_deinit(pdev_cs);

	wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_CP_STATS,
					      pdev_cs);

	qdf_mem_free(pdev_cs);
	cp_stats_debug("pdev cp stats object dettached");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct vdev_cp_stats *vdev_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!vdev) {
		cp_stats_err("vdev is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}

	vdev_cs = qdf_mem_malloc(sizeof(*vdev_cs));
	if (!vdev_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_vdev(vdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_vdev_obj_create_handler_return;
	}
	vdev_cs->vdev_obj = vdev;
	if (csc->cp_stats_vdev_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_vdev_obj_init(vdev_cs)) {
			cp_stats_err("Failed to initialize vdev handlers");
			goto wlan_cp_stats_vdev_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_CP_STATS,
						       vdev_cs,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_vdev_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_vdev_obj_deinit)
				csc->cp_stats_vdev_obj_deinit(vdev_cs);
		}

		if (vdev_cs)
			qdf_mem_free(vdev_cs);
	}

	cp_stats_debug("vdev cp stats object attach");
	return status;
}

QDF_STATUS
wlan_cp_stats_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct vdev_cp_stats *vdev_cs;
	struct cp_stats_context *csc;

	if (!vdev) {
		cp_stats_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	vdev_cs = wlan_objmgr_vdev_get_comp_private_obj(vdev,
						WLAN_UMAC_COMP_CP_STATS);
	if (!vdev_cs) {
		cp_stats_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_vdev(vdev);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_vdev_obj_deinit)
		csc->cp_stats_vdev_obj_deinit(vdev_cs);

	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_CP_STATS,
					      vdev_cs);

	qdf_mem_free(vdev_cs);
	cp_stats_debug("vdev cp stats object dettach");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct cp_stats_context *csc = NULL;
	struct peer_cp_stats *peer_cs = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!peer) {
		cp_stats_err("peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}

	peer_cs = qdf_mem_malloc(sizeof(*peer_cs));
	if (!peer_cs) {
		status = QDF_STATUS_E_NOMEM;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}
	csc = wlan_cp_stats_ctx_get_from_peer(peer);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		status = QDF_STATUS_E_INVAL;
		goto wlan_cp_stats_peer_obj_create_handler_return;
	}
	peer_cs->peer_obj = peer;
	if (csc->cp_stats_peer_obj_init) {
		if (QDF_STATUS_SUCCESS !=
				csc->cp_stats_peer_obj_init(peer_cs)) {
			cp_stats_err("Failed to initialize peer handlers");
			goto wlan_cp_stats_peer_obj_create_handler_return;
		}
	}

	status = wlan_objmgr_peer_component_obj_attach(peer,
						       WLAN_UMAC_COMP_CP_STATS,
						       peer_cs,
						       QDF_STATUS_SUCCESS);

wlan_cp_stats_peer_obj_create_handler_return:
	if (QDF_IS_STATUS_ERROR(status)) {
		if (csc) {
			if (csc->cp_stats_peer_obj_deinit)
				csc->cp_stats_peer_obj_deinit(peer_cs);
		}

		if (peer_cs)
			qdf_mem_free(peer_cs);
	}

	cp_stats_debug("peer cp stats object attach");
	return status;
}

QDF_STATUS
wlan_cp_stats_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cp_stats *peer_cs;
	struct cp_stats_context *csc;

	if (!peer) {
		cp_stats_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	peer_cs = wlan_objmgr_peer_get_comp_private_obj(peer,
						WLAN_UMAC_COMP_CP_STATS);
	if (!peer_cs) {
		cp_stats_err("peer is NULL");
		return QDF_STATUS_E_INVAL;
	}
	csc = wlan_cp_stats_ctx_get_from_peer(peer);
	if (!csc) {
		cp_stats_err("cp_stats context is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (csc->cp_stats_peer_obj_deinit)
		csc->cp_stats_peer_obj_deinit(peer_cs);

	wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_CP_STATS,
					      peer_cs);

	qdf_mem_free(peer_cs);
	cp_stats_debug("peer cp stats object dettached");
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
QDF_STATUS
wlan_cp_stats_infra_cp_register_resp_cb(struct wlan_objmgr_psoc *psoc,
					struct infra_cp_stats_cmd_info *req)
{
	struct psoc_cp_stats *psoc_cp_stats_priv;

	psoc_cp_stats_priv = wlan_cp_stats_get_psoc_stats_obj(psoc);
	if (!psoc_cp_stats_priv) {
		cp_stats_err("psoc cp stats object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wlan_cp_stats_psoc_obj_lock(psoc_cp_stats_priv);
	psoc_cp_stats_priv->get_infra_cp_stats = req->infra_cp_stats_resp_cb;
	psoc_cp_stats_priv->infra_cp_stats_req_context = req->request_cookie;
	wlan_cp_stats_psoc_obj_unlock(psoc_cp_stats_priv);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_infra_cp_get_context(struct wlan_objmgr_psoc *psoc,
				   get_infra_cp_stats_cb *resp_cb,
				   void **context)
{
	struct psoc_cp_stats *psoc_cp_stats_priv;

	psoc_cp_stats_priv = wlan_cp_stats_get_psoc_stats_obj(psoc);
	if (!psoc_cp_stats_priv) {
		cp_stats_err("psoc cp stats object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wlan_cp_stats_psoc_obj_lock(psoc_cp_stats_priv);
	*resp_cb = psoc_cp_stats_priv->get_infra_cp_stats;
	*context = psoc_cp_stats_priv->infra_cp_stats_req_context;
	wlan_cp_stats_psoc_obj_unlock(psoc_cp_stats_priv);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_send_infra_cp_req(struct wlan_objmgr_psoc *psoc,
				struct infra_cp_stats_cmd_info *req)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops) {
		cp_stats_err("could not get tx_ops");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->send_req_infra_cp_stats) {
		cp_stats_err("could not get send_req_infra_twt_stats");
		return QDF_STATUS_E_NULL_VALUE;
	}
	return tx_ops->send_req_infra_cp_stats(psoc, req);
}

#if defined(WLAN_SUPPORT_TWT) && defined (WLAN_TWT_CONV_SUPPORTED)
/**
 * wlan_cp_stats_twt_get_peer_session_param() - Obtains twt session parameters
 * of a peer if twt session is valid
 * @mc_cp_stats: pointer to peer specific stats
 * @param: Pointer to copy twt session parameters
 * @num_twt_sessions Pointer holding total number of valid twt sessions
 *
 * Return: QDF_STATUS success if valid twt session parameters are obtained
 * else other qdf error values
 */
static QDF_STATUS
wlan_cp_stats_twt_get_peer_session_param(struct peer_cp_stats *peer_cp_stat_prv,
					 struct twt_session_stats_info *params,
					 int *num_twt_session)
{
	struct twt_session_stats_info *twt_params;
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;
	uint32_t event_type;
	int i;

	if (!peer_cp_stat_prv || !params)
		return qdf_status;

	for (i = 0; i < TWT_PEER_MAX_SESSIONS; i++) {
		twt_params = &peer_cp_stat_prv->twt_param[i];
		event_type = peer_cp_stat_prv->twt_param[i].event_type;

		/* Check twt session is established */

		if (event_type == HOST_TWT_SESSION_SETUP ||
		    event_type == HOST_TWT_SESSION_UPDATE) {
			qdf_mem_copy(&params[*num_twt_session], twt_params,
				     sizeof(*twt_params));
			qdf_status = QDF_STATUS_SUCCESS;
			*num_twt_session += 1;
		}
	}

	return qdf_status;
}

/**
 * wlan_cp_stats_twt_get_all_peer_session_params()- Retrieves twt session
 * parameters of all peers with valid twt session
 * @psoc_obj: psoc object
 * @vdvev_id: vdev_id
 * @params: array of pointer to store peer twt session parameters
 *
 * Return: total number of valid twt sessions
 */
static int
wlan_cp_stats_twt_get_all_peer_session_params(
					struct wlan_objmgr_psoc *psoc_obj,
					uint8_t vdev_id,
					struct twt_session_stats_info *params)
{
	qdf_list_t *peer_list;
	struct wlan_objmgr_peer *peer, *peer_next;
	struct wlan_objmgr_vdev *vdev;
	struct peer_cp_stats *cp_stats_peer_obj, *peer_cp_stat_prv;
	int num_twt_session = 0;
	enum QDF_OPMODE opmode;
	uint16_t sap_num_peer;

	if (!psoc_obj) {
		cp_stats_err("psoc is NULL");
		return num_twt_session;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc_obj, vdev_id,
						    WLAN_CP_STATS_ID);
	if (!vdev) {
		cp_stats_err("vdev is NULL, vdev_id: %d", vdev_id);
		return num_twt_session;
	}

	sap_num_peer = wlan_vdev_get_peer_count(vdev);
	opmode = wlan_vdev_mlme_get_opmode(vdev);

	peer_list = &vdev->vdev_objmgr.wlan_peer_list;
	if (!peer_list) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_CP_STATS_ID);
		cp_stats_err("Peer list for vdev obj is NULL");
		return num_twt_session;
	}

	peer = wlan_vdev_peer_list_peek_active_head(vdev, peer_list,
						    WLAN_CP_STATS_ID);
	while (peer) {
		cp_stats_peer_obj = wlan_objmgr_peer_get_comp_private_obj(
						peer, WLAN_UMAC_COMP_CP_STATS);

		peer_cp_stat_prv = wlan_cp_stats_get_peer_stats_obj(peer);
		if (peer_cp_stat_prv) {
			wlan_cp_stats_peer_obj_lock(peer_cp_stat_prv);
			wlan_cp_stats_twt_get_peer_session_param(
							peer_cp_stat_prv,
							params,
							&num_twt_session);
			wlan_cp_stats_peer_obj_unlock(peer_cp_stat_prv);
		}

		if (opmode == QDF_STA_MODE &&
		    num_twt_session >= TWT_PEER_MAX_SESSIONS) {
			wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);
			goto done;
		}

		if (opmode == QDF_SAP_MODE &&
		    num_twt_session >= (sap_num_peer * TWT_PEER_MAX_SESSIONS)) {
			wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);
			goto done;
		}

		peer_next = wlan_peer_get_next_active_peer_of_vdev(
						vdev, peer_list, peer,
						WLAN_CP_STATS_ID);
		wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);
		peer = peer_next;
	}
done:
	if (!num_twt_session)
		cp_stats_err("Unable to find a peer with twt session established");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_CP_STATS_ID);
	return num_twt_session;
}

/**
 * wlan_cp_stats_twt_get_peer_session_param_by_dlg_id() - Finds a Peer twt
 * session with dialog id matching with input dialog id. If a match is found
 * copies the twt session parameters
 * @peer_cp_stats_priv: pointer to peer specific stats
 * @input_dialog_id: input dialog id
 * @dest_param: Pointer to copy twt session parameters when a peer with
 * given dialog id is found
 * @num_twt_session: Pointer holding total number of valid twt session
 *
 * Return: Success if stats are copied for a peer with given dialog,
 * else failure
 */
static QDF_STATUS
wlan_cp_stats_twt_get_peer_session_param_by_dlg_id(
				struct peer_cp_stats *peer_cp_stats_priv,
				uint32_t input_dialog_id,
				struct twt_session_stats_info *dest_param,
				int *num_twt_session)
{
	struct twt_session_stats_info *src_param;
	uint32_t event_type;
	int i = 0;
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;

	if (!peer_cp_stats_priv || !dest_param)
		return qdf_status;

	for (i = 0; i < TWT_PEER_MAX_SESSIONS; i++) {
		event_type = peer_cp_stats_priv->twt_param[i].event_type;
		src_param = &peer_cp_stats_priv->twt_param[i];
		if (!event_type ||
		    (src_param->dialog_id != input_dialog_id &&
		    input_dialog_id != TWT_ALL_SESSIONS_DIALOG_ID))
			continue;

		if (event_type == HOST_TWT_SESSION_SETUP ||
		    event_type == HOST_TWT_SESSION_UPDATE) {
			qdf_mem_copy(&dest_param[*num_twt_session], src_param,
				     sizeof(*src_param));
			qdf_status = QDF_STATUS_SUCCESS;
			*num_twt_session += 1;
			if (*num_twt_session >= TWT_PEER_MAX_SESSIONS)
				break;
		}
	}

	return qdf_status;
}

/**
 * wlan_cp_stats_twt_get_single_peer_session_params()- Extracts twt session
 * parameters corresponding to a peer given by dialog_id
 * @psoc_obj: psoc object
 * @mac_addr: mac addr of peer
 * @dialog_id: dialog id of peer for which twt session params to be retrieved
 * @params: pointer to store peer twt session parameters
 *
 * Return: total number of valid twt session
 */
static int
wlan_cp_stats_twt_get_single_peer_session_params(
					struct wlan_objmgr_psoc *psoc_obj,
					uint8_t *mac_addr, uint32_t dialog_id,
					struct twt_session_stats_info *params)
{
	struct wlan_objmgr_peer *peer;
	struct peer_cp_stats *peer_cp_stats_priv;
	QDF_STATUS qdf_status = QDF_STATUS_E_INVAL;
	int num_twt_session = 0;

	if (!psoc_obj || !params)
		return num_twt_session;

	peer = wlan_objmgr_get_peer_by_mac(psoc_obj, mac_addr,
					   WLAN_CP_STATS_ID);
	if (!peer)
		return num_twt_session;
	peer_cp_stats_priv = wlan_cp_stats_get_peer_stats_obj(peer);

	if (!peer_cp_stats_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);
		return num_twt_session;
	}

	wlan_cp_stats_peer_obj_lock(peer_cp_stats_priv);

	qdf_status = wlan_cp_stats_twt_get_peer_session_param_by_dlg_id(
							peer_cp_stats_priv,
							dialog_id,
							params,
							&num_twt_session);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		qdf_err("No TWT session for " QDF_MAC_ADDR_FMT " dialog_id %d",
			QDF_MAC_ADDR_REF(mac_addr), dialog_id);
	}

	wlan_cp_stats_peer_obj_unlock(peer_cp_stats_priv);
	wlan_objmgr_peer_release_ref(peer, WLAN_CP_STATS_ID);

	return num_twt_session;
}

int
wlan_cp_stats_twt_get_peer_session_params(struct wlan_objmgr_psoc *psoc,
					  struct twt_session_stats_info *params)
{
	uint8_t *mac_addr;
	uint32_t dialog_id;
	uint8_t vdev_id;
	int num_twt_session = 0;

	if (!psoc || !params)
		return num_twt_session;

	mac_addr = params[0].peer_mac.bytes;
	dialog_id = params[0].dialog_id;
	vdev_id = params[0].vdev_id;

	/*
	 * Currently for STA case, twt_get_params nl is sending only dialog_id
	 * and mac_addr is being filled by driver in STA peer case.
	 * For SAP case, twt_get_params nl is sending dialog_id and
	 * peer mac_addr. When twt_get_params add mac_addr and dialog_id of
	 * STA/SAP, we need handle unicast/multicast macaddr in
	 * wlan_cp_stats_twt_get_peer_session_params.
	 */
	if (!QDF_IS_ADDR_BROADCAST(mac_addr))
		num_twt_session =
			wlan_cp_stats_twt_get_single_peer_session_params(
								psoc, mac_addr,
								dialog_id,
								params);
	else
		num_twt_session = wlan_cp_stats_twt_get_all_peer_session_params(
								psoc, vdev_id,
								params);
	return num_twt_session;
}
#endif
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */


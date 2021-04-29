/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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

#include <sir_common.h>
#include <ani_global.h>
#include <csr_inside_api.h>
#include <csr_neighbor_roam.h>
#include <sir_api.h>

#ifdef WLAN_FEATURE_HOST_ROAM

#ifndef FEATURE_CM_ENABLE
/* Initialize the FT context. */
void sme_ft_open(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	if (pSession) {
		/* Clean up the context */
		qdf_mem_zero(&pSession->ftSmeContext, sizeof(tftSMEContext));

		pSession->ftSmeContext.pUsrCtx =
			qdf_mem_malloc(sizeof(tFTRoamCallbackUsrCtx));
		if (!pSession->ftSmeContext.pUsrCtx)
			return;

		pSession->ftSmeContext.pUsrCtx->mac = mac;
		pSession->ftSmeContext.pUsrCtx->sessionId = sessionId;

		status =
			qdf_mc_timer_init(&pSession->ftSmeContext.preAuthReassocIntvlTimer,
					  QDF_TIMER_TYPE_SW,
				sme_preauth_reassoc_intvl_timer_callback,
					(void *)pSession->ftSmeContext.pUsrCtx);

		if (QDF_STATUS_SUCCESS != status) {
			sme_err("Preauth Reassoc interval Timer allocation failed");
			qdf_mem_free(pSession->ftSmeContext.pUsrCtx);
			pSession->ftSmeContext.pUsrCtx = NULL;
			return;
		}
	}
}

/* Cleanup the SME FT Global context. */
void sme_ft_close(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = NULL;

	pSession = CSR_GET_SESSION(mac, sessionId);
	if (pSession) {
		if (pSession->ftSmeContext.psavedFTPreAuthRsp) {
			qdf_mem_free(pSession->ftSmeContext.psavedFTPreAuthRsp);
			pSession->ftSmeContext.psavedFTPreAuthRsp = NULL;
		}
		qdf_mem_zero(pSession->ftSmeContext.preAuthbssId,
			     QDF_MAC_ADDR_SIZE);
		/* check if the timer is running */
		if (QDF_TIMER_STATE_RUNNING ==
		    qdf_mc_timer_get_current_state(&pSession->ftSmeContext.
						   preAuthReassocIntvlTimer)) {
			qdf_mc_timer_stop(&pSession->ftSmeContext.
					  preAuthReassocIntvlTimer);
		}

		qdf_mc_timer_destroy(&pSession->ftSmeContext.
					preAuthReassocIntvlTimer);
		if (pSession->ftSmeContext.pUsrCtx) {
			qdf_mem_free(pSession->ftSmeContext.pUsrCtx);
			pSession->ftSmeContext.pUsrCtx = NULL;
		}
	}
}

/*
 * HDD Interface to SME. SME now sends the Auth 2 and RIC IEs up to the
 * supplicant. The supplicant will then proceed to send down the
 * Reassoc Req.
 */
void sme_get_ft_pre_auth_response(mac_handle_t mac_handle, uint8_t sessionId,
				  uint8_t *ft_ies, uint32_t ft_ies_ip_len,
				  uint16_t *ft_ies_length)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	*ft_ies_length = 0;

	if (!pSession) {
		sme_err("pSession is NULL");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_ERROR(status))
		goto end;

	/* All or nothing - proceed only if both BSSID and FT IE fit */
	if ((QDF_MAC_ADDR_SIZE +
	     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length) >
	    ft_ies_ip_len) {
		sme_release_global_lock(&mac->sme);
		goto end;
	}
	/* hdd needs to pack the bssid also along with the */
	/* auth response to supplicant */
	qdf_mem_copy(ft_ies, pSession->ftSmeContext.preAuthbssId,
		     QDF_MAC_ADDR_SIZE);

	/* Copy the auth resp FTIEs */
	qdf_mem_copy(&(ft_ies[QDF_MAC_ADDR_SIZE]),
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies,
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length);

	*ft_ies_length = QDF_MAC_ADDR_SIZE +
		pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length;

	sme_debug("Filled auth resp: %d", *ft_ies_length);
	sme_release_global_lock(&mac->sme);
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

}

/*
 * SME now sends the RIC IEs up to the supplicant.
 * The supplicant will then proceed to send down the
 * Reassoc Req.
 */
void sme_get_rici_es(mac_handle_t mac_handle, uint8_t sessionId,
		     uint8_t *ric_ies,
		     uint32_t ric_ies_ip_len, uint32_t *ric_ies_length)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	*ric_ies_length = 0;
	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return;
	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		goto end;
	}

	/* All or nothing */
	if (mlme_priv->connect_info.ft_info.ric_ies_length > ric_ies_ip_len)
		goto end;

	qdf_mem_copy(ric_ies,
		     mlme_priv->connect_info.ft_info.ric_ies,
		     mlme_priv->connect_info.ft_info.ric_ies_length);

	*ric_ies_length = mlme_priv->connect_info.ft_info.ric_ies_length;

	sme_debug("Filled ric ies: %d", *ric_ies_length);
end:

	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
}

/*
 * Timer callback for the timer that is started between the preauth completion
 * and reassoc request to the PE. In this interval, it is expected that the
 * pre-auth response and RIC IEs are passed up to the WPA supplicant and
 * received back the necessary FTIEs required to be sent in the reassoc request
 */
void sme_preauth_reassoc_intvl_timer_callback(void *context)
{
	tFTRoamCallbackUsrCtx *pUsrCtx = (tFTRoamCallbackUsrCtx *) context;

	if (pUsrCtx)
		csr_neighbor_roam_request_handoff(pUsrCtx->mac,
						  pUsrCtx->sessionId);
}

#endif

void sme_set_ft_pre_auth_state(mac_handle_t mac_handle, uint8_t sessionId,
			       bool state)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	mlme_priv->connect_info.ft_info.set_ft_preauth_state = state;
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
}

bool sme_get_ft_pre_auth_state(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;
	bool val = false;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return val;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	val = mlme_priv->connect_info.ft_info.set_ft_preauth_state;
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return val;
}

void sme_set_ft_ies(mac_handle_t mac_handle, uint8_t session_id,
		    const uint8_t *ft_ies, uint16_t ft_ies_length)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	if (!ft_ies) {
		sme_err("ft ies is NULL");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac_ctx->pdev, session_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_ERROR(status))
		goto end;

	sme_debug("FT IEs Req is received in state %d",
		  mlme_priv->connect_info.ft_info.ft_state);

	/* Global Station FT State */
	switch (mlme_priv->connect_info.ft_info.ft_state) {
	case FT_START_READY:
	case FT_AUTH_REQ_READY:
		sme_debug("ft_ies_length: %d", ft_ies_length);
		ft_ies_length = QDF_MIN(ft_ies_length, MAX_FTIE_SIZE);
		mlme_priv->connect_info.ft_info.auth_ie_len = ft_ies_length;
		qdf_mem_copy(mlme_priv->connect_info.ft_info.auth_ft_ie,
			     ft_ies, ft_ies_length);
		mlme_priv->connect_info.ft_info.ft_state = FT_AUTH_REQ_READY;
		break;

	case FT_REASSOC_REQ_WAIT:
		/*
		 * We are done with pre-auth, hence now waiting for
		 * reassoc req. This is the new FT Roaming in place At
		 * this juncture we'r ready to start sending Reassoc req
		 */

		ft_ies_length = QDF_MIN(ft_ies_length, MAX_FTIE_SIZE);

		sme_debug("New Reassoc Req: %pK in state %d",
			ft_ies, mlme_priv->connect_info.ft_info.ft_state);
		mlme_priv->connect_info.ft_info.reassoc_ie_len =
							ft_ies_length;
		qdf_mem_copy(mlme_priv->connect_info.ft_info.reassoc_ft_ie,
				ft_ies, ft_ies_length);

		mlme_priv->connect_info.ft_info.ft_state = FT_SET_KEY_WAIT;
		sme_debug("ft_ies_length: %d state: %d", ft_ies_length,
			  mlme_priv->connect_info.ft_info.ft_state);
		break;

	default:
		sme_warn("Unhandled state: %d",
			 mlme_priv->connect_info.ft_info.ft_state);
		break;
	}
	sme_release_global_lock(&mac_ctx->sme);
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
}

QDF_STATUS sme_check_ft_status(mac_handle_t mac_handle, uint8_t session_id)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, session_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return status;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_ERROR(status))
		goto end;

	sme_debug("FT update key is received in state %d",
		  mlme_priv->connect_info.ft_info.ft_state);

	/* Global Station FT State */
	switch (mlme_priv->connect_info.ft_info.ft_state) {
	case FT_SET_KEY_WAIT:
		if (sme_get_ft_pre_auth_state(mac_handle, session_id)) {
			mlme_priv->connect_info.ft_info.ft_state = FT_START_READY;
			sme_debug("state changed to %d",
				  mlme_priv->connect_info.ft_info.ft_state);
			break;
		}
		/* fallthrough */
	default:
		sme_debug("Unhandled state:%d",
			  mlme_priv->connect_info.ft_info.ft_state);
		status = QDF_STATUS_E_FAILURE;
		break;
	}
	sme_release_global_lock(&mac->sme);
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return status;
}

bool sme_ft_key_ready_for_install(mac_handle_t mac_handle, uint8_t session_id)
{
	bool ret = false;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, session_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return ret;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	if (sme_get_ft_pre_auth_state(mac_handle, session_id) &&
	    mlme_priv->connect_info.ft_info.ft_state == FT_START_READY) {
		ret = true;
		sme_set_ft_pre_auth_state(mac_handle, session_id, false);
	}
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

	return ret;
}

/* Reset the FT context. */
void sme_ft_reset(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	pSession = CSR_GET_SESSION(mac, sessionId);
	if (!pSession)
		return;
	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, sessionId,
						    WLAN_LEGACY_SME_ID);
	if (!vdev)
		return;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv)
		goto end;

	qdf_mem_zero(&mlme_priv->connect_info.ft_info,
		     sizeof(struct ft_context));
#ifndef FEATURE_CM_ENABLE
	if (pSession->ftSmeContext.psavedFTPreAuthRsp) {
		qdf_mem_free(pSession->ftSmeContext.psavedFTPreAuthRsp);
		pSession->ftSmeContext.psavedFTPreAuthRsp = NULL;
	}
	qdf_mem_zero(pSession->ftSmeContext.preAuthbssId,
		     QDF_MAC_ADDR_SIZE);
#endif
	mlme_priv->connect_info.ft_info.ft_state = FT_START_READY;
end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
}

#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#ifdef FEATURE_WLAN_ESE
static void sme_reset_esecckm_info(struct mac_context *mac, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct rso_config *rso_cfg;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac->pdev, vdev_id,
						    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("vdev object is NULL for vdev %d", vdev_id);
		return;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
		return;
	}

	qdf_mem_zero(rso_cfg->krk, WMI_KRK_KEY_LEN);
	qdf_mem_zero(rso_cfg->btk, WMI_BTK_KEY_LEN);
	rso_cfg->is_ese_assoc = false;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

}
#else
static void sme_reset_esecckm_info(struct mac_context *mac, uint8_t vdev_id)
{
}
#endif

void sme_reset_key(mac_handle_t mac_handle, uint8_t vdev_id)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		sme_err("mac is NULL");
		return;
	}

	wlan_cm_set_psk_pmk(mac->pdev, vdev_id, NULL, 0);
	sme_reset_esecckm_info(mac, vdev_id);
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

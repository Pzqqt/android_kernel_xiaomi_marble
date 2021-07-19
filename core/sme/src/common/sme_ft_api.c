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
#include "wlan_cm_roam_api.h"
#include <sir_api.h>

#ifdef WLAN_FEATURE_HOST_ROAM

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

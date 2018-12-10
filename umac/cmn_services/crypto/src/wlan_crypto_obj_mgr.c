/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Public API intialization of crypto service with object manager
 */
#include <qdf_types.h>
#include <wlan_cmn.h>
#include <wlan_objmgr_cmn.h>

#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#include "wlan_crypto_global_def.h"
#include "wlan_crypto_global_api.h"
#include "wlan_crypto_def_i.h"
#include "wlan_crypto_main_i.h"
#include "wlan_crypto_obj_mgr_i.h"
#ifdef WLAN_CRYPTO_SUPPORT_FILS
#include "wlan_crypto_fils_api.h"
#endif


extern const struct wlan_crypto_cipher
				*wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_MAX];

static QDF_STATUS wlan_crypto_register_all_ciphers(
					struct wlan_crypto_params *crypto_param)
{

	if (HAS_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_WEP)) {
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_WEP]
							= wep_register();
	}
	if (HAS_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_TKIP_MIC)) {
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_TKIP]
							= tkip_register();
	}
	if (HAS_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_AES)) {
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_AES_CCM]
							= ccmp_register();
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_AES_CCM_256]
							= ccmp256_register();
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_AES_GCM]
							= gcmp_register();
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_AES_GCM_256]
							= gcmp256_register();
	}
	if (HAS_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_WAPI_SMS4)) {
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_WAPI_SMS4]
							= wapi_register();
	}
	if (HAS_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_FILS_AEAD)) {
		wlan_crypto_cipher_ops[WLAN_CRYPTO_CIPHER_FILS_AEAD]
							= fils_register();
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wlan_crypto_vdev_obj_create_handler(
						struct wlan_objmgr_vdev *vdev,
						void *arg)
{
	struct wlan_crypto_comp_priv *crypto_priv;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_crypto_params *crypto_param;
	QDF_STATUS status;

	if (!vdev)
		return QDF_STATUS_E_INVAL;

	crypto_priv = qdf_mem_malloc(sizeof(struct wlan_crypto_comp_priv));
	if (!crypto_priv)
		return QDF_STATUS_E_NOMEM;

	crypto_param = &(crypto_priv->crypto_params);

	RESET_AUTHMODE(crypto_param);
	RESET_UCAST_CIPHERS(crypto_param);
	RESET_MCAST_CIPHERS(crypto_param);
	RESET_MGMT_CIPHERS(crypto_param);
	RESET_KEY_MGMT(crypto_param);
	RESET_CIPHER_CAP(crypto_param);

	pdev = wlan_vdev_get_pdev(vdev);
	wlan_pdev_obj_lock(pdev);
	if (wlan_pdev_nif_fw_cap_get(pdev, WLAN_SOC_C_WEP))
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_WEP);
	if (wlan_pdev_nif_fw_cap_get(pdev, WLAN_SOC_C_TKIP))
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_TKIP_MIC);
	if (wlan_pdev_nif_fw_cap_get(pdev, WLAN_SOC_C_AES)) {
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_AES);
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_CCM256);
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_GCM);
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_GCM_256);
	}
	if (wlan_pdev_nif_fw_cap_get(pdev, WLAN_SOC_C_CKIP))
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_CKIP);
	if (wlan_pdev_nif_fw_cap_get(pdev, WLAN_SOC_C_WAPI))
		SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_WAPI_SMS4);
	SET_CIPHER_CAP(crypto_param, WLAN_CRYPTO_CAP_FILS_AEAD);
	wlan_pdev_obj_unlock(pdev);
	/* update the crypto cipher table based on the fw caps*/
	/* update the fw_caps into ciphercaps then attach to objmgr*/
	wlan_crypto_register_all_ciphers(crypto_param);

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
							WLAN_UMAC_COMP_CRYPTO,
							(void *)crypto_priv,
							QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS)
		qdf_mem_free(crypto_priv);

	return status;
}

static QDF_STATUS wlan_crypto_peer_obj_create_handler(
						struct wlan_objmgr_peer *peer,
						void *arg)
{
	struct wlan_crypto_comp_priv *crypto_priv;
	struct wlan_crypto_params *crypto_param;
	QDF_STATUS status;

	if (!peer)
		return QDF_STATUS_E_INVAL;

	crypto_priv = qdf_mem_malloc(sizeof(struct wlan_crypto_comp_priv));
	if (!crypto_priv)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_peer_component_obj_attach(peer,
				WLAN_UMAC_COMP_CRYPTO, (void *)crypto_priv,
				QDF_STATUS_SUCCESS);

	if (status == QDF_STATUS_SUCCESS) {
		crypto_param = &crypto_priv->crypto_params;
		RESET_AUTHMODE(crypto_param);
		RESET_UCAST_CIPHERS(crypto_param);
		RESET_MCAST_CIPHERS(crypto_param);
		RESET_MGMT_CIPHERS(crypto_param);
		RESET_KEY_MGMT(crypto_param);
		RESET_CIPHER_CAP(crypto_param);
		if (wlan_vdev_get_selfpeer(peer->peer_objmgr.vdev) != peer) {
			wlan_crypto_set_peer_wep_keys(
					wlan_peer_get_vdev(peer), peer);
		}
	} else {
		crypto_err("peer obj failed status %d", status);
		qdf_mem_free(crypto_priv);
	}

	return status;
}

static void wlan_crypto_free_key(struct wlan_crypto_comp_priv *crypto_priv)
{
	uint8_t i;

	if (!crypto_priv) {
		crypto_err("crypto_priv NULL");
		return;
	}

	for (i = 0; i < WLAN_CRYPTO_MAXKEYIDX; i++) {
		if (crypto_priv->key[i]) {
			qdf_mem_free(crypto_priv->key[i]);
			crypto_priv->key[i] = NULL;
		}
	}

	for (i = 0; i < WLAN_CRYPTO_MAXIGTKKEYIDX; i++) {
		if (crypto_priv->igtk_key[i]) {
			qdf_mem_free(crypto_priv->igtk_key[i]);
			crypto_priv->igtk_key[i] = NULL;
		}
	}

}

static QDF_STATUS wlan_crypto_vdev_obj_destroy_handler(
						struct wlan_objmgr_vdev *vdev,
						void *arg){
	struct wlan_crypto_comp_priv *crypto_priv;

	if (!vdev) {
		crypto_err("Vdev NULL");
		return QDF_STATUS_E_INVAL;
	}

	crypto_priv = (struct wlan_crypto_comp_priv *)
				wlan_get_vdev_crypto_obj(vdev);

	if (!crypto_priv) {
		crypto_err("crypto_priv NULL");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_component_obj_detach(vdev,
						WLAN_UMAC_COMP_CRYPTO,
						(void *)crypto_priv);

	wlan_crypto_pmksa_flush(&crypto_priv->crypto_params);
	wlan_crypto_free_key(crypto_priv);
	qdf_mem_free(crypto_priv);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wlan_crypto_peer_obj_destroy_handler(
						struct wlan_objmgr_peer *peer,
						void *arg){
	struct wlan_crypto_comp_priv *crypto_priv;

	if (!peer) {
		crypto_err("Peer NULL");
		return QDF_STATUS_E_INVAL;
	}
	crypto_priv = (struct wlan_crypto_comp_priv *)
				wlan_get_peer_crypto_obj(peer);
	if (!crypto_priv) {
		crypto_err("crypto_priv NULL");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_peer_component_obj_detach(peer,
						WLAN_UMAC_COMP_CRYPTO,
						(void *)crypto_priv);
	wlan_crypto_free_key(crypto_priv);
	qdf_mem_free(crypto_priv);

	return QDF_STATUS_SUCCESS;
}
/**
 * __wlan_crypto_init - Init the crypto service with object manager
 *                    Called from crypto init context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS __wlan_crypto_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_vdev_create_handler(
				WLAN_UMAC_COMP_CRYPTO,
				wlan_crypto_vdev_obj_create_handler, NULL);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_objmgr_register_peer_create_handler(
				WLAN_UMAC_COMP_CRYPTO,
				wlan_crypto_peer_obj_create_handler, NULL);
	if (status != QDF_STATUS_SUCCESS)
		goto err_peer_create;

	status = wlan_objmgr_register_vdev_destroy_handler(
				WLAN_UMAC_COMP_CRYPTO,
				wlan_crypto_vdev_obj_destroy_handler, NULL);
	if (status != QDF_STATUS_SUCCESS)
		goto err_vdev_delete;

	status = wlan_objmgr_register_peer_destroy_handler(
				WLAN_UMAC_COMP_CRYPTO,
				wlan_crypto_peer_obj_destroy_handler, NULL);
	if (status != QDF_STATUS_SUCCESS)
		goto err_peer_delete;

	goto register_success;
err_peer_delete:
	wlan_objmgr_unregister_vdev_destroy_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_vdev_obj_destroy_handler, NULL);
err_vdev_delete:
	wlan_objmgr_unregister_peer_create_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_peer_obj_create_handler, NULL);
err_peer_create:
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_vdev_obj_create_handler, NULL);

register_success:
	return status;
}

/**
 * __wlan_crypto_deinit - Deinit the crypto service with object manager
 *                         Called from crypto context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS __wlan_crypto_deinit(void)
{

	if (wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_vdev_obj_create_handler, NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_peer_create_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_peer_obj_create_handler, NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_vdev_destroy_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_vdev_obj_destroy_handler, NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_peer_destroy_handler(WLAN_UMAC_COMP_CRYPTO,
			wlan_crypto_peer_obj_destroy_handler, NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

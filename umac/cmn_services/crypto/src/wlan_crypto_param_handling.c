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
 * DOC: Public APIs for crypto service
 */
/* include files */
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
#include "wlan_crypto_param_handling_i.h"

static uint32_t
cipher2cap(int cipher)
{
	switch (cipher)	{
	case WLAN_CRYPTO_CIPHER_WEP:  return WLAN_CRYPTO_CAP_WEP;
	case WLAN_CRYPTO_CIPHER_WEP_40:  return WLAN_CRYPTO_CAP_WEP;
	case WLAN_CRYPTO_CIPHER_WEP_104:  return WLAN_CRYPTO_CAP_WEP;
	case WLAN_CRYPTO_CIPHER_AES_OCB:  return WLAN_CRYPTO_CAP_AES;
	case WLAN_CRYPTO_CIPHER_AES_CCM:  return WLAN_CRYPTO_CAP_AES;
	case WLAN_CRYPTO_CIPHER_AES_CCM_256:  return WLAN_CRYPTO_CAP_AES;
	case WLAN_CRYPTO_CIPHER_AES_GCM:  return WLAN_CRYPTO_CAP_AES;
	case WLAN_CRYPTO_CIPHER_AES_GCM_256:  return WLAN_CRYPTO_CAP_AES;
	case WLAN_CRYPTO_CIPHER_CKIP: return WLAN_CRYPTO_CAP_CKIP;
	case WLAN_CRYPTO_CIPHER_TKIP: return WLAN_CRYPTO_CAP_TKIP_MIC;
	case WLAN_CRYPTO_CIPHER_WAPI_SMS4: return WLAN_CRYPTO_CAP_WAPI_SMS4;
	case WLAN_CRYPTO_CIPHER_WAPI_GCM4: return WLAN_CRYPTO_CAP_WAPI_GCM4;
	case WLAN_CRYPTO_CIPHER_FILS_AEAD: return WLAN_CRYPTO_CAP_FILS_AEAD;
	}
	return 0;
}

/**
 * wlan_crypto_set_authmode - called by ucfg to configure authmode for vdev
 * @vdev: vdev
 * @authmode: authmode
 *
 * This function gets called from ucfg to configure authmode for vdev.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_authmode(struct wlan_crypto_params *crypto_params,
					uint32_t authmode)
{
	crypto_params->authmodeset = authmode;
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_crypto_get_authmode - called by ucfg to get authmode of particular vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get authmode of particular vdev
 *
 * Return: authmode
 */
int32_t wlan_crypto_get_authmode(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->authmodeset;
}

/**
 * wlan_crypto_set_mcastcipher - called by ucfg to configure mcastcipher in vdev
 * @vdev: vdev
 * @wlan_crypto_cipher_type: mcast cipher value.
 *
 * This function gets called from ucfg to configure mcastcipher in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_mcastcipher(struct wlan_crypto_params *crypto_params,
					wlan_crypto_cipher_type cipher)
{
	uint16_t i;
	uint32_t cap;
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	RESET_MCAST_CIPHERS(crypto_params);

	for (i = 0; i < WLAN_CRYPTO_CIPHER_MAX; i++) {
		if (HAS_PARAM(cipher, i)) {
			cap = cipher2cap(i);
			if (cap && HAS_CIPHER_CAP(crypto_params, cap)) {
				SET_MCAST_CIPHER(crypto_params, i);
				status = QDF_STATUS_SUCCESS;
			}
		}
		CLEAR_PARAM(cipher, i);
	}
	return status;
}
/**
 * wlan_crypto_get_mcastcipher - called by ucfg to get mcastcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get mcastcipher of particular vdev
 *
 * Return: mcast cipher
 */
int32_t wlan_crypto_get_mcastcipher(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->mcastcipherset;
}

/**
 * wlan_crypto_set_ucastciphers - called by ucfg to configure
 *                                        unicast ciphers in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_ucastciphers(
				struct wlan_crypto_params *crypto_params,
				uint32_t cipher)
{
	uint16_t i;
	uint32_t cap;
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	RESET_UCAST_CIPHERS(crypto_params);

	for (i = 0; i < WLAN_CRYPTO_CIPHER_MAX ; i++) {
		if (HAS_PARAM(cipher, i)) {
			cap = cipher2cap(i);
			if (cap && HAS_CIPHER_CAP(crypto_params, cap)) {
				SET_UCAST_CIPHER(crypto_params, i);
				status = QDF_STATUS_SUCCESS;
			}
		}
		CLEAR_PARAM(cipher, i);
	}

	return status;
}

/**
 * wlan_crypto_get_ucastciphers - called by ucfg to get ucastcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_ucastciphers(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->ucastcipherset;
}

/**
 * wlan_crypto_set_mgmtcipher - called by ucfg to configure
 *                                        mgmt ciphers in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_mgmtcipher(
				struct wlan_crypto_params *crypto_params,
				uint32_t value)
{
	SET_MGMT_CIPHER(crypto_params, value);
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_crypto_get_mgmtciphers - called by ucfg to get mgmtcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_mgmtciphers(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->mgmtcipherset;
}

/**
 * wlan_crypto_set_cipher_cap - called by ucfg to configure
 *                                        cipher cap in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_cipher_cap(
				struct wlan_crypto_params *crypto_params,
				uint32_t value)
{
	crypto_params->cipher_caps = value;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_crypto_get_cipher_cap - called by ucfg to get cipher caps from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_cipher_cap(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->cipher_caps;
}

/**
 * wlan_crypto_set_rsn_cap - called by ucfg to configure
 *                                        cipher cap in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_rsn_cap(
				struct wlan_crypto_params *crypto_params,
				uint32_t value)
{
	crypto_params->rsn_caps = value;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_crypto_get_rsn_cap - called by ucfg to get rsn caps from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_rsn_cap(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->rsn_caps;
}


/**
 * wlan_crypto_set_key_mgmt - called by ucfg to configure
 *                                        key_mgmt in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_key_mgmt(
				struct wlan_crypto_params *crypto_params,
				uint32_t value)
{
	crypto_params->key_mgmt = value;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_crypto_get_key_mgmt - called by ucfg to get key mgmt from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_key_mgmt(struct wlan_crypto_params *crypto_params)
{
	return crypto_params->key_mgmt;
}

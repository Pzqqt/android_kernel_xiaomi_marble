/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
 * DOC: defines crypto driver functions interfacing with linux kernel
 */
#include <wlan_crypto_global_def.h>
#include <wlan_crypto_global_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_crypto_main_i.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_crypto_def_i.h>
#include <wlan_crypto_obj_mgr_i.h>
#include <net/cfg80211.h>
#include <wlan_nl_to_crypto_params.h>
#include "wlan_cfg80211_crypto.h"
#include <wlan_cfg80211.h>
#include <wlan_osif_request_manager.h>

static void wlan_cfg80211_translate_key(struct wlan_objmgr_vdev *vdev,
					uint8_t key_index,
					enum wlan_crypto_key_type key_type,
					const u8 *mac_addr,
					struct key_params *params,
					struct wlan_crypto_key *crypto_key)
{
	qdf_mem_zero(crypto_key, sizeof(*crypto_key));
	crypto_key->keylen = params->key_len;
	crypto_key->keyix = key_index;
	osif_debug("key_type %d, opmode %d, key_len %d, seq_len %d",
		   key_type, vdev->vdev_mlme.vdev_opmode,
		   params->key_len, params->seq_len);
	qdf_mem_copy(&crypto_key->keyval[0], params->key, params->key_len);
	qdf_mem_copy(&crypto_key->keyrsc[0], params->seq, params->seq_len);

	crypto_key->cipher_type = osif_nl_to_crypto_cipher_type(params->cipher);
	if (IS_WEP_CIPHER(crypto_key->cipher_type) && !mac_addr) {
		/*
		 * This is a valid scenario in case of WEP, where-in the
		 * keys are passed by the user space during the connect request
		 * but since we did not connect yet, so we do not know the peer
		 * address yet.
		 */
		osif_debug("No Mac Address to copy");
		return;
	}
	if (key_type == WLAN_CRYPTO_KEY_TYPE_UNICAST) {
		qdf_mem_copy(&crypto_key->macaddr, mac_addr, QDF_MAC_ADDR_SIZE);
	} else {
		if ((vdev->vdev_mlme.vdev_opmode == QDF_STA_MODE) ||
		    (vdev->vdev_mlme.vdev_opmode == QDF_P2P_CLIENT_MODE))
			qdf_mem_copy(&crypto_key->macaddr, mac_addr,
				     QDF_MAC_ADDR_SIZE);
		else
			qdf_mem_copy(&crypto_key->macaddr,
				     vdev->vdev_mlme.macaddr,
				     QDF_MAC_ADDR_SIZE);
	}
	osif_debug("mac "QDF_MAC_ADDR_FMT,
		   QDF_MAC_ADDR_REF(crypto_key->macaddr));
}

int wlan_cfg80211_store_key(struct wlan_objmgr_vdev *vdev,
			    uint8_t key_index,
			    enum wlan_crypto_key_type key_type,
			    const u8 *mac_addr, struct key_params *params)
{
	struct wlan_crypto_key *crypto_key = NULL;
	enum wlan_crypto_cipher_type cipher;
	int cipher_len;
	QDF_STATUS status;

	if (!vdev) {
		osif_err("vdev is NULL");
		return -EINVAL;
	}
	if (!params) {
		osif_err("Key params is NULL");
		return -EINVAL;
	}
	cipher_len = osif_nl_to_crypto_cipher_len(params->cipher);
	if (cipher_len < 0 || params->key_len < cipher_len) {
		osif_err("cipher length %d less than reqd len %d",
			 params->key_len, cipher_len);
		return -EINVAL;
	}
	cipher = osif_nl_to_crypto_cipher_type(params->cipher);
	if (!IS_WEP_CIPHER(cipher)) {
		if ((key_type == WLAN_CRYPTO_KEY_TYPE_UNICAST) &&
		    !mac_addr) {
			osif_err("mac_addr is NULL for pairwise Key");
			return -EINVAL;
		}
	}
	status = wlan_crypto_validate_key_params(cipher, key_index,
						 params->key_len,
						 params->seq_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Invalid key params");
		return -EINVAL;
	}

	/*
	 * key may already exist at times and may be retrieved only to
	 * update it.
	 */
	crypto_key = wlan_crypto_get_key(vdev, key_index);
	if (!crypto_key) {
		crypto_key = qdf_mem_malloc(sizeof(*crypto_key));
		if (!crypto_key)
			return -EINVAL;
	}

	wlan_cfg80211_translate_key(vdev, key_index, key_type, mac_addr,
				    params, crypto_key);

	status = wlan_crypto_save_key(vdev, key_index, crypto_key);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to save key");
		qdf_mem_free(crypto_key);
		return -EINVAL;
	}
	return 0;
}

#define WLAN_WAIT_TIME_ADD_KEY 100

static void
wlan_cfg80211_crypto_add_key_cb(void *context,
				struct crypto_add_key_result *result)
{
	struct osif_request *request;
	struct crypto_add_key_result *priv;

	request = osif_request_get(context);
	if (!request) {
		osif_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	qdf_mem_copy(priv, result, sizeof(*priv));
	osif_request_complete(request);
	osif_request_put(request);
}

int wlan_cfg80211_crypto_add_key(struct wlan_objmgr_vdev *vdev,
				 enum wlan_crypto_key_type key_type,
				 uint8_t key_index, bool sync)
{
	struct wlan_crypto_key *crypto_key;
	QDF_STATUS status;
	struct osif_request *request;
	struct crypto_add_key_result *result;
	struct wlan_crypto_comp_priv *priv;
	int ret;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*result),
		.timeout_ms = WLAN_WAIT_TIME_ADD_KEY,
	};

	crypto_key = wlan_crypto_get_key(vdev, key_index);
	if (!crypto_key) {
		osif_err("Crypto KEY is NULL");
		return -EINVAL;
	}

	if (sync) {
		priv = wlan_get_vdev_crypto_obj(vdev);
		if (!priv) {
			osif_err("Invalid crypto_priv");
			return -EINVAL;
		}

		request = osif_request_alloc(&params);
		if (!request) {
			osif_err("Request allocation failure");
			return -ENOMEM;
		}

		priv->add_key_ctx = osif_request_cookie(request);;
		priv->add_key_cb = wlan_cfg80211_crypto_add_key_cb;

		status  = ucfg_crypto_set_key_req(vdev, crypto_key, key_type);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			ret = osif_request_wait_for_response(request);
			if (ret) {
				osif_err("Target response timed out");
			} else {
				result = osif_request_priv(request);
				osif_debug("complete, vdev_id %u, ix: %u, flags: %u, status: %u",
					   result->vdev_id, result->key_ix,
					   result->key_flags, result->status);
			}
		}

		priv->add_key_ctx = NULL;
		priv->add_key_cb = NULL;
		osif_request_put(request);
	} else {
		status  = ucfg_crypto_set_key_req(vdev, crypto_key, key_type);
	}

	return qdf_status_to_os_return(status);
}

int wlan_cfg80211_set_default_key(struct wlan_objmgr_vdev *vdev,
				  uint8_t key_index, struct qdf_mac_addr *bssid)
{
	return wlan_crypto_default_key(vdev, (uint8_t *)bssid,
				       key_index, true);
}

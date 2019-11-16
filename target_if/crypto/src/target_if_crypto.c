/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: offload lmac interface APIs definitions for crypto
 */

#include <qdf_mem.h>
#include <qdf_status.h>
#include <target_if_crypto.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_param.h>
#include <wlan_objmgr_psoc_obj.h>
#include <target_if.h>
#include <wlan_crypto_global_def.h>
#include <wlan_crypto_global_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <cdp_txrx_cmn_struct.h>
#include <cds_api.h>
#include <cdp_txrx_cmn.h>
#include <wmi_unified_api.h>
#include <wmi_unified_crypto_api.h>
#include <cdp_txrx_peer_ops.h>

#ifdef FEATURE_WLAN_WAPI
static void wlan_crypto_set_wapi_key(struct wlan_objmgr_vdev *vdev,
				     bool pairwise,
				     enum wlan_crypto_cipher_type cipher_type,
				     struct set_key_params *params)
{
	static const unsigned char tx_iv[16] = {0x36, 0x5c, 0x36, 0x5c, 0x36,
						0x5c, 0x36, 0x5c, 0x36, 0x5c,
						0x36, 0x5c, 0x36, 0x5c, 0x36,
						0x5c};

	static const unsigned char rx_iv[16] = {0x5c, 0x36, 0x5c, 0x36, 0x5c,
						0x36, 0x5c, 0x36, 0x5c, 0x36,
						0x5c, 0x36, 0x5c, 0x36, 0x5c,
						0x37};

	if (cipher_type != WLAN_CRYPTO_CIPHER_WAPI_SMS4 ||
	    cipher_type != WLAN_CRYPTO_CIPHER_WAPI_GCM4)
		return;

	qdf_mem_copy(&params->rx_iv, &rx_iv,
		     WLAN_CRYPTO_WAPI_IV_SIZE);
	qdf_mem_copy(&params->tx_iv, &tx_iv,
		     WLAN_CRYPTO_WAPI_IV_SIZE);

	if (vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE) {
		if (pairwise)
			params->tx_iv[0] = 0x37;

		params->rx_iv[WLAN_CRYPTO_WAPI_IV_SIZE - 1] = 0x36;
	} else {
		if (!pairwise)
			params->rx_iv[WLAN_CRYPTO_WAPI_IV_SIZE - 1] = 0x36;
	}

	params->key_txmic_len = WLAN_CRYPTO_MIC_LEN;
	params->key_rxmic_len = WLAN_CRYPTO_MIC_LEN;
}
#else
static inline void wlan_crypto_set_wapi_key(struct wlan_objmgr_vdev *vdev,
					    bool pairwise,
					    enum wlan_crypto_cipher_type cipher,
					    struct set_key_params *params)
{
}
#endif /* FEATURE_WLAN_WAPI */

#ifdef BIG_ENDIAN_HOST
static void wlan_crypto_endianness_conversion(uint8_t *dest, uint8_t *src,
					      uint32_t keylen)
{
	int8_t i;

	for (i = 0; i < roundup(keylen, sizeof(uint32_t)) / 4; i++) {
		*dest = le32_to_cpu(*src);
		dest++;
		src++;
	}
}
#else
static void wlan_crypto_endianness_conversion(uint8_t *dest, uint8_t *src,
					      uint32_t keylen)
{
	qdf_mem_copy(dest, src, keylen);
}
#endif

QDF_STATUS target_if_crypto_set_key(struct wlan_objmgr_vdev *vdev,
				    struct wlan_crypto_key *req,
				    enum wlan_crypto_key_type key_type)
{
	struct set_key_params params = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	enum cdp_sec_type sec_type = cdp_sec_type_none;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct cdp_pdev *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct cdp_vdev *txrx_vdev;
	uint32_t pn[4] = {0, 0, 0, 0};
	struct cdp_peer *peer = NULL;
	uint8_t def_tx_idx;
	wmi_unified_t pdev_wmi_handle;
	bool pairwise;
	QDF_STATUS status;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		target_if_err("Invalid PDEV");
		return QDF_STATUS_E_FAILURE;
	}
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("Invalid PSOC");
		return QDF_STATUS_E_FAILURE;
	}
	soc = wlan_psoc_get_dp_handle(psoc);
	if (!soc) {
		target_if_err("Invalid DP Handle");
		return QDF_STATUS_E_FAILURE;
	}
	params.vdev_id = wlan_vdev_get_id(vdev);
	params.key_idx = req->keyix;
	qdf_mem_copy(params.peer_mac, req->macaddr, QDF_MAC_ADDR_SIZE);
	pdev_wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!pdev_wmi_handle) {
		target_if_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	params.key_flags = req->flags;
	if (key_type != WLAN_CRYPTO_KEY_TYPE_UNICAST) {
		pairwise = false;
		params.key_flags |= GROUP_USAGE;

	} else {
		pairwise = true;
		params.key_flags |= PAIRWISE_USAGE;
	}
	qdf_mem_copy(&params.key_rsc_ctr,
		     &req->keyrsc[0], sizeof(uint64_t));
	txrx_vdev = (struct cdp_vdev *)cdp_get_vdev_from_vdev_id(soc,
				(struct cdp_pdev *)txrx_pdev, params.vdev_id);
	peer = cdp_peer_find_by_addr(soc, txrx_pdev, req->macaddr);

	if (!txrx_vdev) {
		target_if_err("Invalid txrx vdev");
		return QDF_STATUS_E_FAILURE;
	}

	target_if_debug("key_type %d, mac: %02x:%02x:%02x:%02x:%02x:%02x",
			key_type, req->macaddr[0], req->macaddr[1],
			req->macaddr[2], req->macaddr[3], req->macaddr[4],
			req->macaddr[5]);

	if ((key_type == WLAN_CRYPTO_KEY_TYPE_UNICAST) && !peer) {
		target_if_err("Invalid peer");
		return QDF_STATUS_E_FAILURE;
	}

	params.key_cipher = wlan_crypto_cipher_to_wmi_cipher(req->cipher_type);
	sec_type = wlan_crypto_cipher_to_cdp_sec_type(req->cipher_type);
	wlan_crypto_set_wapi_key(vdev, pairwise, req->cipher_type, &params);

	switch (req->cipher_type) {
	case WLAN_CRYPTO_CIPHER_WEP:
	case WLAN_CRYPTO_CIPHER_WEP_40:
	case WLAN_CRYPTO_CIPHER_WEP_104:
		def_tx_idx = wlan_crypto_get_default_key_idx(vdev, false);
		if (pairwise && params.key_idx == def_tx_idx)
			params.key_flags |= TX_USAGE;
		else if ((vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE) &&
			 (params.key_idx == def_tx_idx))
			params.key_flags |= TX_USAGE;
		break;
	case WLAN_CRYPTO_CIPHER_TKIP:
		params.key_txmic_len = WLAN_CRYPTO_MIC_LEN;
		params.key_rxmic_len = WLAN_CRYPTO_MIC_LEN;
		break;
	default:
		break;
	}

	wlan_crypto_endianness_conversion(&params.key_data[0],
					  &req->keyval[0],
					  req->keylen);
	params.key_len = req->keylen;
	if (peer) {
		/* Set PN check & security type in data path */
		qdf_mem_copy(&pn[0], &params.key_rsc_ctr, sizeof(pn));
		cdp_set_pn_check(soc, txrx_vdev, peer, sec_type, pn);
		cdp_set_key(soc, peer, pairwise, (uint32_t *)(req->keyval +
			    WLAN_CRYPTO_IV_SIZE + WLAN_CRYPTO_MIC_LEN));
	} else {
		target_if_info("peer not found");
	}

	target_if_debug("vdev_id:%d, key: idx:%d,len:%d", params.vdev_id,
			params.key_idx, params.key_len);
	target_if_debug("peer mac %pM", params.peer_mac);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_CRYPTO, QDF_TRACE_LEVEL_DEBUG,
			   &params.key_rsc_ctr, sizeof(uint64_t));
	status = wmi_unified_setup_install_key_cmd(pdev_wmi_handle, &params);

	/* Zero-out local key variables */
	qdf_mem_zero(&params, sizeof(struct set_key_params));
	return status;
}

QDF_STATUS target_if_crypto_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_crypto_tx_ops *crypto;

	if (!tx_ops) {
		target_if_err("txops NULL");
		return QDF_STATUS_E_FAILURE;
	}
	crypto = &tx_ops->crypto_tx_ops;

	crypto->set_key = target_if_crypto_set_key;

	return QDF_STATUS_SUCCESS;
}


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
 * DOC: wlan_nl_to_crypto_params.c
 *
 * Conversion of NL param type to Crypto param type APIs implementation
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/cfg80211.h>

#include <qdf_types.h>
#include "wlan_objmgr_vdev_obj.h"
#include <qdf_module.h>

#include "wlan_nl_to_crypto_params.h"
#include "wlan_crypto_global_def.h"

/**
 * struct osif_akm_crypto_mapping - mapping akm type received from
 *                                 NL to internal crypto type
 * @akm_suite: NL akm type
 * @akm_type_crypto: akm crypto type
 *
 * mapping akm type received from NL to internal crypto type
 */
struct osif_akm_type_crypto_mapping {
	u32 akm_suite;
	wlan_crypto_key_mgmt akm_type_crypto;
};

/**
 * struct osif_cipher_crypto_mapping - mapping cipher type received from NL
 *                                    to internal crypto cipher type
 * @cipher_suite: NL cipher type
 * @cipher_crypto: cipher crypto type
 * @cipher_len: Length of the cipher
 *
 * mapping cipher type received from NL to internal crypto cipher type
 */
struct osif_cipher_crypto_mapping {
	u32 cipher_suite;
	wlan_crypto_cipher_type cipher_crypto;
	u32 cipher_len;
};

/**
 * mapping table for auth type received from NL and cryto auth type
 */
static const wlan_crypto_auth_mode
	osif_auth_type_crypto_mapping[] = {
	[NL80211_AUTHTYPE_AUTOMATIC] = WLAN_CRYPTO_AUTH_AUTO,
	[NL80211_AUTHTYPE_OPEN_SYSTEM] = WLAN_CRYPTO_AUTH_OPEN,
	[NL80211_AUTHTYPE_FT] = WLAN_CRYPTO_AUTH_OPEN,
	[NL80211_AUTHTYPE_SHARED_KEY] = WLAN_CRYPTO_AUTH_SHARED,
	[NL80211_AUTHTYPE_NETWORK_EAP] = WLAN_CRYPTO_AUTH_8021X,
#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
	[NL80211_AUTHTYPE_FILS_SK] = WLAN_CRYPTO_AUTH_FILS_SK,
#endif
	[NL80211_AUTHTYPE_SAE] = WLAN_CRYPTO_AUTH_SAE,
};

/* mapping table for akm type received from NL and cryto akm type */
static const struct osif_akm_type_crypto_mapping
	osif_akm_type_crypto_mapping[] = {
	{
		.akm_suite = WLAN_AKM_SUITE_8021X,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_IEEE8021X,
	},
	{
		.akm_suite = WLAN_AKM_SUITE_PSK,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_PSK,
	},
	{
		.akm_suite = WLAN_AKM_SUITE_8021X_SHA256,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256,
	},
	{
		.akm_suite = WLAN_AKM_SUITE_PSK_SHA256,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_PSK_SHA256,
	},
	{
		.akm_suite = WLAN_AKM_SUITE_SAE,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_SAE,
	},
	{
		.akm_suite = WLAN_AKM_SUITE_FT_OVER_SAE,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FT_SAE,
	},
#if defined(WLAN_AKM_SUITE_FT_8021X) || \
			defined(FEATURE_WLAN_FT_IEEE8021X)
	{
		.akm_suite = WLAN_AKM_SUITE_FT_8021X,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X,
	},
#endif
#if defined(WLAN_AKM_SUITE_FT_PSK) || \
			defined(FEATURE_WLAN_FT_PSK)
	{
		.akm_suite = WLAN_AKM_SUITE_FT_PSK,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FT_PSK,
	},
#endif
#if defined(WLAN_AKM_SUITE_8021X_SUITE_B) || \
		defined(FEATURE_WLAN_IEEE8021X_SUITE_B)
	{
		.akm_suite = WLAN_AKM_SUITE_8021X_SUITE_B,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B,
	},
#endif
#if defined(WLAN_AKM_SUITE_8021X_SUITE_B_192) || \
		defined(FEATURE_WLAN_IEEE8021X_SUITE_B)
	{
		.akm_suite = WLAN_AKM_SUITE_8021X_SUITE_B_192,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192,
	},
#endif
#if defined(WLAN_AKM_SUITE_FILS_SHA256) || \
				defined(FEATURE_WLAN_FILS)
	{
		.akm_suite = WLAN_AKM_SUITE_FILS_SHA256,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FILS_SHA256,
	},
#endif
#if defined(WLAN_AKM_SUITE_FILS_SHA384) || \
				defined(FEATURE_WLAN_FILS)
	{
		.akm_suite = WLAN_AKM_SUITE_FILS_SHA384,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FILS_SHA384,
	},
#endif
#if defined(WLAN_AKM_SUITE_FT_FILS_SHA256) || \
				defined(FEATURE_WLAN_FILS)
	{
		.akm_suite = WLAN_AKM_SUITE_FT_FILS_SHA256,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256,
	},
#endif
#if defined(WLAN_AKM_SUITE_FT_FILS_SHA384) || \
				defined(FEATURE_WLAN_FILS)
	{
		.akm_suite = WLAN_AKM_SUITE_FT_FILS_SHA384,
		.akm_type_crypto = WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384,
	},
#endif
};

/* mapping table for cipher type received from NL and cryto cipher type */
static const struct osif_cipher_crypto_mapping
	osif_cipher_crypto_mapping[] = {
	{
		.cipher_suite = IW_AUTH_CIPHER_NONE,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_NONE,
		.cipher_len = 0,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_WEP40,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_WEP_40,
		.cipher_len = WLAN_CRYPTO_KEY_WEP40_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_TKIP,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_TKIP,
		.cipher_len = WLAN_CRYPTO_KEY_TKIP_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_CCMP,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_CCM,
		.cipher_len = WLAN_CRYPTO_KEY_CCMP_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_WEP104,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_WEP_104,
		.cipher_len = WLAN_CRYPTO_KEY_WEP104_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_GCMP,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_GCM,
		.cipher_len = WLAN_CRYPTO_KEY_GCMP_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_GCMP_256,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_GCM_256,
		.cipher_len = WLAN_CRYPTO_KEY_GCMP_256_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_CCMP_256,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_CCM_256,
		.cipher_len = WLAN_CRYPTO_KEY_CCMP_256_LEN,
	},
	{
		.cipher_suite = WLAN_CIPHER_SUITE_AES_CMAC,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_CMAC,
		.cipher_len = WLAN_CRYPTO_KEY_CCMP_LEN,
	},
#ifdef WLAN_CIPHER_SUITE_BIP_GMAC_128
	{
		.cipher_suite = WLAN_CIPHER_SUITE_BIP_GMAC_128,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_GMAC,
		.cipher_len = WLAN_CRYPTO_KEY_GMAC_LEN,
	},
#endif
#ifdef WLAN_CIPHER_SUITE_BIP_GMAC_256
	{
		.cipher_suite = WLAN_CIPHER_SUITE_BIP_GMAC_256,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_GMAC_256,
		.cipher_len = WLAN_CRYPTO_KEY_GMAC_256_LEN,
	},
#endif
#ifdef WLAN_CIPHER_SUITE_BIP_CMAC_256
	{
		.cipher_suite = WLAN_CIPHER_SUITE_BIP_CMAC_256,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_AES_CMAC_256,
		.cipher_len = WLAN_CRYPTO_KEY_CCMP_256_LEN,
	},
#endif
#ifdef FEATURE_WLAN_WAPI
	{
		.cipher_suite = WLAN_CIPHER_SUITE_SMS4,
		.cipher_crypto = WLAN_CRYPTO_CIPHER_WAPI_SMS4,
		.cipher_len = WLAN_CRYPTO_KEY_WAPI_LEN,
	},
#endif
};

int osif_nl_to_crypto_auth_type(enum nl80211_auth_type auth_type)
{
	wlan_crypto_auth_mode crypto_auth_type = WLAN_CRYPTO_AUTH_NONE;

	if (auth_type < NL80211_AUTHTYPE_OPEN_SYSTEM ||
	    auth_type >= QDF_ARRAY_SIZE(osif_auth_type_crypto_mapping)) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, "Unknown type: %d",
				auth_type);
		return -EINVAL;
	}
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, "Auth type, NL: %d, crypto: %d",
			auth_type, osif_auth_type_crypto_mapping[auth_type]);

	return crypto_auth_type;
}

int osif_nl_to_crypto_akm_type(u32 key_mgmt)
{
	uint8_t index;
	wlan_crypto_key_mgmt crypto_akm_type = WLAN_CRYPTO_KEY_MGMT_NONE;
	bool akm_type_crypto_exist = false;

	for (index = 0; index < QDF_ARRAY_SIZE(osif_akm_type_crypto_mapping);
	     index++) {
		if (osif_akm_type_crypto_mapping[index].akm_suite == key_mgmt) {
			crypto_akm_type = osif_akm_type_crypto_mapping[index].
							akm_type_crypto;
			akm_type_crypto_exist = true;
			break;
		}
	}
	if (!akm_type_crypto_exist) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, "Unknown type: %d",
				key_mgmt);
		return -EINVAL;
	}
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, "Akm suite, NL: %d, crypto: %d",
			key_mgmt, crypto_akm_type);

	return crypto_akm_type;
}

enum wlan_crypto_cipher_type osif_nl_to_crypto_cipher_type(u32 cipher)
{
	uint8_t index;
	bool cipher_crypto_exist = false;
	wlan_crypto_cipher_type crypto_cipher_type = WLAN_CRYPTO_CIPHER_NONE;

	for (index = 0; index < QDF_ARRAY_SIZE(osif_cipher_crypto_mapping);
	     index++) {
		if (osif_cipher_crypto_mapping[index].cipher_suite == cipher) {
			crypto_cipher_type = osif_cipher_crypto_mapping[index].
								cipher_crypto;
			cipher_crypto_exist = true;
			break;
		}
	}
	if (!cipher_crypto_exist) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, "Unknown type: %d",
				cipher);
		return WLAN_CRYPTO_CIPHER_INVALID;
	}
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, "Cipher suite, NL: %d, crypto: %d",
			cipher, crypto_cipher_type);

	return crypto_cipher_type;
}

int osif_nl_to_crypto_cipher_len(u32 cipher)
{
	uint8_t index;

	for (index = 0; index < QDF_ARRAY_SIZE(osif_cipher_crypto_mapping);
	     index++) {
		if (osif_cipher_crypto_mapping[index].cipher_suite == cipher)
			return osif_cipher_crypto_mapping[index].cipher_len;
	}

	return -EINVAL;
}


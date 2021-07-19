/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: Public definations  for crypto service
 */

#ifndef _WLAN_CRYPTO_GLOBAL_DEF_H_
#define _WLAN_CRYPTO_GLOBAL_DEF_H_

#include <wlan_cmn.h>
#ifdef WLAN_CRYPTO_SUPPORT_FILS
#include "wlan_crypto_fils_def.h"
#endif
#include <wlan_objmgr_cmn.h>
#include <wlan_cmn_ieee80211.h>

#define WLAN_CRYPTO_TID_SIZE         (17)
#define WLAN_CRYPTO_RSC_SIZE         (16)
#define WLAN_CRYPTO_KEYBUF_SIZE      (32)
#define WLAN_CRYPTO_MICBUF_SIZE      (16)
#define WLAN_CRYPTO_MIC_LEN          (8)
#define WLAN_CRYPTO_IV_SIZE          (16)
#define WLAN_CRYPTO_MIC256_LEN       (16)
#define WLAN_CRYPTO_TXMIC_OFFSET     (0)
#define WLAN_CRYPTO_RXMIC_OFFSET     (WLAN_CRYPTO_TXMIC_OFFSET + \
					WLAN_CRYPTO_MIC_LEN)
#define WLAN_CRYPTO_WAPI_IV_SIZE     (16)
#define WLAN_CRYPTO_CRC_LEN          (4)
#define WLAN_CRYPTO_IV_LEN           (3)
#define WLAN_CRYPTO_KEYID_LEN        (1)
#define WLAN_CRYPTO_EXT_IV_LEN       (4)
#define WLAN_CRYPTO_EXT_IV_BIT       (0x20)
#define WLAN_CRYPTO_KEYIX_NONE       ((uint16_t)-1)
#define WLAN_CRYPTO_MAXKEYIDX        (4)
#define WLAN_CRYPTO_MAXIGTKKEYIDX    (2)
#define WLAN_CRYPTO_MAXBIGTKKEYIDX   (2)
#ifndef WLAN_CRYPTO_MAX_VLANKEYIX
#define WLAN_CRYPTO_MAX_VLANKEYIX    WLAN_CRYPTO_MAXKEYIDX
#endif
#define WLAN_CRYPTO_MAX_PMKID        (16)

/* 40 bit wep key len */
#define WLAN_CRYPTO_KEY_WEP40_LEN    (5)
/* 104 bit wep key len */
#define WLAN_CRYPTO_KEY_WEP104_LEN   (13)
/* 128 bit wep key len */
#define WLAN_CRYPTO_KEY_WEP128_LEN   (16)

#define WLAN_CRYPTO_KEY_TKIP_LEN     (32)
#define WLAN_CRYPTO_KEY_CCMP_LEN     (16)
#define WLAN_CRYPTO_KEY_CCMP_256_LEN (32)
#define WLAN_CRYPTO_KEY_GCMP_LEN     (16)
#define WLAN_CRYPTO_KEY_GCMP_256_LEN (32)
#define WLAN_CRYPTO_KEY_WAPI_LEN     (32)
#define WLAN_CRYPTO_KEY_GMAC_LEN     (16)
#define WLAN_CRYPTO_KEY_GMAC_256_LEN (32)
#define WLAN_CRYPTO_WPI_SMS4_IVLEN   (16)
#define WLAN_CRYPTO_WPI_SMS4_KIDLEN  (1)
#define WLAN_CRYPTO_WPI_SMS4_PADLEN  (1)
#define WLAN_CRYPTO_WPI_SMS4_MICLEN  (16)

/* FILS definitions */
#define WLAN_CRYPTO_FILS_OPTIONAL_DATA_LEN 3
#define WLAN_CRYPTO_FILS_RIK_LABEL "Re-authentication Integrity Key@ietf.org"

/* key used for xmit */
#define WLAN_CRYPTO_KEY_XMIT         (0x01)
/* key used for recv */
#define WLAN_CRYPTO_KEY_RECV         (0x02)
/* key used for WPA group operation */
#define WLAN_CRYPTO_KEY_GROUP        (0x04)
/* key also used for management frames */
#define WLAN_CRYPTO_KEY_MFP          (0x08)
/* host-based encryption */
#define WLAN_CRYPTO_KEY_SWENCRYPT    (0x10)
/* host-based enmic */
#define WLAN_CRYPTO_KEY_SWENMIC      (0x20)
/* do not remove unless OS commands us to do so */
#define WLAN_CRYPTO_KEY_PERSISTENT   (0x40)
/* per STA default key */
#define WLAN_CRYPTO_KEY_DEFAULT      (0x80)
/* host-based decryption */
#define WLAN_CRYPTO_KEY_SWDECRYPT    (0x100)
/* host-based demic */
#define WLAN_CRYPTO_KEY_SWDEMIC      (0x200)
/* get pn from fw for key */
#define WLAN_CRYPTO_KEY_GET_PN       (0x400)

#define WLAN_CRYPTO_KEY_SWCRYPT      (WLAN_CRYPTO_KEY_SWENCRYPT \
						| WLAN_CRYPTO_KEY_SWDECRYPT)

#define WLAN_CRYPTO_KEY_SWMIC        (WLAN_CRYPTO_KEY_SWENMIC \
						| WLAN_CRYPTO_KEY_SWDEMIC)

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
#define WLAN_AKM_SUITE_FT_8021X         0x000FAC03
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#endif

/* Maximum lifetime for a PMKID entry - 12 Hrs */
#define WLAN_CRYPTO_MAX_PMKID_LIFETIME 43200
#define WLAN_CRYPTO_MAX_PMKID_LIFETIME_THRESHOLD 100

/*
 * Cipher types
 */
typedef enum wlan_crypto_cipher_type {
	WLAN_CRYPTO_CIPHER_WEP             = 0,
	WLAN_CRYPTO_CIPHER_TKIP            = 1,
	WLAN_CRYPTO_CIPHER_AES_OCB         = 2,
	WLAN_CRYPTO_CIPHER_AES_CCM         = 3,
	WLAN_CRYPTO_CIPHER_WAPI_SMS4       = 4,
	WLAN_CRYPTO_CIPHER_CKIP            = 5,
	WLAN_CRYPTO_CIPHER_AES_CMAC        = 6,
	WLAN_CRYPTO_CIPHER_AES_CCM_256     = 7,
	WLAN_CRYPTO_CIPHER_AES_CMAC_256    = 8,
	WLAN_CRYPTO_CIPHER_AES_GCM         = 9,
	WLAN_CRYPTO_CIPHER_AES_GCM_256     = 10,
	WLAN_CRYPTO_CIPHER_AES_GMAC        = 11,
	WLAN_CRYPTO_CIPHER_AES_GMAC_256    = 12,
	WLAN_CRYPTO_CIPHER_WAPI_GCM4       = 13,
	WLAN_CRYPTO_CIPHER_FILS_AEAD       = 14,
	WLAN_CRYPTO_CIPHER_WEP_40          = 15,
	WLAN_CRYPTO_CIPHER_WEP_104         = 16,
	WLAN_CRYPTO_CIPHER_NONE            = 17,
	WLAN_CRYPTO_CIPHER_MAX             = (WLAN_CRYPTO_CIPHER_NONE + 1),
	WLAN_CRYPTO_CIPHER_INVALID,
} wlan_crypto_cipher_type;

/* Auth types */
typedef enum wlan_crypto_auth_mode {
	WLAN_CRYPTO_AUTH_NONE     = 0,
	WLAN_CRYPTO_AUTH_OPEN     = 1,
	WLAN_CRYPTO_AUTH_SHARED   = 2,
	WLAN_CRYPTO_AUTH_8021X    = 3,
	WLAN_CRYPTO_AUTH_AUTO     = 4,
	WLAN_CRYPTO_AUTH_WPA      = 5,
	WLAN_CRYPTO_AUTH_RSNA     = 6,
	WLAN_CRYPTO_AUTH_CCKM     = 7,
	WLAN_CRYPTO_AUTH_WAPI     = 8,
	WLAN_CRYPTO_AUTH_SAE      = 9,
	WLAN_CRYPTO_AUTH_FILS_SK  = 10,
	/** Keep WLAN_CRYPTO_AUTH_MAX at the end. */
	WLAN_CRYPTO_AUTH_MAX      = WLAN_CRYPTO_AUTH_FILS_SK,
} wlan_crypto_auth_mode;

/* crypto capabilities */
typedef enum wlan_crypto_cap {
	WLAN_CRYPTO_CAP_PRIVACY          = 0,
	WLAN_CRYPTO_CAP_WPA1             = 1,
	WLAN_CRYPTO_CAP_WPA2             = 2,
	WLAN_CRYPTO_CAP_WPA              = 3,
	WLAN_CRYPTO_CAP_AES              = 4,
	WLAN_CRYPTO_CAP_WEP              = 5,
	WLAN_CRYPTO_CAP_CKIP             = 6,
	WLAN_CRYPTO_CAP_TKIP_MIC         = 7,
	WLAN_CRYPTO_CAP_CCM256           = 8,
	WLAN_CRYPTO_CAP_GCM              = 9,
	WLAN_CRYPTO_CAP_GCM_256          = 10,
	WLAN_CRYPTO_CAP_WAPI_SMS4        = 11,
	WLAN_CRYPTO_CAP_WAPI_GCM4        = 12,
	WLAN_CRYPTO_CAP_KEY_MGMT_OFFLOAD = 13,
	WLAN_CRYPTO_CAP_PMF_OFFLOAD      = 14,
	WLAN_CRYPTO_CAP_PN_TID_BASED     = 15,
	WLAN_CRYPTO_CAP_FILS_AEAD        = 16,
} wlan_crypto_cap;

typedef enum wlan_crypto_rsn_cap {
	WLAN_CRYPTO_RSN_CAP_PREAUTH       = 0x01,
	WLAN_CRYPTO_RSN_CAP_MFP_ENABLED   = 0x80,
	WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED  = 0x40,
	WLAN_CRYPTO_RSN_CAP_OCV_SUPPORTED  = 0x4000,
} wlan_crypto_rsn_cap;

enum wlan_crypto_rsnx_cap {
	WLAN_CRYPTO_RSNX_CAP_PROTECTED_TWT = 0x10,
	WLAN_CRYPTO_RSNX_CAP_SAE_H2E = 0x20,
	WLAN_CRYPTO_RSNX_CAP_SAE_PK = 0x40,
};

typedef enum wlan_crypto_key_mgmt {
	WLAN_CRYPTO_KEY_MGMT_IEEE8021X             = 0,
	WLAN_CRYPTO_KEY_MGMT_PSK                   = 1,
	WLAN_CRYPTO_KEY_MGMT_NONE                  = 2,
	WLAN_CRYPTO_KEY_MGMT_IEEE8021X_NO_WPA      = 3,
	WLAN_CRYPTO_KEY_MGMT_WPA_NONE              = 4,
	WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X          = 5,
	WLAN_CRYPTO_KEY_MGMT_FT_PSK                = 6,
	WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256      = 7,
	WLAN_CRYPTO_KEY_MGMT_PSK_SHA256            = 8,
	WLAN_CRYPTO_KEY_MGMT_WPS                   = 9,
	WLAN_CRYPTO_KEY_MGMT_SAE                   = 10,
	WLAN_CRYPTO_KEY_MGMT_FT_SAE                = 11,
	WLAN_CRYPTO_KEY_MGMT_WAPI_PSK              = 12,
	WLAN_CRYPTO_KEY_MGMT_WAPI_CERT             = 13,
	WLAN_CRYPTO_KEY_MGMT_CCKM                  = 14,
	WLAN_CRYPTO_KEY_MGMT_OSEN                  = 15,
	WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B     = 16,
	WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192 = 17,
	WLAN_CRYPTO_KEY_MGMT_FILS_SHA256           = 18,
	WLAN_CRYPTO_KEY_MGMT_FILS_SHA384           = 19,
	WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256        = 20,
	WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384        = 21,
	WLAN_CRYPTO_KEY_MGMT_OWE                   = 22,
	WLAN_CRYPTO_KEY_MGMT_DPP                   = 23,
	WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384   = 24,
	WLAN_CRYPTO_KEY_MGMT_FT_PSK_SHA384         = 25,
	WLAN_CRYPTO_KEY_MGMT_PSK_SHA384            = 26,
	/** Keep WLAN_CRYPTO_KEY_MGMT_MAX at the end. */
	WLAN_CRYPTO_KEY_MGMT_MAX   = WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384,
} wlan_crypto_key_mgmt;

enum wlan_crypto_key_type {
	WLAN_CRYPTO_KEY_TYPE_UNICAST,
	WLAN_CRYPTO_KEY_TYPE_GROUP,
};

#define IS_WEP_CIPHER(_c)      ((_c == WLAN_CRYPTO_CIPHER_WEP) || \
				(_c == WLAN_CRYPTO_CIPHER_WEP_40) || \
				(_c == WLAN_CRYPTO_CIPHER_WEP_104))

#define DEFAULT_KEYMGMT_6G_MASK 0xFFFFFFFF

/* AKM wlan_crypto_key_mgmt 1, 6, 8, 25 and 26 are not allowed. */
#define ALLOWED_KEYMGMT_6G_MASK 0x01FFFEBD

/*
 * enum fils_erp_cryptosuite: this enum defines the cryptosuites used
 * to calculate auth tag and auth tag length as defined by RFC 6696 5.3.1
 * @HMAC_SHA256_64: sha256 with auth tag len as 64 bits
 * @HMAC_SHA256_128: sha256 with auth tag len as 128 bits
 * @HMAC_SHA256_256: sha256 with auth tag len as 256 bits
 */
enum fils_erp_cryptosuite {
	INVALID_CRYPTO = 0, /* reserved */
	HMAC_SHA256_64,
	HMAC_SHA256_128,
	HMAC_SHA256_256,
};

/**
 * struct mobility_domain_params - structure containing
 *				   mobility domain info
 * @mdie_present: mobility domain present or not
 * @mobility_domain: mobility domain
 */
struct mobility_domain_params {
	uint8_t mdie_present;
	uint16_t mobility_domain;
};

/**
 * struct wlan_crypto_pmksa - structure of crypto to contain pmkid
 * @bssid: bssid for which pmkid is saved
 * @pmkid: pmkid info
 * @pmk: pmk info
 * @pmk_len: pmk len
 * @ssid_len: ssid length
 * @ssid: ssid information
 * @cache_id: cache id
 * @pmk_lifetime: Duration in seconds for which the pmk is valid
 * @pmk_lifetime_threshold: Percentage of pmk liftime within which
 * full authentication is expected to avoid disconnection.
 * @pmk_entry_ts: System timestamp at which the PMK entry was created.
 * @single_pmk_supported: SAE single pmk supported BSS
 * @mdid: structure to contain mobility domain parameters
 */
struct wlan_crypto_pmksa {
	struct qdf_mac_addr bssid;
	uint8_t    pmkid[PMKID_LEN];
	uint8_t    pmk[MAX_PMK_LEN];
	uint8_t    pmk_len;
	uint8_t    ssid_len;
	uint8_t    ssid[WLAN_SSID_MAX_LEN];
	uint8_t    cache_id[WLAN_CACHE_ID_LEN];
	uint32_t   pmk_lifetime;
	uint8_t    pmk_lifetime_threshold;
	qdf_time_t pmk_entry_ts;
#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
	bool       single_pmk_supported;
#endif
	struct mobility_domain_params mdid;
};

/**
 * struct wlan_crypto_params - holds crypto params
 * @authmodeset:        authentication mode
 * @ucastcipherset:     unicast ciphers
 * @mcastcipherset:     multicast cipher
 * @mgmtcipherset:      mgmt cipher
 * @cipher_caps:        cipher capability
 * @key_mgmt:           key mgmt
 * @pmksa:              pmksa
 * @rsn_caps:           rsn_capability
 *
 * This structure holds crypto params for peer or vdev
 */
struct wlan_crypto_params {
	uint32_t authmodeset;
	uint32_t ucastcipherset;
	uint32_t mcastcipherset;
	uint32_t mgmtcipherset;
	uint32_t cipher_caps;
	uint32_t key_mgmt;
	struct   wlan_crypto_pmksa *pmksa[WLAN_CRYPTO_MAX_PMKID];
	uint16_t rsn_caps;
};

typedef enum wlan_crypto_param_type {
	WLAN_CRYPTO_PARAM_AUTH_MODE,
	WLAN_CRYPTO_PARAM_UCAST_CIPHER,
	WLAN_CRYPTO_PARAM_MCAST_CIPHER,
	WLAN_CRYPTO_PARAM_MGMT_CIPHER,
	WLAN_CRYPTO_PARAM_CIPHER_CAP,
	WLAN_CRYPTO_PARAM_RSN_CAP,
	WLAN_CRYPTO_PARAM_KEY_MGMT,
	WLAN_CRYPTO_PARAM_PMKSA,
} wlan_crypto_param_type;

/**
 * struct wlan_crypto_key - key structure
 * @keylen:         length of the key
 * @valid:          is key valid or not
 * @flags:          key flags
 * @keyix:          key id
 * @cipher_type:    cipher type being used for this key
 * @mac_addr:       MAC address of the peer
 * @cipher_table:   table which stores cipher related info
 * @private:        private pointer to save cipher context
 * @keylock:        spin lock
 * @recviv:         WAPI key receive sequence counter
 * @txiv:           WAPI key transmit sequence counter
 * @keytsc:         key transmit sequence counter
 * @keyrsc:         key receive sequence counter
 * @keyrsc_suspect: key receive sequence counter under
 *                  suspect when pN jump is detected
 * @keyglobal:      key receive global sequence counter used with suspect
 * @keyval:         key value buffer
 *
 * This key structure to key related details.
 */
struct wlan_crypto_key {
	uint8_t     keylen;
	bool        valid;
	uint16_t    flags;
	uint16_t    keyix;
	enum wlan_crypto_cipher_type cipher_type;
	uint8_t     macaddr[QDF_MAC_ADDR_SIZE];
	void        *cipher_table;
	void        *private;
	qdf_spinlock_t	keylock;
	uint8_t     recviv[WLAN_CRYPTO_WAPI_IV_SIZE];
	uint8_t     txiv[WLAN_CRYPTO_WAPI_IV_SIZE];
	uint64_t    keytsc;
	uint64_t    keyrsc[WLAN_CRYPTO_TID_SIZE];
	uint64_t    keyrsc_suspect[WLAN_CRYPTO_TID_SIZE];
	uint64_t    keyglobal;
	uint8_t     keyval[WLAN_CRYPTO_KEYBUF_SIZE
				+ WLAN_CRYPTO_MICBUF_SIZE];
#define txmic    (keyval + WLAN_CRYPTO_KEYBUF_SIZE \
				+ WLAN_CRYPTO_TXMIC_OFFSET)
#define rxmic    (keyval + WLAN_CRYPTO_KEYBUF_SIZE \
				+ WLAN_CRYPTO_RXMIC_OFFSET)
};

/**
 * struct wlan_crypto_req_key - key request structure
 * @type:                       key/cipher type
 * @pad:                        padding member
 * @keyix:                      key index
 * @keylen:                     length of the key value
 * @flags:                      key flags
 * @macaddr:                    macaddr of the key
 * @keyrsc:                     key receive sequence counter
 * @keytsc:                     key transmit sequence counter
 * @keydata:                    key value
 * @txiv:                       wapi key tx iv
 * @rxiv:                       wapi key rx iv
 * @filsaad:                    FILS AEAD data
 *
 * Key request structure used for setkey, getkey or delkey
 */
struct wlan_crypto_req_key {
	uint8_t    type;
	uint8_t    pad;
	uint16_t   keyix;
	uint8_t    keylen;
	uint16_t    flags;
	uint8_t    macaddr[QDF_MAC_ADDR_SIZE];
	uint64_t   keyrsc;
	uint64_t   keytsc;
	uint8_t    keydata[WLAN_CRYPTO_KEYBUF_SIZE + WLAN_CRYPTO_MICBUF_SIZE];
	uint8_t    txiv[WLAN_CRYPTO_WAPI_IV_SIZE];
	uint8_t    recviv[WLAN_CRYPTO_WAPI_IV_SIZE];
#ifdef WLAN_CRYPTO_SUPPORT_FILS
	struct     wlan_crypto_fils_aad_key   filsaad;
#endif
};

/**
 * struct wlan_lmac_if_crypto_tx_ops - structure of crypto function
 *                  pointers
 * @allockey: function pointer to alloc key in hw
 * @setkey:  function pointer to setkey in hw
 * @delkey: function pointer to delkey in hw
 * @defaultkey: function pointer to set default key
 * @set_key: converged function pointer to set key in hw
 * @getpn: function pointer to get current pn value of peer
 * @register_events: function pointer to register wmi event handler
 * @deregister_events: function pointer to deregister wmi event handler
 */

struct wlan_lmac_if_crypto_tx_ops {
	QDF_STATUS (*allockey)(struct wlan_objmgr_vdev *vdev,
			       struct wlan_crypto_key *key,
			       uint8_t *macaddr, uint32_t key_type);
	QDF_STATUS (*setkey)(struct wlan_objmgr_vdev *vdev,
			     struct wlan_crypto_key *key,
			     uint8_t *macaddr, uint32_t key_type);
	QDF_STATUS (*delkey)(struct wlan_objmgr_vdev *vdev,
			     struct wlan_crypto_key *key,
			     uint8_t *macaddr, uint32_t key_type);
	QDF_STATUS (*defaultkey)(struct wlan_objmgr_vdev *vdev,
				 uint8_t keyix, uint8_t *macaddr);
	QDF_STATUS (*set_key)(struct wlan_objmgr_vdev *vdev,
			      struct wlan_crypto_key *key,
			      enum wlan_crypto_key_type key_type);
	QDF_STATUS(*getpn)(struct wlan_objmgr_vdev *vdev,
			   uint8_t *macaddr, uint32_t key_type);
	QDF_STATUS (*register_events)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*deregister_events)(struct wlan_objmgr_psoc *psoc);
};

/**
 * struct wlan_lmac_if_crypto_rx_ops - structure of crypto rx  function
 *                  pointers
 * @encap: function pointer to encap tx frame
 * @decap:  function pointer to decap rx frame in hw
 * @enmic: function pointer to enmic tx frame
 * @demic: function pointer to demic rx frame
 */

struct wlan_lmac_if_crypto_rx_ops {
	QDF_STATUS(*crypto_encap)(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf, uint8_t *macaddr,
					uint8_t encapdone);
	QDF_STATUS(*crypto_decap)(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf, uint8_t *macaddr,
					uint8_t tid);
	QDF_STATUS(*crypto_enmic)(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf, uint8_t *macaddr,
					uint8_t encapdone);
	QDF_STATUS(*crypto_demic)(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf, uint8_t *macaddr,
					uint8_t tid, uint8_t keyid);
	QDF_STATUS(*set_peer_wep_keys)(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_peer *peer);
};

#define WLAN_CRYPTO_RX_OPS_ENCAP(crypto_rx_ops) \
				(crypto_rx_ops->crypto_encap)
#define WLAN_CRYPTO_RX_OPS_DECAP(crypto_rx_ops) \
				(crypto_rx_ops->crypto_decap)
#define WLAN_CRYPTO_RX_OPS_ENMIC(crypto_rx_ops) \
				(crypto_rx_ops->crypto_enmic)
#define WLAN_CRYPTO_RX_OPS_DEMIC(crypto_rx_ops) \
				(crypto_rx_ops->crypto_demic)
#define WLAN_CRYPTO_RX_OPS_SET_PEER_WEP_KEYS(crypto_rx_ops) \
				(crypto_rx_ops->set_peer_wep_keys)

#endif /* end of _WLAN_CRYPTO_GLOBAL_DEF_H_ */

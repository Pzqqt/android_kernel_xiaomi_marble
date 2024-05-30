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
 * DOC: Private definations for handling crypto params
 */
#ifndef _WLAN_CRYPTO_DEF_I_H_
#define _WLAN_CRYPTO_DEF_I_H_

#include <wlan_cmn_ieee80211.h>
#ifdef WLAN_CRYPTO_AES
#include "wlan_crypto_aes_i.h"
#endif


/* Number of bits per byte */
#define CRYPTO_NBBY  8

/* Macros for handling unaligned memory accesses */

static inline uint16_t wlan_crypto_get_be16(const uint8_t *a)
{
	return (a[0] << 8) | a[1];
}

static inline void wlan_crypto_put_be16(uint8_t *a, uint16_t val)
{
	a[0] = val >> 8;
	a[1] = val & 0xff;
}

static inline uint16_t wlan_crypto_get_le16(const uint8_t *a)
{
	return (a[1] << 8) | a[0];
}

static inline void wlan_crypto_put_le16(uint8_t *a, uint16_t val)
{
	a[1] = val >> 8;
	a[0] = val & 0xff;
}

static inline uint32_t wlan_crypto_get_be32(const uint8_t *a)
{
	return ((u32) a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

static inline void wlan_crypto_put_be32(uint8_t *a, uint32_t val)
{
	a[0] = (val >> 24) & 0xff;
	a[1] = (val >> 16) & 0xff;
	a[2] = (val >> 8) & 0xff;
	a[3] = val & 0xff;
}

static inline uint32_t wlan_crypto_get_le32(const uint8_t *a)
{
	return ((u32) a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
}

static inline void wlan_crypto_put_le32(uint8_t *a, uint32_t val)
{
	a[3] = (val >> 24) & 0xff;
	a[2] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[0] = val & 0xff;
}

static inline void wlan_crypto_put_be64(u8 *a, u64 val)
{
	a[0] = val >> 56;
	a[1] = val >> 48;
	a[2] = val >> 40;
	a[3] = val >> 32;
	a[4] = val >> 24;
	a[5] = val >> 16;
	a[6] = val >> 8;
	a[7] = val & 0xff;
}

#define WLAN_CRYPTO_TX_OPS_ALLOCKEY(tx_ops) \
	((tx_ops)->crypto_tx_ops.allockey)
#define WLAN_CRYPTO_TX_OPS_SETKEY(tx_ops) \
	((tx_ops)->crypto_tx_ops.setkey)
#define WLAN_CRYPTO_TX_OPS_DELKEY(tx_ops) \
	((tx_ops)->crypto_tx_ops.delkey)
#define WLAN_CRYPTO_TX_OPS_DEFAULTKEY(tx_ops) \
	((tx_ops)->crypto_tx_ops.defaultkey)
#define WLAN_CRYPTO_TX_OPS_SET_KEY(tx_ops) \
	((tx_ops)->crypto_tx_ops.set_key)
#define WLAN_CRYPTO_TX_OPS_GETPN(tx_ops) \
	((tx_ops)->crypto_tx_ops.getpn)
#define WLAN_CRYPTO_TX_OPS_REGISTER_EVENTS(tx_ops) \
	((tx_ops)->crypto_tx_ops.register_events)
#define WLAN_CRYPTO_TX_OPS_DEREGISTER_EVENTS(tx_ops) \
	((tx_ops)->crypto_tx_ops.deregister_events)

/* unalligned little endian access */
#ifndef LE_READ_2
#define LE_READ_2(p) \
	((uint16_t)                          \
	((((const uint8_t *)(p))[0]) |       \
	(((const uint8_t *)(p))[1] <<  8)))
#endif

#ifndef LE_READ_4
#define LE_READ_4(p)                   \
	((uint32_t)                          \
	((((const uint8_t *)(p))[0]) |       \
	(((const uint8_t *)(p))[1] <<  8) |  \
	(((const uint8_t *)(p))[2] << 16) |  \
	(((const uint8_t *)(p))[3] << 24)))
#endif

#ifndef BE_READ_4
#define BE_READ_4(p)                        \
	((uint32_t)                              \
	((((const uint8_t *)(p))[0] << 24) |     \
	(((const uint8_t *)(p))[1] << 16) |      \
	(((const uint8_t *)(p))[2] <<  8) |      \
	(((const uint8_t *)(p))[3])))
#endif

#ifndef READ_6
#define READ_6(b0, b1, b2, b3, b4, b5)  ({ \
	uint32_t iv32 = (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);\
	uint16_t iv16 = (b4 << 0) | (b5 << 8);\
	(((uint64_t)iv16) << 32) | iv32;\
})
#endif

#define OUI_SIZE   (4)
#define WLAN_CRYPTO_ADDSHORT(frm, v)  \
	do {frm[0] = (v) & 0xff; frm[1] = (v) >> 8; frm += 2; } while (0)

#define	WLAN_CRYPTO_ADDSELECTOR(frm, sel) \
	do { \
		uint32_t value = sel;\
		qdf_mem_copy(frm, (uint8_t *)&value, OUI_SIZE); \
	frm += OUI_SIZE; } while (0)

#define WLAN_CRYPTO_SELECTOR(a, b, c, d) \
	((((uint32_t) (a)) << 24) | \
	 (((uint32_t) (b)) << 16) | \
	 (((uint32_t) (c)) << 8) | \
		(uint32_t) (d))

#define WPA_TYPE_OUI                    WLAN_WPA_SEL(WLAN_WPA_OUI_TYPE)

#define WLAN_CRYPTO_WAPI_IE_LEN      20
#define WLAN_CRYPTO_WAPI_SMS4_CIPHER 0x01

#define WPA_AUTH_KEY_MGMT_NONE          WLAN_WPA_SEL(WLAN_ASE_NONE)
#define WPA_AUTH_KEY_MGMT_UNSPEC_802_1X WLAN_WPA_SEL(WLAN_ASE_8021X_UNSPEC)
#define WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X \
					WLAN_WPA_SEL(WLAN_ASE_8021X_PSK)
#define WPA_AUTH_KEY_MGMT_CCKM WLAN_WPA_CCKM_AKM

#define WPA_CIPHER_SUITE_NONE   WLAN_WPA_SEL(WLAN_CSE_NONE)
#define WPA_CIPHER_SUITE_WEP40  WLAN_WPA_SEL(WLAN_CSE_WEP40)
#define WPA_CIPHER_SUITE_WEP104 WLAN_WPA_SEL(WLAN_CSE_WEP104)
#define WPA_CIPHER_SUITE_TKIP   WLAN_WPA_SEL(WLAN_CSE_TKIP)
#define WPA_CIPHER_SUITE_CCMP   WLAN_WPA_SEL(WLAN_CSE_CCMP)

#define RSN_AUTH_KEY_MGMT_NONE          WLAN_RSN_SEL(0)
#define RSN_AUTH_KEY_MGMT_UNSPEC_802_1X WLAN_RSN_SEL(1)
#define RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X\
					WLAN_RSN_SEL(2)
#define RSN_AUTH_KEY_MGMT_FT_802_1X     WLAN_RSN_SEL(3)
#define RSN_AUTH_KEY_MGMT_FT_PSK        WLAN_RSN_SEL(4)
#define RSN_AUTH_KEY_MGMT_802_1X_SHA256\
					WLAN_RSN_SEL(5)
#define RSN_AUTH_KEY_MGMT_PSK_SHA256    WLAN_RSN_SEL(6)
#define RSN_AUTH_KEY_MGMT_WPS           WLAN_RSN_SEL(7)
#define RSN_AUTH_KEY_MGMT_SAE           WLAN_RSN_SEL(8)
#define RSN_AUTH_KEY_MGMT_FT_SAE        WLAN_RSN_SEL(9)
#define RSN_AUTH_KEY_MGMT_802_1X_SUITE_B\
					WLAN_RSN_SEL(11)
#define RSN_AUTH_KEY_MGMT_802_1X_SUITE_B_192\
					WLAN_RSN_SEL(12)
#define RSN_AUTH_KEY_MGMT_FT_802_1X_SUITE_B_384\
					WLAN_RSN_SEL(13)
#define RSN_AUTH_KEY_MGMT_FILS_SHA256   WLAN_RSN_SEL(14)
#define RSN_AUTH_KEY_MGMT_FILS_SHA384   WLAN_RSN_SEL(15)
#define RSN_AUTH_KEY_MGMT_FT_FILS_SHA256\
					WLAN_RSN_SEL(16)
#define RSN_AUTH_KEY_MGMT_FT_FILS_SHA384\
					WLAN_RSN_SEL(17)
#define RSN_AUTH_KEY_MGMT_OWE           WLAN_RSN_SEL(18)
#define RSN_AUTH_KEY_MGMT_FT_PSK_SHA384 WLAN_RSN_SEL(19)
#define RSN_AUTH_KEY_MGMT_PSK_SHA384    WLAN_RSN_SEL(20)

#define RSN_AUTH_KEY_MGMT_CCKM          (WLAN_RSN_CCKM_AKM)
#define RSN_AUTH_KEY_MGMT_OSEN          (0x019a6f50)
#define RSN_AUTH_KEY_MGMT_DPP           (WLAN_RSN_DPP_AKM)

#define RSN_CIPHER_SUITE_NONE           WLAN_RSN_SEL(WLAN_CSE_NONE)
#define RSN_CIPHER_SUITE_WEP40          WLAN_RSN_SEL(WLAN_CSE_WEP40)
#define RSN_CIPHER_SUITE_TKIP           WLAN_RSN_SEL(WLAN_CSE_TKIP)
#define RSN_CIPHER_SUITE_WEP104         WLAN_RSN_SEL(WLAN_CSE_WEP104)
#define RSN_CIPHER_SUITE_CCMP           WLAN_RSN_SEL(WLAN_CSE_CCMP)
#define RSN_CIPHER_SUITE_AES_CMAC       WLAN_RSN_SEL(WLAN_CSE_AES_CMAC)
#define RSN_CIPHER_SUITE_GCMP           WLAN_RSN_SEL(WLAN_CSE_GCMP_128)
#define RSN_CIPHER_SUITE_GCMP_256       WLAN_RSN_SEL(WLAN_CSE_GCMP_256)
#define RSN_CIPHER_SUITE_CCMP_256       WLAN_RSN_SEL(WLAN_CSE_CCMP_256)
#define RSN_CIPHER_SUITE_BIP_GMAC_128   WLAN_RSN_SEL(WLAN_CSE_BIP_GMAC_128)
#define RSN_CIPHER_SUITE_BIP_GMAC_256   WLAN_RSN_SEL(WLAN_CSE_BIP_GMAC_256)
#define RSN_CIPHER_SUITE_BIP_CMAC_256   WLAN_RSN_SEL(WLAN_CSE_BIP_CMAC_256)

#define RESET_PARAM(__param)         ((__param) = 0)
#define SET_PARAM(__param, __val)    ((__param) |= (1 << (__val)))
#define HAS_PARAM(__param, __val)    ((__param) &  (1 << (__val)))
#define CLEAR_PARAM(__param, __val)  ((__param) &= ((~1) << (__val)))


#define RESET_AUTHMODE(_param)       ((_param)->authmodeset = 0)

#define SET_AUTHMODE(_param, _mode)  ((_param)->authmodeset |= (1 << (_mode)))
#define HAS_AUTHMODE(_param, _mode)  ((_param)->authmodeset &  (1 << (_mode)))

#define AUTH_IS_OPEN(_param)   HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_OPEN)
#define AUTH_IS_SHARED_KEY(_param)  \
				HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_SHARED)
#define AUTH_IS_8021X(_param)  HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_8021X)
#define AUTH_IS_WPA(_param)    HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_WPA)
#define AUTH_IS_RSNA(_param)   HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_RSNA)
#define AUTH_IS_CCKM(_param)   HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_CCKM)
#define AUTH_IS_WAI(_param)    HAS_AUTHMODE((_param), WLAN_CRYPTO_AUTH_WAPI)
#define AUTH_IS_WPA2(_param)   AUTH_IS_RSNA(_param)

#define AUTH_MATCH(_param1, _param2) \
		(((_param1)->authmodeset & (_param2)->authmodeset) != 0)


#define RESET_UCAST_CIPHERS(_param)   ((_param)->ucastcipherset = 0)
#define SET_UCAST_CIPHER(_param, _c)  ((_param)->ucastcipherset |= (1 << (_c)))
#define HAS_UCAST_CIPHER(_param, _c)  ((_param)->ucastcipherset & (1 << (_c)))

#define UCIPHER_IS_CLEAR(_param)   \
		((_param)->ucastcipherset == 0)
#define UCIPHER_IS_WEP(_param)     \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WEP)
#define UCIPHER_IS_TKIP(_param)    \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_TKIP)
#define UCIPHER_IS_CCMP128(_param) \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CCM)
#define UCIPHER_IS_CCMP256(_param) \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CCM_256)
#define UCIPHER_IS_GCMP128(_param) \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GCM)
#define UCIPHER_IS_GCMP256(_param) \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GCM_256)
#define UCIPHER_IS_SMS4(_param)    \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WAPI_SMS4)

#define RESET_MCAST_CIPHERS(_param)   ((_param)->mcastcipherset = 0)
#define SET_MCAST_CIPHER(_param, _c)  ((_param)->mcastcipherset |= (1 << (_c)))
#define HAS_MCAST_CIPHER(_param, _c)  ((_param)->mcastcipherset & (1 << (_c)))
#define HAS_ANY_MCAST_CIPHER(_param)  ((_param)->mcastcipherset)
#define CLEAR_MCAST_CIPHER(_param, _c)  \
			((_param)->mcastcipherset &= (~(1)<<(_c)))

#define MCIPHER_IS_CLEAR(_param)   \
		((_param)->mcastcipherset == 0)
#define MCIPHER_IS_WEP(_param)     \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WEP)
#define MCIPHER_IS_TKIP(_param)    \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_TKIP)
#define MCIPHER_IS_CCMP128(_param) \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CCM)
#define MCIPHER_IS_CCMP256(_param) \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CCM_256)
#define MCIPHER_IS_GCMP128(_param) \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GCM)
#define MCIPHER_IS_GCMP256(_param) \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GCM_256)
#define MCIPHER_IS_SMS4(_param)    \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WAPI_SMS4)

#define RESET_MGMT_CIPHERS(_param)   ((_param)->mgmtcipherset = 0)
#define SET_MGMT_CIPHER(_param, _c)  ((_param)->mgmtcipherset |= (1 << (_c)))
#define HAS_MGMT_CIPHER(_param, _c)  ((_param)->mgmtcipherset & (1 << (_c)))
#define IS_MGMT_CIPHER(_c)      ((_c == WLAN_CRYPTO_CIPHER_AES_CMAC) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_CMAC_256) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_GMAC) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_GMAC_256))

#define IS_FILS_CIPHER(_c)      ((_c) == WLAN_CRYPTO_CIPHER_FILS_AEAD)

#define MGMT_CIPHER_IS_CLEAR(_param)   \
		((_param)->mgmtcipherset == 0)
#define MGMT_CIPHER_IS_CMAC(_param)    \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CMAC)
#define MGMT_CIPHER_IS_CMAC256(_param) \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CMAC_256)
#define MGMT_CIPHER_IS_GMAC(_param)    \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GMAC)
#define MGMT_CIPHER_IS_GMAC256(_param) \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GMAC_256)

#define RESET_KEY_MGMT(_param)   ((_param)->key_mgmt = 0)
#define SET_KEY_MGMT(_param, _c)  ((_param)->key_mgmt |= (1 << (_c)))
#define HAS_KEY_MGMT(_param, _c)  ((_param)->key_mgmt & (1 << (_c)))

#define UCAST_CIPHER_MATCH(_param1, _param2)    \
	(((_param1)->ucastcipherset & (_param2)->ucastcipherset) != 0)

#define MCAST_CIPHER_MATCH(_param1, _param2)    \
	(((_param1)->mcastcipherset & (_param2)->mcastcipherset) != 0)

#define MGMT_CIPHER_MATCH(_param1, _param2)    \
	(((_param1)->mgmtcipherset & (_param2)->mgmtcipherset) != 0)

#define KEY_MGMTSET_MATCH(_param1, _param2)      \
	(((_param1)->key_mgmt & (_param2)->key_mgmt) != 0 ||    \
	(!(_param1)->key_mgmt && !(_param2)->key_mgmt))

#define RESET_CIPHER_CAP(_param)   ((_param)->cipher_caps = 0)
#define SET_CIPHER_CAP(_param, _c)  ((_param)->cipher_caps |= (1 << (_c)))
#define HAS_CIPHER_CAP(_param, _c)  ((_param)->cipher_caps & (1 << (_c)))
#define HAS_ANY_CIPHER_CAP(_param)  ((_param)->cipher_caps)

#define crypto_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_CRYPTO, params)
#define crypto_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_CRYPTO, params)
#define crypto_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_CRYPTO, params)

/**
 * struct wlan_crypto_mmie - MMIE IE
 * @element_id:      element id
 * @length:          length of the ie
 * @key_id:          igtk key_id used
 * @sequence_number: igtk PN number
 * @mic:             MIC for the frame
 *
 * This structure represents Management MIC information element (IEEE 802.11w)
 */
struct wlan_crypto_mmie {
	uint8_t  element_id;
	uint8_t  length;
	uint16_t key_id;
	uint8_t  sequence_number[6];
	uint8_t  mic[16];
} __packed;

/**
 * struct crypto_add_key_result - add key result structure
 * @vdev_id: unique id identifying the VDEV
 * @key_ix: key index
 * @key_flags: key flags
 * @status: status of add key
 * @peer_macaddr: MAC address of the peer
 *
 * Structure used for add key result.
 */
struct crypto_add_key_result {
	uint32_t vdev_id;
	uint32_t key_ix;
	uint32_t key_flags;
	uint32_t status;
	uint8_t peer_macaddr[QDF_MAC_ADDR_SIZE];
};

/**
 * typedef crypto_add_key_callback - add key callback
 * @context: opaque context that the client can use to associate the
 *  callback with the request
 * @result: result of add key
 */
typedef void (*crypto_add_key_callback)(void *context,
					struct crypto_add_key_result *result);

/**
 * struct wlan_crypto_comp_priv - crypto component private structure
 * @crypto_params:    crypto params for the peer
 * @key:              key buffers for this peer
 * @igtk_key:         igtk key buffer for this peer
 * @bigtk_key:        bigtk key buffer for this peer
 * @igtk_key_type:    igtk key type
 * @def_tx_keyid:     default key used for this peer
 * @def_igtk_tx_keyid default igtk key used for this peer
 * @def_bigtk_tx_keyid default bigtk key used for this peer
 * @fils_aead_set     fils params for this peer
 * @add_key_ctx: Opaque context to be used by the caller to associate the
 *  add key request with the response
 * @add_key_cb: Callback function to be called with the add key result
 *
 */
struct wlan_crypto_comp_priv {
	struct wlan_crypto_params crypto_params;
	struct wlan_crypto_key *key[WLAN_CRYPTO_MAX_VLANKEYIX];
	struct wlan_crypto_key *igtk_key[WLAN_CRYPTO_MAXIGTKKEYIDX];
	struct wlan_crypto_key *bigtk_key[WLAN_CRYPTO_MAXBIGTKKEYIDX];
	enum wlan_crypto_cipher_type igtk_key_type;
	uint8_t def_tx_keyid;
	uint8_t def_igtk_tx_keyid;
	uint8_t def_bigtk_tx_keyid;
	uint8_t fils_aead_set;
	void *add_key_ctx;
	crypto_add_key_callback add_key_cb;
};

/**
 * struct wlan_crypto_cipher - crypto cipher table
 * @cipher_name: printable name
 * @cipher:      cipher type WLAN_CRYPTO_CIPHER_*
 * @header:      size of privacy header (bytes)
 * @trailer:     size of privacy trailer (bytes)
 * @miclen:      size of mic trailer (bytes)
 * @keylen:      max key length
 * @setkey:      function pointer for setkey
 * @encap:       function pointer for encap
 * @decap:       function pointer for decap
 * @enmic:       function pointer for enmic
 * @demic:       function pointer for demic
 *
 */
struct wlan_crypto_cipher {
	const char *cipher_name;
	wlan_crypto_cipher_type cipher;
	const uint8_t   header;
	const uint8_t   trailer;
	const uint8_t   miclen;
	const uint32_t  keylen;
	QDF_STATUS(*setkey)(struct wlan_crypto_key *);
	QDF_STATUS(*encap)(struct wlan_crypto_key *,
				qdf_nbuf_t, uint8_t,  uint8_t);
	QDF_STATUS(*decap)(struct wlan_crypto_key *,
				qdf_nbuf_t, uint8_t,  uint8_t);
	QDF_STATUS(*enmic)(struct wlan_crypto_key *,
				qdf_nbuf_t, uint8_t,  uint8_t);
	QDF_STATUS(*demic)(struct wlan_crypto_key *,
				qdf_nbuf_t, uint8_t,  uint8_t);
};


/**
 * wlan_crypto_is_data_protected - check is frame is protected or not
 * @data: frame
 *
 * This function check is frame is protected or not
 *
 * Return: TRUE/FALSE
 */
static inline bool wlan_crypto_is_data_protected(const void *data)
{
	const struct wlan_frame_hdr *hdr = (const struct wlan_frame_hdr *)data;
	if (hdr->i_fc[1] & WLAN_FC1_ISWEP)
		return true;
	else
		return false;
}

/**
 * ieee80211_hdrsize - calculate frame header size
 * @data: frame
 *
 * This function calculate frame header size
 *
 * Return: header size of the frame
 */
static inline uint8_t ieee80211_hdrsize(const void *data)
{
	const struct wlan_frame_hdr *hdr = (const struct wlan_frame_hdr *)data;
	uint8_t size = sizeof(struct wlan_frame_hdr);

	if ((hdr->i_fc[1] & WLAN_FC1_DIR_MASK)
				== (WLAN_FC1_DSTODS)) {
		size += QDF_MAC_ADDR_SIZE;
	}

	if (((WLAN_FC0_GET_STYPE(hdr->i_fc[0])
			== WLAN_FC0_STYPE_QOS_DATA))) {
		size += sizeof(uint16_t);
		/* Qos frame with Order bit set indicates an HTC frame */
		if (hdr->i_fc[1] & WLAN_FC1_ORDER)
			size += (sizeof(uint8_t)*4);
	}
	if (((WLAN_FC0_GET_STYPE(hdr->i_fc[0])
			== WLAN_FC0_STYPE_ACTION))) {
		/* Action frame with Order bit set indicates an HTC frame */
		if (hdr->i_fc[1] & WLAN_FC1_ORDER)
			size += (sizeof(uint8_t)*4);
	}
	return size;
}

/**
 * ieee80211_hdrspace - calculate frame header size with padding
 * @pdev: pdev
 * @data: frame header
 *
 * This function returns the space occupied by the 802.11 header
 * and any padding required by the driver. This works for a management
 * or data frame.
 *
 * Return: header size of the frame with padding
 */
static inline uint8_t
ieee80211_hdrspace(struct wlan_objmgr_pdev *pdev, const void *data)
{
	uint8_t size = ieee80211_hdrsize(data);

	if (wlan_pdev_nif_feat_cap_get(pdev, WLAN_PDEV_F_DATAPAD))
		size = roundup(size, sizeof(u_int32_t));

	return size;
}

/**
 * wlan_get_tid - get tid of the frame
 * @data: frame
 *
 * This function get tid of the frame
 *
 * Return: tid of the frame
 */
static inline int wlan_get_tid(const void *data)
{
	const struct wlan_frame_hdr *hdr = (const struct wlan_frame_hdr *)data;

	if (((WLAN_FC0_GET_STYPE(hdr->i_fc[0])
				== WLAN_FC0_STYPE_QOS_DATA))) {
		if ((hdr->i_fc[1] & WLAN_FC1_DIR_MASK)
					== (WLAN_FC1_DSTODS)) {
			return ((struct wlan_frame_hdr_qos_addr4 *)data)->i_qos[0]
							& WLAN_QOS_TID_MASK;
		} else {
			return ((struct wlan_frame_hdr_qos *)data)->i_qos[0]
							& WLAN_QOS_TID_MASK;
		}
	} else
		return WLAN_NONQOS_SEQ;
}
#endif /* end of _WLAN_CRYPTO_DEF_I_H_ */

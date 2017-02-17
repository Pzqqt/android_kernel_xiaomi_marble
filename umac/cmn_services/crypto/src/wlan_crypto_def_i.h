/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#define WLAN_CRYPTO_TX_OPS_ALLOCKEY(psoc) \
		(psoc->soc_cb.tx_ops.crypto_tx_ops.allockey)
#define WLAN_CRYPTO_TX_OPS_SETKEY(psoc) \
		(psoc->soc_cb.tx_ops.crypto_tx_ops.setkey)
#define WLAN_CRYPTO_TX_OPS_DELKEY(psoc) \
		(psoc->soc_cb.tx_ops.crypto_tx_ops.delkey)
#define WLAN_CRYPTO_TX_OPS_DEFAULTKEY(psoc) \
		(psoc->soc_cb.tx_ops.crypto_tx_ops.defaultkey)

/* unalligned little endian access */
#ifndef LE_READ_2
#define LE_READ_2(p)                            \
	((uint16_t)                                \
	((((const uint8_t *)(p))[0]) |       \
	(((const uint8_t *)(p))[1] <<  8)))
#endif

#ifndef LE_READ_4
#define LE_READ_4(p)                            \
	((uint32_t)                                \
	((((const uint8_t *)(p))[0]) |       \
	(((const uint8_t *)(p))[1] <<  8) |        \
	(((const uint8_t *)(p))[2] << 16) |        \
	(((const uint8_t *)(p))[3] << 24)))
#endif

#ifndef BE_READ_4
#define BE_READ_4(p)                            \
	((uint32_t)                                \
	((((const uint8_t *)(p))[0] << 24) |      \
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

#define OUI_SIZE		(4)
#define	WLAN_CRYPTO_ADDSHORT(frm, v)  \
	do {frm[0] = (v) & 0xff; frm[1] = (v) >> 8; frm += 2; } while (0)

#define	WLAN_CRYPTO_ADDSELECTOR(frm, sel) \
	do {qdf_mem_copy(frm, (uint8_t *)sel, OUI_SIZE); \
	frm += OUI_SIZE; } while (0)

#define WLAN_CRYPTO_SELECTOR(a, b, c, d) \
	((((uint32_t) (a)) << 24) | \
	 (((uint32_t) (b)) << 16) | \
	 (((uint32_t) (c)) << 8) | \
		(uint32_t) (d))

#define WPA_TYPE_OUI            WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 1)

#define WPA_AUTH_KEY_MGMT_NONE        \
				WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_AUTH_KEY_MGMT_UNSPEC_802_1X \
				WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X \
				WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_AUTH_KEY_MGMT_CCKM \
				WLAN_CRYPTO_SELECTOR(0x00, 0x40, 0x96, 0)

#define WPA_CIPHER_SUITE_NONE   WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_CIPHER_SUITE_WEP40  WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_CIPHER_SUITE_WEP104 WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 5)
#define WPA_CIPHER_SUITE_TKIP   WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_CCMP   WLAN_CRYPTO_SELECTOR(0x00, 0x50, 0xf2, 4)

#define RSN_AUTH_KEY_MGMT_NONE  WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 0)
#define RSN_AUTH_KEY_MGMT_UNSPEC_802_1X \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 2)
#define RSN_AUTH_KEY_MGMT_FT_802_1X \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 3)
#define RSN_AUTH_KEY_MGMT_FT_PSK \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_AUTH_KEY_MGMT_802_1X_SHA256 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_AUTH_KEY_MGMT_PSK_SHA256 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 6)
#define RSN_AUTH_KEY_MGMT_TPK_HANDSHAKE \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 7)
#define RSN_AUTH_KEY_MGMT_SAE   WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 8)
#define RSN_AUTH_KEY_MGMT_FT_SAE WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 9)
#define RSN_AUTH_KEY_MGMT_802_1X_SUITE_B \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 11)
#define RSN_AUTH_KEY_MGMT_802_1X_SUITE_B_192 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 12)
#define RSN_AUTH_KEY_MGMT_FT_802_1X_SUITE_B_192 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 13)
#define RSN_AUTH_KEY_MGMT_CCKM  WLAN_CRYPTO_SELECTOR(0x00, 0x40, 0x96, 0x00)
#define RSN_AUTH_KEY_MGMT_OSEN  WLAN_CRYPTO_SELECTOR(0x50, 0x6f, 0x9a, 0x01)

#define RSN_CIPHER_SUITE_NONE   WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 0)
#define RSN_CIPHER_SUITE_WEP40  WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_CIPHER_SUITE_TKIP   WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 2)
#define RSN_CIPHER_SUITE_WEP104 WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_CIPHER_SUITE_CCMP   WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_CIPHER_SUITE_AES_CMAC WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 6)
#define RSN_CIPHER_SUITE_GCMP   WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 8)
#define RSN_CIPHER_SUITE_GCMP_256 WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 9)
#define RSN_CIPHER_SUITE_CCMP_256 WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 10)
#define RSN_CIPHER_SUITE_BIP_GMAC_128 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 11)
#define RSN_CIPHER_SUITE_BIP_GMAC_256 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 12)
#define RSN_CIPHER_SUITE_BIP_CMAC_256 \
				WLAN_CRYPTO_SELECTOR(0x00, 0x0f, 0xac, 13)

#define RESET_PARAM(__param)			((__param) = 0)
#define SET_PARAM(__param, __val)		((__param) |= (1<<(__val)))
#define HAS_PARAM(__param, __val)		((__param) &  (1<<(__val)))
#define CLEAR_PARAM(__param, __val)		((__param) &= ((~1) << (__val)))


#define RESET_AUTHMODE(_param)        ((_param)->authmodeset = 0)
#define SET_AUTHMODE(_param, _mode)   ((_param)->authmodeset |= (1<<(_mode)))
#define HAS_AUTHMODE(_param, _mode)   ((_param)->authmodeset &  (1<<(_mode)))

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
#define SET_UCAST_CIPHER(_param, _c)  ((_param)->ucastcipherset |= (1<<(_c)))
#define HAS_UCAST_CIPHER(_param, _c)  ((_param)->ucastcipherset & (1<<(_c)))

#define UCIPHER_IS_CLEAR(_param)   \
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_NONE)
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
		HAS_UCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WAPI)

#define RESET_MCAST_CIPHERS(_param)   ((_param)->mcastcipherset = 0)
#define SET_MCAST_CIPHER(_param, _c)  ((_param)->mcastcipherset |= (1<<(_c)))
#define HAS_MCAST_CIPHER(_param, _c)  ((_param)->mcastcipherset & (1<<(_c)))
#define HAS_ANY_MCAST_CIPHER(_param)  ((_param)->mcastcipherset)
#define CLEAR_MCAST_CIPHER(_param, _c)  \
			((_param)->mcastcipherset &= (~(1)<<(_c)))

#define MCIPHER_IS_CLEAR(_param)   \
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_NONE)
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
		HAS_MCAST_CIPHER((_param), WLAN_CRYPTO_CIPHER_WAPI)

#define RESET_MGMT_CIPHERS(_param)   ((_param)->mgmtcipherset = 0)
#define SET_MGMT_CIPHER(_param, _c)  ((_param)->mgmtcipherset |= (1<<(_c)))
#define HAS_MGMT_CIPHER(_param, _c)  ((_param)->mgmtcipherset & (1<<(_c)))
#define IS_MGMT_CIPHER(_c)      ((_c == WLAN_CRYPTO_CIPHER_AES_CMAC) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_CMAC_256) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_GMAC) || \
				 (_c == WLAN_CRYPTO_CIPHER_AES_GMAC_256))

#define MGMT_CIPHER_IS_CMAC(_param)    \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CMAC)
#define MGMT_CIPHER_IS_CMAC256(_param) \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_CMAC_256)
#define MGMT_CIPHER_IS_GMAC(_param)    \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GMAC)
#define MGMT_CIPHER_IS_GMAC256(_param) \
		HAS_MGMT_CIPHER((_param), WLAN_CRYPTO_CIPHER_AES_GMAC_256)

#define RESET_KEY_MGMT(_param)   ((_param)->key_mgmt = 0)
#define SET_KEY_MGMT(_param, _c)  ((_param)->key_mgmt |= (1<<(_c)))
#define HAS_KEY_MGMT(_param, _c)  ((_param)->key_mgmt & (1<<(_c)))

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
#define SET_CIPHER_CAP(_param, _c)  ((_param)->cipher_caps |= (1<<(_c)))
#define HAS_CIPHER_CAP(_param, _c)  ((_param)->cipher_caps & (1<<(_c)))
#define HAS_ANY_CIPHER_CAP(_param)  ((_param)->cipher_caps)

struct wlan_crypto_comp_priv {
	struct wlan_crypto_params crypto_params;
	struct wlan_crypto_key *key[4];
	struct wlan_crypto_key *igtk_key;
	uint8_t def_tx_keyid;
};


struct wlan_crypto_cipher {
	/* printable name */
	const char *cipher_name;
	/* WLAN_CRYPTO_CIPHER_* */
	wlan_crypto_cipher_type cipher;
	/* size of privacy header (bytes) */
	const uint8_t	header;
	/* size of privacy trailer (bytes) */
	const uint8_t	trailer;
	/* size of mic trailer (bytes) */
	const uint8_t	miclen;
	/* max key length */
	const uint32_t keylen;
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

/*
* Return the size of the 802.11 header for a management or data frame.
*/
static inline int ieee80211_hdrsize(const void *data)
{
	const struct ieee80211_frame *wh = (const struct ieee80211_frame *)data;
	int16_t size = sizeof(struct ieee80211_frame);

	if ((wh->i_fc[1] & 0x03) == 0x03)
		size += 6;

	if ((((wh)->i_fc[0] & (0x0c | 0x80)) == (0x00 | 0x80))) {
		size += sizeof(uint16_t);
		/* Qos frame with Order bit set indicates an HTC frame */
		if (wh->i_fc[1] & 0x80)
			size += (sizeof(uint8_t)*4);
	}
	return size;
}
#endif /* end of _WLAN_CRYPTO_DEF_I_H_ */

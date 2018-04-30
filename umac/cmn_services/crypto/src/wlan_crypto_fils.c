/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Private API for handling FILS related operations
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
#ifdef WLAN_SUPPORT_FILS
#include "wlan_crypto_aes_siv_i.h"
#endif /* WLAN_SUPPORT_FILS */

#define ASSOC_RESP_FIXED_FIELDS_LEN  6 /* cap info + status + assoc id */
#define ASSOC_REQ_FIXED_FIELDS_LEN   4 /* cap info + listen interval */
#define REASSOC_REQ_FIXED_FIELDS_LEN 10 /* cap info + listen interval + BSSID */

#ifdef WLAN_SUPPORT_FILS
/**
 * fils_parse_ie - Parse IEs from (Re-)association Req/Response frames
 * @wbuf: Packet buffer
 * @hdrlen: Header length
 * @cap_info: Pointer to capability Information
 * @fils_sess: Pointer to the end of Fils session Element
 * @ie_start: Pointer to the start of Information element
 *
 * Parse IE  and return required pointers to encrypt/decrypt routines
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
fils_parse_ie(qdf_nbuf_t wbuf, uint8_t hdrlen, uint8_t **cap_info,
	      uint8_t **fils_sess, uint8_t **ie_start)
{
	struct ieee80211_hdr *hdr;
	uint32_t pktlen_left = 0;
	bool fils_found = 0;
	uint8_t subtype = 0;
	uint8_t *frm = NULL;
	uint8_t elem_id;
	uint32_t len;

	frm = (uint8_t *)qdf_nbuf_data(wbuf);
	hdr = (struct ieee80211_hdr *)frm;
	subtype = WLAN_FC0_GET_STYPE(hdr->frame_control[0]);

	pktlen_left = qdf_nbuf_len(wbuf);

	if (pktlen_left < hdrlen) {
		qdf_print(FL("Parse error.pktlen_left:%d Framehdr size:%d\n"),
			  pktlen_left, hdrlen);
		return QDF_STATUS_E_INVAL;
	}

	frm += hdrlen;
	pktlen_left -= hdrlen;

	/* pointer to the capability information field */
	*cap_info = (uint8_t *)frm;

	if (subtype == WLAN_FC0_STYPE_ASSOC_RESP ||
	    subtype == WLAN_FC0_STYPE_REASSOC_RESP) {
		/* assoc resp frame - capability (2), status (2), associd (2) */
		if (pktlen_left < ASSOC_RESP_FIXED_FIELDS_LEN) {
			qdf_print(
			FL("Parse error.pktlen_left:%d Fixed Fields len:%d\n"),
				pktlen_left, ASSOC_RESP_FIXED_FIELDS_LEN);
			return QDF_STATUS_E_INVAL;
		}

		frm += ASSOC_RESP_FIXED_FIELDS_LEN;
		pktlen_left -= ASSOC_RESP_FIXED_FIELDS_LEN;
	} else if (subtype == WLAN_FC0_STYPE_ASSOC_REQ) {
		/* assoc req frame - capability(2), listen interval (2) */
		if (pktlen_left < ASSOC_REQ_FIXED_FIELDS_LEN) {
			qdf_print(
			FL("Parse Error.pktlen_left:%d Fixed Fields len:%d\n"),
				pktlen_left, ASSOC_REQ_FIXED_FIELDS_LEN);
			return QDF_STATUS_E_INVAL;
		}

		frm += ASSOC_REQ_FIXED_FIELDS_LEN;
		pktlen_left -= ASSOC_REQ_FIXED_FIELDS_LEN;
	} else if (subtype == WLAN_FC0_STYPE_REASSOC_REQ) {
		/* assoc req frame - capability(2),
		 * Listen interval(2),
		 * Current AP address(6)
		 */
		if (pktlen_left < REASSOC_REQ_FIXED_FIELDS_LEN) {
			qdf_print(
			FL("Parse Error.pktlen_left:%d Fixed Fields len:%d\n"),
				pktlen_left, REASSOC_REQ_FIXED_FIELDS_LEN);
			return QDF_STATUS_E_INVAL;
		}

		frm += REASSOC_REQ_FIXED_FIELDS_LEN;
		pktlen_left -= REASSOC_REQ_FIXED_FIELDS_LEN;
	}

	*ie_start = frm;
	/* 'frm' now pointing to TLVs.
	 * Parse through All IE's till FILS Session Element
	 */
	while ((pktlen_left >= 2) && frm) {
		/* element ID & len*/
		elem_id = *frm++;
		len = *frm++;
		pktlen_left -= 2;

		/* for extension element, check the sub element ID */
		if (elem_id == WLAN_ELEMID_EXTN_ELEM) {
			if ((len + 1) > pktlen_left) {
				qdf_print(FL("Parse Error.pktlen_left:%did:%d"),
					  pktlen_left, elem_id);
				qdf_print("len:%dextid:%d\n", len, *frm);
				return QDF_STATUS_E_INVAL;
			}

			if (*frm == WLAN_ELEMID_EXT_FILS_SESSION) {
				fils_found = 1;
				break;
			}
			frm++;
			pktlen_left--;
		}

		if (len > pktlen_left) {
			qdf_print(
			FL("Parse Error.pktlen_left:%did:%dlen:%dextid:%d\n"),
				pktlen_left, elem_id, len, *frm);
			return QDF_STATUS_E_INVAL;
		}

		/* switch to the next IE */
		frm += len;
		pktlen_left -= len;
	}

	if (!fils_found) {
		qdf_print(FL("FILS session element not found. Parse failed\n"));
		return QDF_STATUS_E_INVAL;
	}

	/* Points to end of FILS session element */
	*fils_sess = (frm + len);

	return QDF_STATUS_SUCCESS;
}

/**
 * fils_aead_setkey - Setkey function
 * @key: Pointer to wlan_crypto_key
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS fils_aead_setkey(struct wlan_crypto_key *key)
{
	struct wlan_crypto_req_key *req_key;
	struct wlan_crypto_fils_aad_key *fils_key;

	if (!key || !key->private) {
		qdf_print(FL("Failed to set FILS key\n"));
		return QDF_STATUS_E_INVAL;
	}
	req_key = key->private;
	fils_key = qdf_mem_malloc(sizeof(struct wlan_crypto_fils_aad_key));
	if (!fils_key) {
		qdf_print(FL("FILS key alloc failed\n"));
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(fils_key, &req_key->filsaad,
		     sizeof(struct wlan_crypto_fils_aad_key));

	/* Reassign the allocated fils_aad key object */
	key->private = fils_key;

	return QDF_STATUS_SUCCESS;
}

/**
 * fils_aead_encap - FILS AEAD encryption function
 * @key: Pointer to wlan_crypto_key
 * @wbuf: Packet buffer
 * @keyid: Encrypting key ID
 * @hdrlen: Header length
 *
 * This function encrypts FILS Association Response Packet
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
fils_aead_encap(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
		uint8_t keyid, uint8_t hdrlen)
{
	const uint8_t *address[5 + 1];
	size_t length[5 + 1];
	uint8_t *cap_info = NULL, *fils_session = NULL, *ie_start = NULL;
	uint32_t crypt_len = 0;
	struct ieee80211_hdr *hdr = NULL;
	struct wlan_crypto_fils_aad_key *fils_key = NULL;
	uint8_t *buf = NULL;
	uint32_t bufsize = 0;
	uint8_t subtype = 0;

	if (!key) {
		qdf_print(FL("Invalid Input\n"));
		return QDF_STATUS_E_FAILURE;
	}

	fils_key = (struct wlan_crypto_fils_aad_key *)key->private;
	if (!fils_key) {
		qdf_print(FL("Key is not set\n"));
		return QDF_STATUS_E_FAILURE;
	}

	if (!fils_key->kek_len) {
		qdf_print(FL("Key len is zero. Returning error\n"));
		return QDF_STATUS_E_FAILURE;
	}

	hdr = (struct ieee80211_hdr *)qdf_nbuf_data(wbuf);
	if (!hdr) {
		qdf_print(FL("Invalid header\n"));
		return QDF_STATUS_E_FAILURE;
	}

	subtype = WLAN_FC0_GET_STYPE(hdr->frame_control[0]);
	if ((subtype != WLAN_FC0_STYPE_ASSOC_RESP) &&
	    (subtype != WLAN_FC0_STYPE_REASSOC_RESP))
		return QDF_STATUS_E_FAILURE;

	if (fils_parse_ie(wbuf, hdrlen, &cap_info, &fils_session, &ie_start)
			!= QDF_STATUS_SUCCESS) {
		qdf_print(FL("FILS Parsing failed\n"));
		return QDF_STATUS_E_FAILURE;
	}

	/* The AP's BSSID */
	address[0] = hdr->addr2;
	length[0] = WLAN_ALEN;
	/* The STA's MAC address */
	address[1] = hdr->addr1;
	length[1] = WLAN_ALEN;
	/* The AP's nonce */
	address[2] = fils_key->a_nonce;
	length[2] = WLAN_FILS_NONCE_LEN;
	/* The STA's nonce */
	address[3] = fils_key->s_nonce;
	length[3] = WLAN_FILS_NONCE_LEN;
	address[4] = cap_info;
	length[4] = fils_session - cap_info;

	crypt_len = (uint8_t *)hdr + (uint32_t)qdf_nbuf_len(wbuf)
					- fils_session;

	bufsize = ((uint8_t *)hdr + (uint32_t)qdf_nbuf_len(wbuf) - ie_start)
					+ AES_BLOCK_SIZE;
	buf = qdf_mem_malloc(bufsize);
	if (!buf) {
		qdf_print(FL("temp buf allocation failed\n"));
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(buf, ie_start, bufsize);

	if (wlan_crypto_aes_siv_encrypt(fils_key->kek, fils_key->kek_len,
					fils_session, crypt_len, 5, address,
					length, buf + (fils_session - ie_start))
					< 0) {
		qdf_print(FL("aes siv_encryption failed\n"));
		qdf_mem_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	if (!qdf_nbuf_put_tail(wbuf, AES_BLOCK_SIZE))
		qdf_print(FL("Unable to put data in nbuf\n"));

	qdf_mem_copy(ie_start, buf, bufsize);
	qdf_mem_free(buf);

	return QDF_STATUS_SUCCESS;
}

/**
 * fils_aead_decap - FILS AEAD decryption function
 * @key: Pointer to wlan_crypto_key
 * @wbuf: Packet buffer
 * @tid: TID
 * @hdrlen: Header length
 *
 * This function decrypts FILS Association Request Packet
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
fils_aead_decap(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
		uint8_t tid, uint8_t hdrlen)
{
	const uint8_t *address[5];
	size_t length[5];
	uint8_t *cap_info = NULL, *fils_session = NULL, *ie_start = NULL;
	struct ieee80211_hdr *hdr = NULL;
	struct wlan_crypto_fils_aad_key *fils_key = NULL;
	uint32_t crypt_len = 0;
	uint8_t *buf = NULL;
	uint32_t bufsize = 0;

	if (!key) {
		qdf_print(FL("Invalid Input\n"));
		return QDF_STATUS_E_FAILURE;
	}

	fils_key = (struct wlan_crypto_fils_aad_key *)key->private;
	if (!fils_key) {
		qdf_print(FL("Key is not set\n"));
		return QDF_STATUS_E_FAILURE;
	}

	if (!fils_key->kek_len) {
		qdf_print(FL("Key len is zero. Returning error\n"));
		return QDF_STATUS_E_FAILURE;
	}

	if (fils_parse_ie(wbuf, hdrlen, &cap_info, &fils_session, &ie_start)
			!= QDF_STATUS_SUCCESS) {
		qdf_print(FL("IE parse failed\n"));
		return QDF_STATUS_E_FAILURE;
	}

	hdr = (struct ieee80211_hdr *)qdf_nbuf_data(wbuf);
	if (!hdr) {
		qdf_print(FL("Invalid header\n"));
		return QDF_STATUS_E_FAILURE;
	}

	/* The STA's MAC address */
	address[0] = hdr->addr1;
	length[0] = WLAN_ALEN;
	/* The AP's BSSID */
	address[1] = hdr->addr2;
	length[1] = WLAN_ALEN;
	/* The STA's nonce */
	address[2] = fils_key->s_nonce;
	length[2] = WLAN_FILS_NONCE_LEN;
	/* The AP's nonce */
	address[3] = fils_key->a_nonce;
	length[3] = WLAN_FILS_NONCE_LEN;

	address[4] = cap_info;
	length[4] = fils_session - cap_info;

	crypt_len = ((uint8_t *)hdr + (uint32_t)qdf_nbuf_len(wbuf))
				- fils_session;
	if (crypt_len < AES_BLOCK_SIZE) {
		qdf_print(FL(
			"Not enough room for AES-SIV data after FILS Session"));
		qdf_print(
		" element in (Re)Association Request frame from %pM\n",
						hdr->addr1);
		return QDF_STATUS_E_INVAL;
	}

	/* Allocate temp buf & copy contents */
	bufsize = (uint8_t *)hdr + (uint32_t)qdf_nbuf_len(wbuf) - ie_start;
	buf = qdf_mem_malloc(bufsize);
	if (!buf) {
		qdf_print(FL("temp buf allocation failed\n"));
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(buf, ie_start, bufsize);

	if (wlan_crypto_aes_siv_decrypt(fils_key->kek, fils_key->kek_len,
					fils_session, crypt_len, 5, address,
					length, buf + (fils_session - ie_start))
					< 0) {
		qdf_print(FL("AES decrypt of assocreq frame from %s failed\n"),
			  ether_sprintf(hdr->addr1));
		qdf_mem_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(ie_start, buf, bufsize);
	qdf_nbuf_trim_tail(wbuf, AES_BLOCK_SIZE);
	qdf_mem_free(buf);

	return QDF_STATUS_SUCCESS;
}

void wlan_crypto_fils_delkey(struct wlan_objmgr_peer *peer)
{
	struct wlan_crypto_comp_priv *crypto_priv = NULL;
	struct wlan_crypto_key *key = NULL;

	if (!peer) {
		qdf_print(FL("Invalid Input\n"));
		return;
	}

	crypto_priv = wlan_get_peer_crypto_obj(peer);
	if (!crypto_priv) {
		qdf_print(FL("crypto_priv NULL\n"));
		return;
	}

	key = crypto_priv->key[0];
	if (key) {
		qdf_mem_free(key->private);
		key->private = NULL;
	}
}
#else

static QDF_STATUS fils_aead_setkey(struct wlan_crypto_key *key)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
fils_aead_encap(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
		uint8_t keyid, uint8_t hdrlen)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
fils_aead_decap(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
		uint8_t tid, uint8_t hdrlen)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_FILS */

static const struct wlan_crypto_cipher fils_aead_cipher_table = {
	"FILS AEAD",
	WLAN_CRYPTO_CIPHER_FILS_AEAD,
	0,
	0,
	0,
	WLAN_MAX_WPA_KEK_LEN,
	fils_aead_setkey,
	fils_aead_encap,
	fils_aead_decap,
	0,
	0,
};

const struct wlan_crypto_cipher *fils_register(void)
{
	return &fils_aead_cipher_table;
}


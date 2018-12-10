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
#include "wlan_crypto_def_i.h"
#include "wlan_crypto_main_i.h"
#include "wlan_crypto_obj_mgr_i.h"


static QDF_STATUS wep_setkey(struct wlan_crypto_key *key)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wep_encap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen)
{
	uint8_t *ivp;
	struct wlan_crypto_cipher *cipher_table;

	cipher_table = key->cipher_table;
	/*
	 * Copy down 802.11 header and add the IV, KeyID, and ExtIV.
	 */

	if (encapdone) {
		ivp = (uint8_t *)qdf_nbuf_data(wbuf);
	} else {
		ivp = (uint8_t *)qdf_nbuf_push_head(wbuf,
						cipher_table->header
						+ cipher_table->trailer);
		qdf_mem_move(ivp,
			ivp + cipher_table->header + cipher_table->trailer,
			hdrlen);
		qdf_mem_move(ivp + hdrlen + cipher_table->header,
			ivp + hdrlen
			+ cipher_table->header + cipher_table->trailer,
			(qdf_nbuf_len(wbuf) - hdrlen
			- cipher_table->header - cipher_table->trailer));
		ivp = (uint8_t *)qdf_nbuf_data(wbuf);
	}

	ivp += hdrlen;
	key->keytsc++;
#if _BYTE_ORDER == _BIG_ENDIAN
	ivp[2] = key->keyrsc[0] >> 0;
	ivp[1] = key->keyrsc[0] >> 8;
	ivp[0] = key->keyrsc[0] >> 16;
#else
	ivp[0] = key->keyrsc[0] >> 0;
	ivp[1] = key->keyrsc[0] >> 8;
	ivp[2] = key->keyrsc[0] >> 16;
#endif
	ivp[3] = key->keyix << 6;

	/*
	 * Finally, do software encrypt if neeed.
	 */
	if ((key->flags & WLAN_CRYPTO_KEY_SWENCRYPT) &&
		!wlan_crypto_wep_encrypt(key->keyval, key->keylen,
				qdf_nbuf_data(wbuf), qdf_nbuf_len(wbuf))) {
		return QDF_STATUS_CRYPTO_ENCRYPT_FAILED;
	}

	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wep_decap(struct wlan_crypto_key *key,
					qdf_nbuf_t wbuf,
					uint8_t tid,
					uint8_t hdrlen)
{
	struct wlan_crypto_cipher *cipher_table;
	uint8_t *origHdr = (uint8_t *)qdf_nbuf_data(wbuf);
	uint16_t off, data_len;

	cipher_table = key->cipher_table;

	/*
	 * Check if the device handled the decrypt in hardware.
	 * If so we just strip the header; otherwise we need to
	 * handle the decrypt in software.
	 */

	off = hdrlen + cipher_table->header;
	data_len = qdf_nbuf_len(wbuf) - off - cipher_table->trailer;
	if ((key->flags & WLAN_CRYPTO_KEY_SWDECRYPT) &&
		!wlan_crypto_wep_decrypt(key->keyval, key->keylen,
				qdf_nbuf_data(wbuf), qdf_nbuf_len(wbuf))) {
		return QDF_STATUS_CRYPTO_DECRYPT_FAILED;
	}
	/*
	 * Copy up 802.11 header and strip crypto bits.
	 */
	qdf_mem_move(origHdr + cipher_table->header, origHdr, hdrlen);
	qdf_nbuf_pull_head(wbuf, cipher_table->header);
	qdf_nbuf_trim_tail(wbuf, cipher_table->trailer);

	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wep_enmic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){

	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wep_demic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){

	return QDF_STATUS_SUCCESS;
}

const struct wlan_crypto_cipher wep_cipher_table = {
	"WEP",
	WLAN_CRYPTO_CIPHER_WEP,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN,
	WLAN_CRYPTO_CRC_LEN,
	0,
	152,
	wep_setkey,
	wep_encap,
	wep_decap,
	wep_enmic,
	wep_demic,
};

const struct wlan_crypto_cipher *wep_register(void)
{
	return &wep_cipher_table;
}


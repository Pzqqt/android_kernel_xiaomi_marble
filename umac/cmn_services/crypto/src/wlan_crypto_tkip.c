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
#include "wlan_crypto_tkip_i.h"


static QDF_STATUS tkip_enmic(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
				uint8_t encapdone, uint8_t hdrlen);
static QDF_STATUS tkip_demic(struct wlan_crypto_key *key, qdf_nbuf_t wbuf,
				uint8_t tid, uint8_t hdrlen);


static QDF_STATUS tkip_setkey(struct wlan_crypto_key *key)
{
	struct wlan_crypto_tkip_ctx *ctx;

	if (key->private == NULL) {
		key->private = qdf_mem_malloc(
					sizeof(struct wlan_crypto_tkip_ctx));
		if (key->private == NULL)
			return QDF_STATUS_E_NOMEM;
	}

	ctx = key->private;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tkip_encap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen){
	uint8_t *ivp;
	struct ieee80211_frame *wh;
	qdf_nbuf_t wbuf0;
	uint16_t pktlen;
	struct wlan_crypto_cipher *cipher_table;

	cipher_table = key->cipher_table;
	wh = (struct ieee80211_frame *)qdf_nbuf_data(wbuf);

	/*
	 * Copy down 802.11 header and add the IV, KeyID, and ExtIV.
	 */
	if (encapdone) {
		ivp = (uint8_t *)qdf_nbuf_data(wbuf);
	} else {
		ivp = (uint8_t *)qdf_nbuf_push_head(wbuf, cipher_table->header);
		qdf_mem_move(ivp, (ivp + cipher_table->header), hdrlen);
		wh = (struct ieee80211_frame *) qdf_nbuf_data(wbuf);
	}

	ivp += hdrlen;
	key->keytsc++;         /* XXX wrap at 48 bits */

	ivp[0] = key->keytsc >> 8;            /* TSC1 */
	ivp[1] = (ivp[0] | 0x20) & 0x7f;      /* WEP seed */
	ivp[2] = key->keytsc >> 0;            /* TSC0*/
	ivp[3] = key->keyix | WLAN_CRYPTO_EXT_IV_BIT; /* KeyID | ExtID */
	ivp[4] = key->keytsc >> 16;           /* PN2 */
	ivp[5] = key->keytsc >> 24;           /* PN3 */
	ivp[6] = key->keytsc >> 32;           /* PN4 */
	ivp[7] = key->keytsc >> 40;           /* PN5 */

	wbuf0 = wbuf;
	pktlen = qdf_nbuf_len(wbuf);
	while (qdf_nbuf_queue_next(wbuf0) != NULL) {
		wbuf = qdf_nbuf_queue_next(wbuf0);
		pktlen += qdf_nbuf_len(wbuf0);
	}

	/*
	 * Finally, do software encrypt if neeed.
	 */
	if (key->flags & WLAN_CRYPTO_KEY_SWENCRYPT) {
		/*if not frag frame then do mic calculation */
		if (tkip_enmic(key, wbuf, encapdone, hdrlen)
						!= QDF_STATUS_SUCCESS) {
			return QDF_STATUS_CRYPTO_ENCRYPT_FAILED;
		}
		if (!wlan_crypto_tkip_encrypt(key, wbuf, hdrlen))
			return QDF_STATUS_CRYPTO_ENCRYPT_FAILED;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tkip_decap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){
	struct ieee80211_frame *wh;
	uint8_t *ivp, *origHdr;
	uint64_t pn;
	struct wlan_crypto_cipher *cipher_table;

	cipher_table = key->cipher_table;

	/*
	 * Header should have extended IV and sequence number;
	 * verify the former and validate the latter.
	 */
	origHdr = (uint8_t *)qdf_nbuf_data(wbuf);
	wh = (struct ieee80211_frame *)origHdr;

	ivp = origHdr + hdrlen;

	if ((ivp[WLAN_CRYPTO_IV_LEN] & WLAN_CRYPTO_EXT_IV_BIT) == 0)
		return 0;

	tid = 16; /* non QoS*/
	if ((((wh)->i_fc[0] & (0x0c | 0x80)) == (0x00 | 0x80))) {
		if ((wh->i_fc[1] & 0x03) == 0x03) {
			tid = ((struct ieee80211_qosframe_addr4 *)wh)->i_qos[0]
									& 0x0f;
		} else {
			tid = ((struct ieee80211_qosframe *)wh)->i_qos[0]
									& 0x0f;
		}
	}

	/* NB: assume IEEEE80211_WEP_MINLEN covers the extended IV */
	pn = READ_6(ivp[0], ivp[1], ivp[4], ivp[5], ivp[6], ivp[7]);

	if (pn <= key->keyrsc[tid]) {
		/* Replay violation.*/
		return 0;
	}

	if ((key->flags & WLAN_CRYPTO_KEY_SWDECRYPT)) {
		if (!wlan_crypto_tkip_decrypt(key, wbuf, hdrlen) == 0)
			return QDF_STATUS_CRYPTO_DECRYPT_FAILED;
	}
	/* PN will be updated in tkip_demic*/

	/*
	 * Copy up 802.11 header and strip crypto bits.
	 */
	qdf_mem_move(origHdr + cipher_table->header, origHdr, hdrlen);

	qdf_nbuf_pull_head(wbuf, cipher_table->header);
	while (qdf_nbuf_queue_next(wbuf) != NULL)
		wbuf = qdf_nbuf_queue_next(wbuf);
	qdf_nbuf_trim_tail(wbuf, cipher_table->trailer);

	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS tkip_enmic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS tkip_demic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}

const struct wlan_crypto_cipher tkip_cipher_table = {
	"TKIP",
	WLAN_CRYPTO_CIPHER_TKIP,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN,
	WLAN_CRYPTO_CRC_LEN,
	WLAN_CRYPTO_MIC_LEN,
	256,
	tkip_setkey,
	tkip_encap,
	tkip_decap,
	tkip_enmic,
	tkip_demic,
};

const struct wlan_crypto_cipher *tkip_register(void){
	return &tkip_cipher_table;
}

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
 * DOC: Private API for handling CCMP related operations
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

#define MAX_CCMP_PN_GAP_ERR_CHECK 0

static QDF_STATUS ccmp_setkey(struct wlan_crypto_key *key)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ccmp_encap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen){
	uint8_t *ivp;
	struct ieee80211_hdr *hdr;
	struct wlan_crypto_cipher *cipher_table;

	cipher_table = (struct wlan_crypto_cipher *)key->cipher_table;

	hdr = (struct ieee80211_hdr *)qdf_nbuf_data(wbuf);

	/*
	 * Copy down 802.11 header and add the IV, KeyID, and ExtIV.
	 */
	if (encapdone) {
		ivp = (uint8_t *)qdf_nbuf_data(wbuf);
	} else {
		uint8_t ivmic_len = cipher_table->header + cipher_table->miclen;
		ivp = (uint8_t *)qdf_nbuf_push_head(wbuf, ivmic_len);
		qdf_mem_move(ivp, ivp + ivmic_len, hdrlen);

		qdf_mem_move(ivp + hdrlen + cipher_table->header,
			ivp + hdrlen + ivmic_len,
			(qdf_nbuf_len(wbuf) - hdrlen - ivmic_len));

		ivp = (uint8_t *) qdf_nbuf_data(wbuf);
	}

	ivp += hdrlen;
	/* XXX wrap at 48 bits */
	key->keytsc++;

	ivp[0] = key->keytsc >> 0;                         /* PN0 */
	ivp[1] = key->keytsc >> 8;                         /* PN1 */
	ivp[2] = 0;                                        /* Reserved */
	ivp[3] = (key->keyix << 6)| WLAN_CRYPTO_EXT_IV_BIT;/* KeyID | ExtID */
	ivp[4] = key->keytsc >> 16;                        /* PN2 */
	ivp[5] = key->keytsc >> 24;                        /* PN3 */
	ivp[6] = key->keytsc >> 32;                        /* PN4 */
	ivp[7] = key->keytsc >> 40;                        /* PN5 */

	/*
	 * Finally, do software encrypt if neeed.
	 */
	if (key->flags & WLAN_CRYPTO_KEY_SWENCRYPT) {
		if (!wlan_crypto_ccmp_encrypt(key->keyval,
						qdf_nbuf_data(wbuf),
						qdf_nbuf_len(wbuf), hdrlen)) {
			return QDF_STATUS_CRYPTO_ENCRYPT_FAILED;
		}
	}

	return QDF_STATUS_SUCCESS;
}

#define WLAN_CRYPTO_CCMP_PN_MAX(pn) (pn + MAX_CCMP_PN_GAP_ERR_CHECK)

static QDF_STATUS ccmp_decap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){
	struct ieee80211_hdr *hdr;
	uint8_t *ivp, *origHdr;
	uint64_t pn;
	uint8_t  update_keyrsc = 1;
	struct wlan_crypto_cipher *cipher_table;

	cipher_table = (struct wlan_crypto_cipher *)key->cipher_table;

	/*
	 * Header should have extended IV and sequence number;
	 * verify the former and validate the latter.
	 */
	origHdr = (uint8_t *)qdf_nbuf_data(wbuf);
	hdr = (struct ieee80211_hdr *)origHdr;

	ivp = origHdr + hdrlen;

	if ((ivp[WLAN_CRYPTO_IV_LEN] & WLAN_CRYPTO_EXT_IV_BIT) == 0) {
		/*invalid CCMP iv*/
		return QDF_STATUS_E_INVAL;
	}

	tid = wlan_get_tid(qdf_nbuf_data(wbuf));

	pn = READ_6(ivp[0], ivp[1], ivp[4], ivp[5], ivp[6], ivp[7]);

	if (pn <= key->keyrsc[tid]) {
		/*
		 * Replay violation.
		 */
		return QDF_STATUS_CRYPTO_PN_ERROR;
	}

	if ((key->flags & WLAN_CRYPTO_KEY_SWDECRYPT)) {
		if (!wlan_crypto_ccmp_decrypt(key->keyval,
				(struct ieee80211_hdr *)origHdr,
				(origHdr + hdrlen),
				(qdf_nbuf_len(wbuf) - hdrlen))) {
			return QDF_STATUS_CRYPTO_DECRYPT_FAILED;
		}
	}

	/* we can get corrupted frame that has a bad PN.
	 * The PN upper bits tend to get corrupted.
	 * The PN should be a monotically increasing counter.
	 * if we detected a big jump, then we will throw away this frame.
	 */
	if ((key->keyrsc[tid] > 1) &&
		(pn > (WLAN_CRYPTO_CCMP_PN_MAX(key->keyrsc[tid])))) {
		/* PN jump wrt keyrsc is > MAX_CCMP_PN_GAP_ERR_CHECK -
		 * PN of current frame is suspected
		 */
		if (key->keyrsc_suspect[tid]) {
			/* Check whether PN of the current frame
			 * is following prev PN seq or not
			 */
			if (pn <  key->keyrsc_suspect[tid]) {
				/* PN number of the curr frame < PN no of prev
				 * rxed frame. As we are not sure about prev
				 * suspect PN, to detect replay, check the
				 * current PN with global PN
				 */
				if (pn < key->keyglobal)
					/* Replay violation */
					return QDF_STATUS_CRYPTO_PN_ERROR;
				else {
					/* Current PN is following global PN,
					 * so mark this as suspected PN
					 * Don't update keyrsc & keyglobal
					 */
					key->keyrsc_suspect[tid] = pn;
					update_keyrsc = 0;
				}
			} else if (pn <
			(WLAN_CRYPTO_CCMP_PN_MAX(key->keyrsc_suspect[tid]))) {
				/* Current PN is following prev suspected
				 * PN seq Update keyrsc & keyglobal
				 * (update_keyrsc = 1;)
				 */
			} else {
				/* Current PN is neither following prev
				 * suspected PN nor prev Keyrsc.
				 * Mark this as new suspect and
				 * don't update keyrsc & keyglobal
				 */
				key->keyrsc_suspect[tid] = pn;
				update_keyrsc = 0;
			}
		} else {
			/* New Jump in PN observed
			 * So mark this PN as suspected and
			 * don't update keyrsc/keyglobal */
			key->keyrsc_suspect[tid] = pn;
			update_keyrsc = 0;
		}
	} else {
		/* Valid PN, update keyrsc & keyglobal (update_keyrsc = 1;) */
	}

	/*
	 * Copy up 802.11 header and strip crypto bits.
	 */
	if (!(key->flags & WLAN_CRYPTO_KEY_SWDECRYPT)) {
		qdf_mem_move(origHdr + cipher_table->header, origHdr, hdrlen);
		qdf_nbuf_pull_head(wbuf, cipher_table->header);
		qdf_nbuf_trim_tail(wbuf, cipher_table->trailer
						+ cipher_table->miclen);
	} else {
		qdf_nbuf_trim_tail(wbuf, cipher_table->header
					+ cipher_table->miclen);
	}

	if (update_keyrsc) {
		/*
		 * Ok to update rsc now.
		 */
		key->keyrsc[tid] = pn;
		key->keyglobal = pn;
		key->keyrsc_suspect[tid] = 0;
	}
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS ccmp_enmic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t keyid,
				uint8_t hdrlen){

	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS ccmp_demic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t keyid,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}


const struct wlan_crypto_cipher ccmp_cipher_table = {
	"AES-CCM",
	WLAN_CRYPTO_CIPHER_AES_CCM,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN + WLAN_CRYPTO_EXT_IV_LEN,
	0,
	WLAN_CRYPTO_MIC_LEN,
	128,
	ccmp_setkey,
	ccmp_encap,
	ccmp_decap,
	ccmp_enmic,
	ccmp_demic,
};


const struct wlan_crypto_cipher ccmp256_cipher_table = {
	"AES-CCM256",
	WLAN_CRYPTO_CIPHER_AES_CCM_256,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN + WLAN_CRYPTO_EXT_IV_LEN,
	0,
	WLAN_CRYPTO_MIC256_LEN,
	256,
	ccmp_setkey,
	ccmp_encap,
	ccmp_decap,
	ccmp_enmic,
	ccmp_demic,
};


const struct wlan_crypto_cipher gcmp_cipher_table = {
	"AES-GCM",
	WLAN_CRYPTO_CIPHER_AES_GCM,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN + WLAN_CRYPTO_EXT_IV_LEN,
	0,
	WLAN_CRYPTO_MIC_LEN,
	128,
	ccmp_setkey,
	ccmp_encap,
	ccmp_decap,
	ccmp_enmic,
	ccmp_demic,
};


const struct wlan_crypto_cipher gcmp256_cipher_table = {
	"AES-GCM256",
	WLAN_CRYPTO_CIPHER_AES_GCM_256,
	WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN + WLAN_CRYPTO_EXT_IV_LEN,
	0,
	WLAN_CRYPTO_MIC256_LEN,
	256,
	ccmp_setkey,
	ccmp_encap,
	ccmp_decap,
	ccmp_enmic,
	ccmp_demic,
};

const struct wlan_crypto_cipher *ccmp_register(void)
{
	return &ccmp_cipher_table;
}

const struct wlan_crypto_cipher *ccmp256_register(void)
{
	return &ccmp256_cipher_table;
}

const struct wlan_crypto_cipher *gcmp_register(void)
{
	return &gcmp_cipher_table;
}

const struct wlan_crypto_cipher *gcmp256_register(void)
{
	return &gcmp256_cipher_table;
}

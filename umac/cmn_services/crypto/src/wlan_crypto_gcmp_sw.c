/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 */
/*
 * GCM with GMAC Protocol (GCMP)
 * Copyright (c) 2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include "wlan_crypto_aes_i.h"
#include "wlan_crypto_def_i.h"

static void gcmp_aad_nonce(const struct ieee80211_hdr *hdr, const uint8_t *data,
			   uint8_t *aad, size_t *aad_len, uint8_t *nonce)
{
	uint16_t seq;
	uint8_t stype;
	int qos = 0, addr4 = 0;
	uint8_t *pos;

	stype = WLAN_FC0_GET_STYPE(hdr->frame_control[0]);
	if ((hdr->frame_control[1] & WLAN_FC1_DIR_MASK) ==
	    (WLAN_FC1_DSTODS))
		addr4 = 1;

	if (WLAN_FC0_GET_TYPE(hdr->frame_control[0]) == WLAN_FC0_TYPE_DATA) {
		aad[0] &= ~0x0070; /* Mask subtype bits */
		if (stype & 0x08) {
			const uint8_t *qc;
			qos = 1;
			aad[1] &= ~WLAN_FC1_ORDER;
			qc = (const uint8_t *) (hdr + 1);
			if (addr4)
				qc += WLAN_ALEN;
		}
	}

	aad[1] &= ~(WLAN_FC1_RETRY | WLAN_FC1_PWRMGT | WLAN_FC1_MOREDATA);
	pos = aad + 2;
	qdf_mem_copy(pos, hdr->addr1, 3 * WLAN_ALEN);
	pos += 3 * WLAN_ALEN;
	seq = qdf_le16_to_cpu(*((uint16_t *)&hdr->seq_ctrl[0]));
	seq &= ~0xfff0; /* Mask Seq#; do not modify Frag# */
	wlan_crypto_put_le16(pos, seq);
	pos += 2;

	qdf_mem_copy(pos, hdr + 1, addr4 * WLAN_ALEN + qos * 2);
	pos += addr4 * WLAN_ALEN;
	if (qos) {
		pos[0] &= ~0x70;
		if (1 /* FIX: either device has SPP A-MSDU Capab = 0 */)
			pos[0] &= ~0x80;
		pos++;
		*pos++ = 0x00;
	}

	*aad_len = pos - aad;

	qdf_mem_copy(nonce, hdr->addr2, WLAN_ALEN);
	nonce[6] = data[7]; /* PN5 */
	nonce[7] = data[6]; /* PN4 */
	nonce[8] = data[5]; /* PN3 */
	nonce[9] = data[4]; /* PN2 */
	nonce[10] = data[1]; /* PN1 */
	nonce[11] = data[0]; /* PN0 */
}


uint8_t *wlan_crypto_gcmp_decrypt(const uint8_t *tk, size_t tk_len,
					const struct ieee80211_hdr *hdr,
					const uint8_t *data, size_t data_len,
					size_t *decrypted_len){
	uint8_t aad[30], nonce[12], *plain;
	size_t aad_len, mlen;
	const uint8_t *m;

	if (data_len < 8 + 16)
		return NULL;

	plain = qdf_mem_malloc(data_len + AES_BLOCK_SIZE);
	if (plain == NULL) {
		qdf_print("%s[%d] mem alloc failed\n", __func__, __LINE__);
		return NULL;
	}

	m = data + 8;
	mlen = data_len - 8 - 16;

	qdf_mem_set(aad, sizeof(aad), 0);
	gcmp_aad_nonce(hdr, data, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "GCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "GCMP nonce", nonce, sizeof(nonce));

	if (wlan_crypto_aes_gcm_ad(tk, tk_len, nonce, sizeof(nonce), m, mlen,
					aad, aad_len, m + mlen, plain) < 0) {
		/*uint16_t seq_ctrl = qdf_le16_to_cpu(hdr->seq_ctrl);
		wpa_printf(MSG_INFO, "Invalid GCMP frame: A1=" MACSTR
			   " A2=" MACSTR " A3=" MACSTR " seq=%u frag=%u",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3),
			   WLAN_GET_SEQ_SEQ(seq_ctrl),
			   WLAN_GET_SEQ_FRAG(seq_ctrl));*/
		qdf_mem_free(plain);
		return NULL;
	}

	*decrypted_len = mlen;
	return plain;
}


uint8_t *wlan_crypto_gcmp_encrypt(const uint8_t *tk, size_t tk_len,
					const uint8_t *frame, size_t len,
					size_t hdrlen, const uint8_t *qos,
					const uint8_t *pn, int keyid,
					size_t *encrypted_len){
	uint8_t aad[30], nonce[12], *crypt, *pos;
	size_t aad_len, plen;
	struct ieee80211_hdr *hdr;

	if (len < hdrlen || hdrlen < 24)
		return NULL;
	plen = len - hdrlen;

	crypt = qdf_mem_malloc(hdrlen + 8 + plen + 16 + AES_BLOCK_SIZE);
	if (crypt == NULL) {
		qdf_print("%s[%d] mem alloc failed\n", __func__, __LINE__);
		return NULL;
	}

	qdf_mem_copy(crypt, frame, hdrlen);
	hdr = (struct ieee80211_hdr *) crypt;
	pos = crypt + hdrlen;
	*pos++ = pn[5]; /* PN0 */
	*pos++ = pn[4]; /* PN1 */
	*pos++ = 0x00; /* Rsvd */
	*pos++ = 0x20 | (keyid << 6);
	*pos++ = pn[3]; /* PN2 */
	*pos++ = pn[2]; /* PN3 */
	*pos++ = pn[1]; /* PN4 */
	*pos++ = pn[0]; /* PN5 */

	qdf_mem_set(aad, sizeof(aad), 0);
	gcmp_aad_nonce(hdr, crypt + hdrlen, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "GCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "GCMP nonce", nonce, sizeof(nonce));

	if (wlan_crypto_aes_gcm_ae(tk, tk_len, nonce, sizeof(nonce),
					frame + hdrlen, plen, aad, aad_len,
					pos, pos + plen) < 0) {
		qdf_mem_free(crypt);
		return NULL;
	}

	wpa_hexdump(MSG_EXCESSIVE, "GCMP MIC", pos + plen, 16);
	wpa_hexdump(MSG_EXCESSIVE, "GCMP encrypted", pos, plen);

	*encrypted_len = hdrlen + 8 + plen + 16;

	return crypt;
}

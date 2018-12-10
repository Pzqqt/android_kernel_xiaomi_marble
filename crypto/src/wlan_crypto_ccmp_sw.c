/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 */
/*
 * CTR with CBC-MAC Protocol (CCMP)
 * Copyright (c) 2010-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include "wlan_crypto_aes_i.h"
#include "wlan_crypto_def_i.h"

static void ccmp_aad_nonce(const struct wlan_frame_hdr *hdr, const uint8_t *data,
			   uint8_t *aad, size_t *aad_len, uint8_t *nonce)
{
	uint16_t seq;
	uint8_t stype;
	int qos = 0, addr4 = 0;
	uint8_t *pos;

	nonce[0] = 0;

	stype = WLAN_FC0_GET_STYPE(hdr->i_fc[0]);
	if ((hdr->i_fc[1] & WLAN_FC1_DIR_MASK) ==
	    (WLAN_FC1_DSTODS))
		addr4 = 1;

	if (WLAN_FC0_GET_TYPE(hdr->i_fc[0]) == WLAN_FC0_TYPE_DATA) {
		aad[0] &= ~0x70; /* Mask subtype bits */
		if (stype & 0x08) {
			const uint8_t *qc;
			qos = 1;
			aad[1] &= ~WLAN_FC1_ORDER;
			qc = (const uint8_t *) (hdr + 1);
			if (addr4)
				qc += QDF_MAC_ADDR_SIZE;
			nonce[0] = qc[0] & 0x0f;
		}
	} else if (WLAN_FC0_GET_TYPE(hdr->i_fc[0])
						== WLAN_FC0_TYPE_MGMT) {
		nonce[0] |= 0x10; /* Management */
	}

	aad[1] &= ~(WLAN_FC1_RETRY | WLAN_FC1_PWRMGT | WLAN_FC1_MOREDATA);
	aad[1] |= WLAN_FC1_ISWEP;
	pos = aad + 2;
	qdf_mem_copy(pos, hdr->i_addr1, 3 * QDF_MAC_ADDR_SIZE);
	pos += 3 * QDF_MAC_ADDR_SIZE;
	seq = qdf_le16_to_cpu(*((uint16_t *)&hdr->i_seq[0]));
	seq &= ~0xfff0; /* Mask Seq#; do not modify Frag# */
	wlan_crypto_put_le16(pos, seq);
	pos += 2;

	qdf_mem_copy(pos, hdr + 1, addr4 * QDF_MAC_ADDR_SIZE + qos * 2);
	pos += addr4 * QDF_MAC_ADDR_SIZE;
	if (qos) {
		pos[0] &= ~0x70;
		if (1 /* FIX: either device has SPP A-MSDU Capab = 0 */)
			pos[0] &= ~0x80;
		pos++;
		*pos++ = 0x00;
	}

	*aad_len = pos - aad;

	qdf_mem_copy(nonce + 1, hdr->i_addr2, QDF_MAC_ADDR_SIZE);
	nonce[7] = data[7]; /* PN5 */
	nonce[8] = data[6]; /* PN4 */
	nonce[9] = data[5]; /* PN3 */
	nonce[10] = data[4]; /* PN2 */
	nonce[11] = data[1]; /* PN1 */
	nonce[12] = data[0]; /* PN0 */
}


uint8_t *wlan_crypto_ccmp_decrypt(const uint8_t *tk,
					const struct wlan_frame_hdr *hdr,
					uint8_t *data, size_t data_len){
	uint8_t aad[30], nonce[13];
	size_t aad_len;
	size_t mlen;
	uint8_t *plain;

	if (data_len < CCMP_IV_SIZE + WLAN_CRYPTO_MIC_LEN)
		return NULL;

	plain = qdf_mem_malloc(data_len + AES_BLOCK_SIZE);
	if (plain == NULL) {
		crypto_err("mem alloc failed");
		return NULL;
	}

	mlen = data_len - CCMP_IV_SIZE - WLAN_CRYPTO_MIC_LEN;

	qdf_mem_set(aad, sizeof(aad), 0);
	ccmp_aad_nonce(hdr, data, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP nonce", nonce, 13);

	if (wlan_crypto_aes_ccm_ad(tk, 16, nonce,
					WLAN_CRYPTO_MIC_LEN,
					data + CCMP_IV_SIZE, mlen,
					aad, aad_len,
					data + CCMP_IV_SIZE + mlen,
					plain) < 0) {
		/*uint16_t seq_ctrl = qdf_le16_to_cpu(hdr->seq_ctrl);
		wpa_printf(MSG_INFO, "Invalid CCMP MIC in frame: A1=" MACSTR
			" A2=" MACSTR " A3=" MACSTR " seq=%u frag=%u",
			MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			MAC2STR(hdr->addr3),
			WLAN_GET_SEQ_SEQ(seq_ctrl),
			WLAN_GET_SEQ_FRAG(seq_ctrl));*/
		qdf_mem_free(plain);
		return NULL;
	}
	wpa_hexdump(MSG_EXCESSIVE, "CCMP decrypted", plain, mlen);

	qdf_mem_copy(data, plain, data_len);
	qdf_mem_free(plain);
	return data;
}


void ccmp_get_pn(uint8_t *pn, const uint8_t *data)
{
	pn[0] = data[7]; /* PN5 */
	pn[1] = data[6]; /* PN4 */
	pn[2] = data[5]; /* PN3 */
	pn[3] = data[4]; /* PN2 */
	pn[4] = data[1]; /* PN1 */
	pn[5] = data[0]; /* PN0 */
}


uint8_t *wlan_crypto_ccmp_encrypt(const uint8_t *tk, uint8_t *frame,
					size_t len, size_t hdrlen){
	uint8_t aad[30], nonce[13];
	size_t aad_len, plen;
	uint8_t *crypt, *pos;
	struct wlan_frame_hdr *hdr;

	if (len < hdrlen || hdrlen < 24)
		return NULL;
	plen = len - hdrlen - CCMP_IV_SIZE - WLAN_CRYPTO_MIC_LEN;

	crypt = qdf_mem_malloc(hdrlen + CCMP_IV_SIZE + plen
				+ WLAN_CRYPTO_MIC_LEN + AES_BLOCK_SIZE);
	if (crypt == NULL) {
		crypto_err("mem alloc failed");
		return NULL;
	}

	qdf_mem_copy(crypt, frame, hdrlen + CCMP_IV_SIZE);

	hdr = (struct wlan_frame_hdr *) crypt;
	hdr->i_fc[1] |= WLAN_FC1_ISWEP;
	pos = crypt + hdrlen + 8;

	qdf_mem_set(aad, sizeof(aad), 0);
	ccmp_aad_nonce(hdr, crypt + hdrlen, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP nonce", nonce, 13);

	if (wlan_crypto_aes_ccm_ae(tk, 16, nonce, WLAN_CRYPTO_MIC_LEN,
					frame + hdrlen + CCMP_IV_SIZE,
			plen, aad, aad_len, pos, pos + plen) < 0) {
		qdf_mem_free(crypt);
		return NULL;
	}

	qdf_mem_copy(frame, crypt, len);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP encrypted",
				crypt + hdrlen + CCMP_IV_SIZE, plen);
	qdf_mem_free(crypt);

	return frame;
}


uint8_t *wlan_crypto_ccmp_256_decrypt(const uint8_t *tk,
					const struct wlan_frame_hdr *hdr,
					const uint8_t *data, size_t data_len,
					size_t *decrypted_len){
	uint8_t aad[30], nonce[13];
	size_t aad_len;
	size_t mlen;
	uint8_t *plain;

	if (data_len < CCMP_IV_SIZE + WLAN_CRYPTO_MIC256_LEN)
		return NULL;

	plain = qdf_mem_malloc(data_len + AES_BLOCK_SIZE);
	if (plain == NULL) {
		crypto_err("mem alloc failed");
		return NULL;
	}

	mlen = data_len - CCMP_IV_SIZE - WLAN_CRYPTO_MIC256_LEN;

	qdf_mem_set(aad, sizeof(aad), 0);
	ccmp_aad_nonce(hdr, data, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 nonce", nonce, 13);

	if (wlan_crypto_aes_ccm_ad(tk, 32, nonce, WLAN_CRYPTO_MIC256_LEN,
					data + CCMP_IV_SIZE, mlen,
					aad, aad_len,
					data + CCMP_IV_SIZE + mlen,
					plain) < 0) {
		/*uint16_t seq_ctrl = qdf_le16_to_cpu(hdr->seq_ctrl);
		wpa_printf(MSG_INFO, "Invalid CCMP-256 MIC in frame: A1=" MACSTR
			   " A2=" MACSTR " A3=" MACSTR " seq=%u frag=%u",
			   MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
			   MAC2STR(hdr->addr3),
			   WLAN_GET_SEQ_SEQ(seq_ctrl),
			   WLAN_GET_SEQ_FRAG(seq_ctrl));*/
		qdf_mem_free(plain);
		return NULL;
	}
	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 decrypted", plain, mlen);

	*decrypted_len = mlen;
	return plain;
}


uint8_t *wlan_crypto_ccmp_256_encrypt(const uint8_t *tk, uint8_t *frame,
					size_t len, size_t hdrlen, uint8_t *qos,
					uint8_t *pn, int keyid,
					size_t *encrypted_len){
	uint8_t aad[30], nonce[13];
	size_t aad_len, plen;
	uint8_t *crypt, *pos;
	struct wlan_frame_hdr *hdr;

	if (len < hdrlen || hdrlen < 24)
		return NULL;
	plen = len - hdrlen;

	crypt = qdf_mem_malloc(hdrlen + CCMP_IV_SIZE + plen
				+ WLAN_CRYPTO_MIC256_LEN + AES_BLOCK_SIZE);
	if (crypt == NULL) {
		crypto_err("mem alloc failed");
		return NULL;
	}

	qdf_mem_copy(crypt, frame, hdrlen);
	hdr = (struct wlan_frame_hdr *) crypt;
	hdr->i_fc[1] |= WLAN_FC1_ISWEP;
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
	ccmp_aad_nonce(hdr, crypt + hdrlen, aad, &aad_len, nonce);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 AAD", aad, aad_len);
	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 nonce", nonce, 13);

	if (wlan_crypto_aes_ccm_ae(tk, 32, nonce, WLAN_CRYPTO_MIC256_LEN,
				frame + hdrlen, plen,
				aad, aad_len, pos, pos + plen) < 0) {
		qdf_mem_free(crypt);
		return NULL;
	}

	wpa_hexdump(MSG_EXCESSIVE, "CCMP-256 encrypted", crypt + hdrlen + 8,
		    plen);

	*encrypted_len = hdrlen + CCMP_IV_SIZE
				+ plen + WLAN_CRYPTO_MIC256_LEN;

	return crypt;
}

/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */
/*
 * Galois/Counter Mode (GCM) and GMAC with AES
 *
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

static void inc32(uint8_t *block)
{
	uint32_t val;
	val = wlan_crypto_get_be32(block + AES_BLOCK_SIZE - 4);
	val++;
	wlan_crypto_put_be32(block + AES_BLOCK_SIZE - 4, val);
}


static void xor_block(uint8_t *dst, const uint8_t *src)
{
	uint32_t *d = (uint32_t *) dst;
	uint32_t *s = (uint32_t *) src;
	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
}


static void shift_right_block(uint8_t *v)
{
	uint32_t val;

	val = wlan_crypto_get_be32(v + 12);
	val >>= 1;
	if (v[11] & 0x01)
		val |= 0x80000000;
	wlan_crypto_put_be32(v + 12, val);

	val = wlan_crypto_get_be32(v + 8);
	val >>= 1;
	if (v[7] & 0x01)
		val |= 0x80000000;
	wlan_crypto_put_be32(v + 8, val);

	val = wlan_crypto_get_be32(v + 4);
	val >>= 1;
	if (v[3] & 0x01)
		val |= 0x80000000;
	wlan_crypto_put_be32(v + 4, val);

	val = wlan_crypto_get_be32(v);
	val >>= 1;
	wlan_crypto_put_be32(v, val);
}


/* Multiplication in GF(2^128) */
static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
{
	uint8_t v[16];
	int i, j;

	qdf_mem_set(z, 16, 0); /* Z_0 = 0^128 */
	qdf_mem_copy(v, y, 16); /* V_0 = Y */

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			if (x[i] & BIT(7 - j)) {
				/* Z_(i + 1) = Z_i XOR V_i */
				xor_block(z, v);
			} else {
				/* Z_(i + 1) = Z_i */
			}

			if (v[15] & 0x01) {
				/* V_(i + 1) = (V_i >> 1) XOR R */
				shift_right_block(v);
				/* R = 11100001 || 0^120 */
				v[0] ^= 0xe1;
			} else {
				/* V_(i + 1) = V_i >> 1 */
				shift_right_block(v);
			}
		}
	}
}


static void ghash_start(uint8_t *y)
{
	/* Y_0 = 0^128 */
	qdf_mem_set(y, 16, 0);
}


static void ghash(const uint8_t *h, const uint8_t *x, size_t xlen, uint8_t *y)
{
	size_t m, i;
	const uint8_t *xpos = x;
	uint8_t tmp[16];

	m = xlen / 16;

	for (i = 0; i < m; i++) {
		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, xpos);
		xpos += 16;

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		qdf_mem_copy(y, tmp, 16);
	}

	if (x + xlen > xpos) {
		/* Add zero padded last block */
		size_t last = x + xlen - xpos;
		qdf_mem_copy(tmp, xpos, last);
		qdf_mem_set(tmp + last, sizeof(tmp) - last, 0);

		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, tmp);

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		qdf_mem_copy(y, tmp, 16);
	}

	/* Return Y_m */
}


static void aes_gctr(void *aes, const uint8_t *icb, const uint8_t *x,
			size_t xlen, uint8_t *y){
	size_t i, n, last;
	uint8_t cb[AES_BLOCK_SIZE], tmp[AES_BLOCK_SIZE];
	const uint8_t *xpos = x;
	uint8_t *ypos = y;

	if (xlen == 0)
		return;

	n = xlen / 16;

	qdf_mem_copy(cb, icb, AES_BLOCK_SIZE);
	/* Full blocks */
	for (i = 0; i < n; i++) {
		wlan_crypto_aes_encrypt(aes, cb, ypos);
		xor_block(ypos, xpos);
		xpos += AES_BLOCK_SIZE;
		ypos += AES_BLOCK_SIZE;
		inc32(cb);
	}

	last = x + xlen - xpos;
	if (last) {
		/* Last, partial block */
		wlan_crypto_aes_encrypt(aes, cb, tmp);
		for (i = 0; i < last; i++)
			*ypos++ = *xpos++ ^ tmp[i];
	}
}


static void *aes_gcm_init_hash_subkey(const uint8_t *key, size_t key_len,
					uint8_t *H){
	void *aes;

	aes = wlan_crypto_aes_encrypt_init(key, key_len);
	if (aes == NULL)
		return NULL;

	/* Generate hash subkey H = AES_K(0^128) */
	qdf_mem_set(H, AES_BLOCK_SIZE, 0);
	wlan_crypto_aes_encrypt(aes, H, H);
	wpa_hexdump_key(MSG_EXCESSIVE, "Hash subkey H for GHASH",
			H, AES_BLOCK_SIZE);
	return aes;
}


static void aes_gcm_prepare_j0(const uint8_t *iv, size_t iv_len,
				const uint8_t *H, uint8_t *J0){
	uint8_t len_buf[16];

	if (iv_len == 12) {
		/* Prepare block J_0 = IV || 0^31 || 1 [len(IV) = 96] */
		qdf_mem_copy(J0, iv, iv_len);
		qdf_mem_set(J0 + iv_len, AES_BLOCK_SIZE - iv_len, 0);
		J0[AES_BLOCK_SIZE - 1] = 0x01;
	} else {
		/*
		 * s = 128 * ceil(len(IV)/128) - len(IV)
		 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64)
		 */
		ghash_start(J0);
		ghash(H, iv, iv_len, J0);
		wlan_crypto_put_be64(len_buf, 0);
		wlan_crypto_put_be64(len_buf + 8, iv_len * 8);
		ghash(H, len_buf, sizeof(len_buf), J0);
	}
}


static void aes_gcm_gctr(void *aes, const uint8_t *J0, const uint8_t *in,
				size_t len, uint8_t *out){
	uint8_t J0inc[AES_BLOCK_SIZE];

	if (len == 0)
		return;

	qdf_mem_copy(J0inc, J0, AES_BLOCK_SIZE);
	inc32(J0inc);
	aes_gctr(aes, J0inc, in, len, out);
}


static void aes_gcm_ghash(const uint8_t *H, const uint8_t *aad, size_t aad_len,
			  const uint8_t *crypt, size_t crypt_len, uint8_t *S)
{
	uint8_t len_buf[16];

	/*
	 * u = 128 * ceil[len(C)/128] - len(C)
	 * v = 128 * ceil[len(A)/128] - len(A)
	 * S = GHASH_H(A || 0^v || C || 0^u || [len(A)]64 || [len(C)]64)
	 * (i.e., zero padded to block size A || C and lengths of each in bits)
	 */
	ghash_start(S);
	ghash(H, aad, aad_len, S);
	ghash(H, crypt, crypt_len, S);
	wlan_crypto_put_be64(len_buf, aad_len * 8);
	wlan_crypto_put_be64(len_buf + 8, crypt_len * 8);
	ghash(H, len_buf, sizeof(len_buf), S);

	wpa_hexdump_key(MSG_EXCESSIVE, "S = GHASH_H(...)", S, 16);
}


/**
 * aes_gcm_ae - GCM-AE_K(IV, P, A)
 */
int wlan_crypto_aes_gcm_ae(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len, const uint8_t *plain,
			size_t plain_len, const uint8_t *aad, size_t aad_len,
			uint8_t *crypt, uint8_t *tag){
	uint8_t H[AES_BLOCK_SIZE];
	uint8_t J0[AES_BLOCK_SIZE];
	uint8_t S[16];
	void *aes;
	int32_t status = -1;

	aes = aes_gcm_init_hash_subkey(key, key_len, H);
	if (aes == NULL)
		return status;

	aes_gcm_prepare_j0(iv, iv_len, H, J0);

	/* C = GCTR_K(inc_32(J_0), P) */
	aes_gcm_gctr(aes, J0, plain, plain_len, crypt);

	aes_gcm_ghash(H, aad, aad_len, crypt, plain_len, S);

	/* T = MSB_t(GCTR_K(J_0, S)) */
	aes_gctr(aes, J0, S, sizeof(S), tag);

	/* Return (C, T) */

	wlan_crypto_aes_encrypt_deinit(aes);

	return 0;
}


/**
 * aes_gcm_ad - GCM-AD_K(IV, C, A, T)
 */
int wlan_crypto_aes_gcm_ad(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len, const uint8_t *crypt,
			size_t crypt_len, const uint8_t *aad, size_t aad_len,
			const uint8_t *tag, uint8_t *plain){
	uint8_t H[AES_BLOCK_SIZE];
	uint8_t J0[AES_BLOCK_SIZE];
	uint8_t S[16], T[16];
	void *aes;
	int32_t status = -1;

	aes = aes_gcm_init_hash_subkey(key, key_len, H);
	if (aes == NULL)
		return status;

	aes_gcm_prepare_j0(iv, iv_len, H, J0);

	/* P = GCTR_K(inc_32(J_0), C) */
	aes_gcm_gctr(aes, J0, crypt, crypt_len, plain);

	aes_gcm_ghash(H, aad, aad_len, crypt, crypt_len, S);

	/* T' = MSB_t(GCTR_K(J_0, S)) */
	aes_gctr(aes, J0, S, sizeof(S), T);

	wlan_crypto_aes_encrypt_deinit(aes);

	if (qdf_mem_cmp(tag, T, 16) != 0) {
		wpa_printf(MSG_EXCESSIVE, "GCM: Tag mismatch");
		return status;
	}

	return 0;
}


int wlan_crypto_aes_gmac(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len, const uint8_t *aad,
			size_t aad_len, uint8_t *tag){
	return wlan_crypto_aes_gcm_ae(key, key_len, iv, iv_len, NULL, 0,
					aad, aad_len, NULL, tag);
}

/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */

/*
 * AES SIV (RFC 5297)
 * Copyright (c) 2013 Cozybit, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef WLAN_SUPPORT_FILS

#include <qdf_crypto.h>
#include <wlan_crypto_aes_i.h>
#include "wlan_crypto_aes_ctr_i.h"

static const uint8_t zero[AES_BLOCK_SIZE];

static void dbl(uint8_t *pad)
{
	int32_t i, carry;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}

static void xor(uint8_t *a, const uint8_t *b)
{
	int32_t i;

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		*a++ ^= *b++;
}

static void xorend(uint8_t *a, int32_t alen, const uint8_t *b, int32_t blen)
{
	int32_t i;

	if (alen < blen)
		return;

	for (i = 0; i < blen; i++)
		a[alen - blen + i] ^= b[i];
}

static void pad_block(uint8_t *pad, const uint8_t *addr, size_t len)
{
	qdf_mem_zero(pad, AES_BLOCK_SIZE);
	qdf_mem_copy(pad, addr, len);

	if (len < AES_BLOCK_SIZE)
		pad[len] = 0x80;
}

static int32_t
aes_s2v(const uint8_t *key, size_t key_len, size_t num_elem,
	const uint8_t *addr[], size_t *len, uint8_t *mac)
{
	uint8_t tmp[AES_BLOCK_SIZE], tmp2[AES_BLOCK_SIZE];
	uint8_t *buf = NULL;
	int32_t ret = -1;
	size_t i;
	const uint8_t *data[1];
	size_t data_len[1];

	if (!num_elem) {
		qdf_mem_copy(tmp, zero, sizeof(zero));
		tmp[AES_BLOCK_SIZE - 1] = 1;
		data[0] = tmp;
		data_len[0] = sizeof(tmp);
		return omac1_aes_vector(key, key_len, 1, data, data_len, mac);
	}

	data[0] = zero;
	data_len[0] = sizeof(zero);
	ret = omac1_aes_vector(key, key_len, 1, data, data_len, tmp);
	if (ret)
		return ret;

	for (i = 0; i < num_elem - 1; i++) {
		ret = omac1_aes_vector(key, key_len, 1, &addr[i], &len[i],
				       tmp2);
		if (ret)
			return ret;

		dbl(tmp);
		xor(tmp, tmp2);
	}
	if (len[i] >= AES_BLOCK_SIZE) {
		buf = OS_MALLOC(NULL, len[i], GFP_ATOMIC);
		if (!buf)
			return -ENOMEM;

		qdf_mem_copy(buf, addr[i], len[i]);
		xorend(buf, len[i], tmp, AES_BLOCK_SIZE);
		data[0] = buf;
		ret = omac1_aes_vector(key, key_len, 1, data, &len[i], mac);
		memset(buf, 0, len[i]);
		OS_FREE(buf);
		return ret;
	}

	dbl(tmp);
	pad_block(tmp2, addr[i], len[i]);
	xor(tmp, tmp2);

	data[0] = tmp;
	data_len[0] = sizeof(tmp);

	return omac1_aes_vector(key, key_len, 1, data, data_len, mac);
}

int32_t wlan_crypto_aes_siv_encrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *pw, size_t pwlen,
				    size_t num_elem, const uint8_t *addr[],
				    const size_t *len, uint8_t *out)
{
	const uint8_t *_addr[6];
	size_t _len[6];
	const uint8_t *k1, *k2;
	uint8_t v[AES_BLOCK_SIZE];
	size_t i;
	uint8_t *iv, *crypt_pw;
	int32_t status = -1;

	if (num_elem > ARRAY_SIZE(_addr) - 1 ||
	    (key_len != 32 && key_len != 48 && key_len != 64))
		return status;

	key_len /= 2;
	k1 = key;
	k2 = key + key_len;

	for (i = 0; i < num_elem; i++) {
		_addr[i] = addr[i];
		_len[i] = len[i];
	}
	_addr[num_elem] = pw;
	_len[num_elem] = pwlen;

	if (aes_s2v(k1, key_len, num_elem + 1, _addr, _len, v))
		return status;

	iv = out;
	crypt_pw = out + AES_BLOCK_SIZE;

	qdf_mem_copy(iv, v, AES_BLOCK_SIZE);
	qdf_mem_copy(crypt_pw, pw, pwlen);

	/* zero out 63rd and 31st bits of ctr (from right) */
	v[8] &= 0x7f;
	v[12] &= 0x7f;

	return wlan_crypto_aes_ctr_encrypt(k2, key_len, v, crypt_pw, pwlen);
}

int32_t wlan_crypto_aes_siv_decrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *iv_crypt, size_t iv_c_len,
				    size_t num_elem, const uint8_t *addr[],
				    const size_t *len, uint8_t *out)
{
	const uint8_t *_addr[6];
	size_t _len[6];
	const uint8_t *k1, *k2;
	size_t crypt_len;
	size_t i;
	int32_t ret = -1;
	uint8_t iv[AES_BLOCK_SIZE];
	uint8_t check[AES_BLOCK_SIZE];

	if (iv_c_len < AES_BLOCK_SIZE || num_elem > ARRAY_SIZE(_addr) - 1 ||
	    (key_len != 32 && key_len != 48 && key_len != 64))
		return ret;

	crypt_len = iv_c_len - AES_BLOCK_SIZE;
	key_len /= 2;
	k1 = key;
	k2 = key + key_len;

	for (i = 0; i < num_elem; i++) {
		_addr[i] = addr[i];
		_len[i] = len[i];
	}
	_addr[num_elem] = out;
	_len[num_elem] = crypt_len;

	qdf_mem_copy(iv, iv_crypt, AES_BLOCK_SIZE);
	qdf_mem_copy(out, iv_crypt + AES_BLOCK_SIZE, crypt_len);

	iv[8] &= 0x7f;
	iv[12] &= 0x7f;

	ret = wlan_crypto_aes_ctr_encrypt(k2, key_len, iv, out, crypt_len);
	if (ret)
		return ret;

	ret = aes_s2v(k1, key_len, num_elem + 1, _addr, _len, check);
	if (ret)
		return ret;

	if (qdf_mem_cmp(check, iv_crypt, AES_BLOCK_SIZE) == 0)
		return 0;

	return ret;
}

#endif /* WLAN_SUPPORT_FILS */

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
 * DOC: qdf_crypto.h
 * This file provides OS abstraction for crypto APIs.
 */

#if !defined(__QDF_CRYPTO_H)
#define __QDF_CRYPTO_H

/* Include Files */
#include "qdf_status.h"
#include <qdf_types.h>
#include <qdf_trace.h>

/* Preprocessor definitions and constants */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AES_BLOCK_SIZE 16
#define HMAC_SHA256_CRYPTO_TYPE "hmac(sha256)"
#define HMAC_SHA386_CRYPTO_TYPE "hmac(sha384)"

#define SHA256_CRYPTO_TYPE "sha256"
#define SHA386_CRYPTO_TYPE "sha384"

#define SHA256_DIGEST_SIZE 32
#define SHA384_DIGEST_SIZE 48

#define FIXED_PARAM_OFFSET_ASSOC_REQ 4
#define FIXED_PARAM_OFFSET_ASSOC_RSP 6

#define AAD_LEN 20
#define IEEE80211_MMIE_GMAC_MICLEN  16

#define IS_VALID_CTR_KEY_LEN(len) ((((len) == 16) || ((len) == 32) || \
	((len) == 48)) ? 1 : 0)

/* Function declarations and documenation */

/**
 * qdf_get_hash: API to get hash using specific crypto and scatterlist
 * @type: crypto type
 * @element_cnt: scatterlist element count
 * @addr: scatterlist element array
 * @addr_len: element length array
 * @hash: new hash
 *
 * Return: 0 if success else error code
 */
int qdf_get_hash(uint8_t *type, uint8_t element_cnt,
		uint8_t *addr[], uint32_t *addr_len,
		int8_t *hash);

/**
 * qdf_get_hmac_hash: API to get hmac hash using specific crypto and
 * scatterlist elements.
 * @type: crypto type
 * @key: key needs to be used for hmac api
 * @keylen: length of key
 * @element_cnt: scatterlist element count
 * @addr: scatterlist element array
 * @addr_len: element length array
 * @hash: new hash
 *
 * Return: 0 if success else error code
 */
int qdf_get_hmac_hash(uint8_t *type, uint8_t *key,
		uint32_t keylen, uint8_t element_cnt,
		uint8_t *addr[], uint32_t *addr_len, int8_t *hash);

/**
 * qdf_get_keyed_hash: API to get hash using specific crypto and
 * scatterlist elements.
 * @type: crypto type
 * @key: key needs to be used for hmac api
 * @keylen: length of key
 * @element_cnt: scatterlist element count
 * @addr: scatterlist element array
 * @addr_len: element length array
 * @hash: new hash
 *
 * Return: 0 if success else error code
 */
int qdf_get_keyed_hash(const char *alg, const uint8_t *key,
			unsigned int key_len, const uint8_t *src[],
			size_t *src_len, size_t num_elements, uint8_t *out);
/**
 * qdf_update_dbl: This API does the doubling operation as defined in RFC5297
 * @d: input for doubling
 *
 * Return: None
 */
void qdf_update_dbl(uint8_t *d);

/**
 * qdf_aes_s2v: This API gets vector from AES string as defined in RFC5297
 * output length will be AES_BLOCK_SIZE.
 * @key: key used for operation
 * @key_len: key len
 * @s: addresses of elements to be used
 * @s_len: array of element length
 * @num_s: number of elements
 * @out: pointer to output vector
 *
 * Return: 0 if success else Error number
 */
int qdf_aes_s2v(const uint8_t *key, unsigned int key_len, const uint8_t *s[],
		   size_t s_len[], size_t num_s, uint8_t *out);

/**
 * qdf_aes_ctr: This API defines AES Counter Mode
 * @key: key used for operation
 * @key_len: key len
 * @siv: Initialization vector
 * @src: input
 * @src_len: input len
 * @dest: output
 * @enc: if encryption needs to be done or decryption
 *
 * Return: 0 if success else Error number
 */
int qdf_aes_ctr(const uint8_t *key, unsigned int key_len, uint8_t *siv,
		const uint8_t *src, size_t src_len, uint8_t *dest, bool enc);

/**
 * qdf_crypto_aes_gmac: This API calculates MIC for GMAC
 * @key: key used for operation
 * @key_length: key length
 * @iv: Initialization vector
 * @aad: Additional authentication data
 * @data: Pointer to data
 * @data_len: Length of data
 * @mic: Pointer to MIC
 *
 * Return: 0 if success else Error number
 */
int qdf_crypto_aes_gmac(uint8_t *key, uint16_t key_length,
			uint8_t *iv, uint8_t *aad, uint8_t *data,
			uint16_t data_len, uint8_t *mic);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __QDF_CRYPTO_H */

/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_crypto.c
 *
 * This source file contains linux specific definitions for QDF crypto APIs
 */

/* Include Files */
#include "qdf_crypto.h"
#include <linux/export.h>
#include <crypto/hash.h>
#include <crypto/aes.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <linux/ieee80211.h>

/* Function Definitions and Documentation */
#define MAX_HMAC_ELEMENT_CNT 10

/*
 * xor: API to calculate xor
 * @a: first variable
 * @b: second variable
 * @len: length of variables
 */
static void xor(uint8_t *a, const uint8_t *b, size_t len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
	a[i] ^= b[i];
}

int qdf_get_hash(uint8_t *type,
		uint8_t element_cnt, uint8_t *addr[], uint32_t *addr_len,
		int8_t *hash)
{
	return qdf_get_hmac_hash(type, NULL, 0, element_cnt,
				 addr, addr_len, hash);
}

int qdf_get_hmac_hash(uint8_t *type, uint8_t *key,
		uint32_t keylen,
		uint8_t element_cnt, uint8_t *addr[], uint32_t *addr_len,
		int8_t *hash)
{
	int i;
	size_t src_len[MAX_HMAC_ELEMENT_CNT];

	if (element_cnt > MAX_HMAC_ELEMENT_CNT) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid element count %d"), element_cnt);
		return -EINVAL;
	}

	for (i = 0; i < element_cnt; i++)
		src_len[i] = addr_len[i];

	return qdf_get_keyed_hash(type, key, keylen, (const uint8_t **)addr,
				  src_len, element_cnt,  hash);
}

/* qdf_update_dbl from RFC 5297. Length of d is AES_BLOCK_SIZE (128 bits) */
void qdf_update_dbl(uint8_t *d)
{
	int i;
	uint8_t msb, msb_prev = 0;

	/* left shift by 1 */
	for (i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
		msb = d[i] & 0x80;
		d[i] = d[i] << 1;
		d[i] += msb_prev ? 1 : 0;
		msb_prev = msb;
	}

	if (msb)
		d[AES_BLOCK_SIZE - 1] ^= 0x87;
}

/**
 * set_desc_flags() - set flags variable in the shash_desc struct
 * @desc: pointer to shash_desc struct
 * @tfm: pointer to crypto_shash struct
 *
 * Set the flags variable in the shash_desc struct by getting the flag
 * from the crypto_hash struct. The flag is not actually used, prompting
 * its removal from kernel code in versions 5.2 and above. Thus, for
 * versions 5.2 and above, do not set the flag variable of shash_desc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
static void set_desc_flags(struct shash_desc *desc, struct crypto_shash *tfm)
{
	desc->flags = crypto_shash_get_flags(tfm);
}
#else
static void set_desc_flags(struct shash_desc *desc, struct crypto_shash *tfm)
{
}
#endif

int qdf_get_keyed_hash(const char *alg, const uint8_t *key,
			unsigned int key_len, const uint8_t *src[],
			size_t *src_len, size_t num_elements, uint8_t *out)
{
	struct crypto_shash *tfm;
	int ret;
	size_t i;

	tfm = crypto_alloc_shash(alg, 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to allocate transformation for %s: %ld"),
			  alg, PTR_ERR(tfm));
		return -EINVAL;
	}

	if (key && key_len) {
		ret = crypto_shash_setkey(tfm, key, key_len);
		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  FL("Set key failed for %s, ret:%d"),
				  alg, -ret);
			goto error;
		}
	}

	do {
		SHASH_DESC_ON_STACK(desc, tfm);
		desc->tfm = tfm;
		set_desc_flags(desc, tfm);

		ret = crypto_shash_init(desc);
		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  FL("Failed to init hash for %s, ret:%d"),
				  alg, -ret);
			goto error;
		}

		for (i = 0; i < num_elements; i++) {
			ret = crypto_shash_update(desc, src[i], src_len[i]);
			if (ret) {
				QDF_TRACE(QDF_MODULE_ID_QDF,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Failed to update hash for %s, ret:%d"),
					  alg, -ret);
				goto error;
			}
		}

		ret = crypto_shash_final(desc, out);
		if (ret)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  FL("Failed to get digest for %s, ret:%d"),
				  alg, -ret);
	} while (0);

error:
	crypto_free_shash(tfm);
	return ret;
}

/* AES String to Vector from RFC 5297, 'out' should be of length AES_BLOCK_SIZE
 */
int qdf_aes_s2v(const uint8_t *key, unsigned int key_len, const uint8_t *s[],
		   size_t s_len[], size_t num_s, uint8_t *out)
{
	const char *alg = "cmac(aes)";
	uint8_t d[AES_BLOCK_SIZE];
	uint8_t buf[AES_BLOCK_SIZE] = { 0 };
	size_t buf_len = AES_BLOCK_SIZE;
	const uint8_t *a[1];
	unsigned int i;
	uint8_t *t = NULL;
	size_t t_len;
	int ret;

	if (num_s == 0) {
		/* V = AES-CMAC(K, <one>) */
		buf[0] = 0x01;
		a[0] = buf;
		ret = qdf_get_keyed_hash(alg, key, key_len, a, &buf_len, 1,
					 out);
		return ret;
	}

	/* D = AES-CMAC(K, <zero>) */
	a[0] = buf;
	ret = qdf_get_keyed_hash(alg, key, key_len, a, &buf_len, 1, d);
	if (ret)
		goto error;

	for (i = 0; i < num_s - 1; i++) {
		/* D = qdf_update_dbl(D) xor AES-CMAC(K, Si) */
		qdf_update_dbl(d);
		ret = qdf_get_keyed_hash(alg, key, key_len, &s[i], &s_len[i], 1,
					 buf);
		if (ret)
			goto error;
		xor(d, buf, AES_BLOCK_SIZE);
	}

	if (s_len[i] >= AES_BLOCK_SIZE) {
		/* len(Sn) >= 128 */
		/* T = Sn xorend D */
		t = qdf_mem_malloc(s_len[i]);
		if (!t)
			return -EINVAL;
		qdf_mem_copy(t, s[i], s_len[i]);
		xor(t + s_len[i] - AES_BLOCK_SIZE, d, AES_BLOCK_SIZE);
		t_len = s_len[i];
	} else {
		/* len(Sn) < 128 */
		/* T = qdf_update_dbl(D) xor pad(Sn) */
		qdf_update_dbl(d);
		qdf_mem_zero(buf, AES_BLOCK_SIZE);
		qdf_mem_copy(buf, s[i], s_len[i]);
		buf[s_len[i]] = 0x80;
		xor(d, s[i], AES_BLOCK_SIZE);
		t = d;
		t_len = AES_BLOCK_SIZE;
	}

	/* V = AES-CMAC(K, T) */
	a[0] = t;
	ret = qdf_get_keyed_hash(alg, key, key_len, a, &t_len, 1, out);

error:
	if (t && t != d)
		qdf_mem_free(t);
	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
int qdf_aes_ctr(const uint8_t *key, unsigned int key_len, uint8_t *siv,
		const uint8_t *src, size_t src_len, uint8_t *dest, bool enc)
{
	struct crypto_skcipher *tfm;
	struct skcipher_request *req = NULL;
	struct scatterlist sg_in, sg_out;
	int ret;

	if (!IS_VALID_CTR_KEY_LEN(key_len)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid key length: %u"), key_len);
		return -EINVAL;
	}

	tfm = crypto_alloc_skcipher("ctr(aes)", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to alloc transformation for ctr(aes):%ld"),
			  PTR_ERR(tfm));
		return -EAGAIN;
	}

	req = skcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to allocate request for ctr(aes)"));
		crypto_free_skcipher(tfm);
		return -EAGAIN;
	}

	ret = crypto_skcipher_setkey(tfm, key, key_len);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Set key failed for ctr(aes), ret:%d"), -ret);
		skcipher_request_free(req);
		crypto_free_skcipher(tfm);
		return ret;
	}

	sg_init_one(&sg_in, src, src_len);
	sg_init_one(&sg_out, dest, src_len);
	skcipher_request_set_crypt(req, &sg_in, &sg_out, src_len, siv);

	if (enc)
		ret = crypto_skcipher_encrypt(req);
	else
		ret = crypto_skcipher_decrypt(req);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("%s failed for ctr(aes), ret:%d"),
			  enc ? "Encryption" : "Decryption", -ret);
	}

	skcipher_request_free(req);
	crypto_free_skcipher(tfm);
	return ret;
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
int qdf_aes_ctr(const uint8_t *key, unsigned int key_len, uint8_t *siv,
		const uint8_t *src, size_t src_len, uint8_t *dest, bool enc)
{
	struct crypto_ablkcipher *tfm;
	struct ablkcipher_request *req = NULL;
	struct scatterlist sg_in, sg_out;
	int ret;

	if (!IS_VALID_CTR_KEY_LEN(key_len)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid key length: %u"), key_len);
		return -EINVAL;
	}

	tfm = crypto_alloc_ablkcipher("ctr(aes)", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to alloc transformation for ctr(aes):%ld"),
			  PTR_ERR(tfm));
		return -EAGAIN;
	}

	req = ablkcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to allocate request for ctr(aes)"));
		crypto_free_ablkcipher(tfm);
		return -EAGAIN;
	}

	ret = crypto_ablkcipher_setkey(tfm, key, key_len);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("Set key failed for ctr(aes), ret:%d"), -ret);
		ablkcipher_request_free(req);
		crypto_free_ablkcipher(tfm);
		return ret;
	}

	sg_init_one(&sg_in, src, src_len);
	sg_init_one(&sg_out, dest, src_len);
	ablkcipher_request_set_crypt(req, &sg_in, &sg_out, src_len, siv);

	if (enc)
		ret = crypto_ablkcipher_encrypt(req);
	else
		ret = crypto_ablkcipher_decrypt(req);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("%s failed for ctr(aes), ret:%d"),
			  enc ? "Encryption" : "Decryption", -ret);
	}

	ablkcipher_request_free(req);
	crypto_free_ablkcipher(tfm);

	return ret;
}
#else
int qdf_aes_ctr(const uint8_t *key, unsigned int key_len, uint8_t *siv,
		const uint8_t *src, size_t src_len, uint8_t *dest, bool enc)
{
	return -EINVAL;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
int qdf_crypto_aes_gmac(uint8_t *key, uint16_t key_length,
			uint8_t *iv, uint8_t *aad, uint8_t *data,
			uint16_t data_len, uint8_t *mic)
{
	struct crypto_aead *tfm;
	int ret = 0;
	struct scatterlist sg[4];
	uint16_t req_size;
	struct aead_request *req = NULL;
	uint8_t *aad_ptr, *input;

	tfm = crypto_alloc_aead("gcm(aes)", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		ret = PTR_ERR(tfm);
		tfm = NULL;
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: crypto_alloc_aead failed (%d)", __func__, ret);
		goto err_tfm;
	}

	ret = crypto_aead_setkey(tfm, key, key_length);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "crypto_aead_setkey failed (%d)", ret);
		goto err_tfm;
	}

	ret = crypto_aead_setauthsize(tfm, IEEE80211_MMIE_GMAC_MICLEN);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "crypto_aead_setauthsize failed (%d)", ret);
		goto err_tfm;
	}

	/* Prepare aead request */
	req_size = sizeof(*req) + crypto_aead_reqsize(tfm) +
			IEEE80211_MMIE_GMAC_MICLEN + AAD_LEN;
	req = qdf_mem_malloc(req_size);
	if (!req) {
		ret = -ENOMEM;
		goto err_tfm;
	}

	input = (uint8_t *)req + sizeof(*req) + crypto_aead_reqsize(tfm);
	aad_ptr = input + IEEE80211_MMIE_GMAC_MICLEN;
	qdf_mem_copy(aad_ptr, aad, AAD_LEN);

	/* Scatter list operations */
	sg_init_table(sg, 4);
	sg_set_buf(&sg[0], aad_ptr, AAD_LEN);
	sg_set_buf(&sg[1], data, data_len);
	sg_set_buf(&sg[2], input, IEEE80211_MMIE_GMAC_MICLEN);
	sg_set_buf(&sg[3], mic, IEEE80211_MMIE_GMAC_MICLEN);

	aead_request_set_tfm(req, tfm);
	aead_request_set_crypt(req, sg, sg, 0, iv);
	aead_request_set_ad(req,
			    AAD_LEN + data_len + IEEE80211_MMIE_GMAC_MICLEN);
	crypto_aead_encrypt(req);

err_tfm:
	if (tfm)
		crypto_free_aead(tfm);

	if (req)
		qdf_mem_free(req);

	return ret;
}
#else
int qdf_crypto_aes_gmac(uint8_t *key, uint16_t key_length,
			uint8_t *iv, uint8_t *aad, uint8_t *data,
			uint16_t data_len, uint8_t *mic)
{
	return -EINVAL;
}
#endif

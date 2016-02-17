/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */
#if !defined(__CDS_CRYPTO_H)
#define __CDS_CRYPTO_H

/**
 * DOC:  cds_crypto.h
 *
 * Crypto APIs
 *
 */

#include <cdf_types.h>
#include <cdf_status.h>
#include <qdf_status.h>
#include <cdf_memory.h>
#include <qdf_list.h>
#include <cdf_trace.h>
#include <qdf_event.h>
#include <cdf_lock.h>
#include <cds_reg_service.h>
#include <cds_mq.h>
#include <cds_packet.h>
#include <cds_sched.h>
#include <cdf_threads.h>
#include <cdf_mc_timer.h>
#include <cds_pack_align.h>
#include <crypto/aes.h>
#include <crypto/hash.h>
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#include <linux/qcomwlan_secif.h>
#endif

#ifdef CONFIG_CNSS
static inline struct crypto_ahash *cds_crypto_alloc_ahash(const char *alg_name,
	u32 type, u32 mask)
{
	return wcnss_wlan_crypto_alloc_ahash(alg_name, type, mask);
}
#else
static inline struct crypto_ahash *cds_crypto_alloc_ahash(const char *alg_name,
	u32 type, u32 mask)
{
	return crypto_alloc_ahash(alg_name, type, mask);
}
#endif

#ifdef CONFIG_CNSS
static inline struct crypto_cipher *
cds_crypto_alloc_cipher(const char *alg_name, u32 type, u32 mask)
{
	return wcnss_wlan_crypto_alloc_cipher(alg_name, type, mask);
}
#else
static inline struct crypto_cipher *
cds_crypto_alloc_cipher(const char *alg_name, u32 type, u32 mask)
{
	return crypto_alloc_cipher(alg_name, type, mask);
}
#endif

#ifdef CONFIG_CNSS
static inline void cds_cmac_calc_mic(struct crypto_cipher *tfm, u8 *m,
		u16 length, u8 *mac)
{
	wcnss_wlan_cmac_calc_mic(tfm, m, length, mac);
}
#endif

#ifdef CONFIG_CNSS
static inline void cds_crypto_free_cipher(struct crypto_cipher *tfm)
{
	wcnss_wlan_crypto_free_cipher(tfm);
}
#else
static inline void cds_crypto_free_cipher(struct crypto_cipher *tfm)
{
	crypto_free_cipher(tfm);
}
#endif

#ifdef CONFIG_CNSS
static inline void cds_crypto_free_ahash(struct crypto_ahash *tfm)
{
	wcnss_wlan_crypto_free_ahash(tfm);
}
#else
static inline void cds_crypto_free_ahash(struct crypto_ahash *tfm)
{
	crypto_free_ahash(tfm);
}
#endif

#ifdef CONFIG_CNSS
static inline int cds_crypto_ahash_setkey(struct crypto_ahash *tfm,
	const u8 *key, unsigned int keylen)
{
	return wcnss_wlan_crypto_ahash_setkey(tfm, key, keylen);
}
#else
static inline int cds_crypto_ahash_setkey(struct crypto_ahash *tfm,
	const u8 *key, unsigned int keylen)
{
	return crypto_ahash_setkey(tfm, key, keylen);
}
#endif

#ifdef CONFIG_CNSS
static inline int cds_crypto_ahash_digest(struct ahash_request *req)
{
	return wcnss_wlan_crypto_ahash_digest(req);
}
#else
static inline int cds_crypto_ahash_digest(struct ahash_request *req)
{
	return crypto_ahash_digest(req);
}
#endif

#ifdef CONFIG_CNSS
static inline struct crypto_ablkcipher *
cds_crypto_alloc_ablkcipher(const char *alg_name, u32 type, u32 mask)
{
	return wcnss_wlan_crypto_alloc_ablkcipher(alg_name, type, mask);
}
#else
static inline struct crypto_ablkcipher *
cds_crypto_alloc_ablkcipher(const char *alg_name, u32 type, u32 mask)
{
	return crypto_alloc_ablkcipher(alg_name, type, mask);
}
#endif

#ifdef CONFIG_CNSS
static inline void cds_ablkcipher_request_free(struct ablkcipher_request *req)
{
	wcnss_wlan_ablkcipher_request_free(req);
}
#else
static inline void cds_ablkcipher_request_free(struct ablkcipher_request *req)
{
	ablkcipher_request_free(req);
}
#endif

#ifdef CONFIG_CNSS
static inline void cds_crypto_free_ablkcipher(struct crypto_ablkcipher *tfm)
{
	wcnss_wlan_crypto_free_ablkcipher(tfm);
}
#else
static inline void cds_crypto_free_ablkcipher(struct crypto_ablkcipher *tfm)
{
	crypto_free_ablkcipher(tfm);
}
#endif

#endif /* if !defined __CDS_CRYPTO_H */

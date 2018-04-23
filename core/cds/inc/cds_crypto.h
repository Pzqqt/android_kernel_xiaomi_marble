/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#if !defined(__CDS_CRYPTO_H)
#define __CDS_CRYPTO_H

/**
 * DOC:  cds_crypto.h
 *
 * Crypto APIs
 *
 */

#include <linux/crypto.h>

static inline struct crypto_cipher *
cds_crypto_alloc_cipher(const char *alg_name, u32 type, u32 mask)
{
	return crypto_alloc_cipher(alg_name, type, mask);
}

static inline void cds_crypto_free_cipher(struct crypto_cipher *tfm)
{
	crypto_free_cipher(tfm);
}

#endif /* if !defined __CDS_CRYPTO_H */

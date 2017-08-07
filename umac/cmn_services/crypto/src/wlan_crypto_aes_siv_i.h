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

#ifndef _WLAN_CRYPTO_AES_SIV_I_H_
#define _WLAN_CRYPTO_AES_SIV_I_H_

#ifdef WLAN_SUPPORT_FILS
int32_t wlan_crypto_aes_siv_encrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *pw, size_t pwlen,
				    size_t num_elem, const uint8_t *addr[],
				    const size_t *len, uint8_t *out);

int32_t wlan_crypto_aes_siv_decrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *iv_crypt, size_t iv_c_len,
				    size_t num_elem, const uint8_t *addr[],
				    const size_t *len, uint8_t *out);
#endif /* WLAN_SUPPORT_FILS */

#endif /* end of _WLAN_CRYPTO_AES_SIV_I_H_ */


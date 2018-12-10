/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */

/*
 * AES functions
 *
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef _WLAN_CRYPTO_AES_CTR_I_H_
#define _WLAN_CRYPTO_AES_CTR_I_H_

#ifdef WLAN_SUPPORT_FILS
/**
 * wlan_crypto_aes_ctr_encrypt - AES-128/192/256 CTR mode encryption
 * @key: Key for encryption (key_len bytes)
 * @key_len: Length of the key (16, 24, or 32 bytes)
 * @nonce: Nonce for counter mode (16 bytes)
 * @data: Data to encrypt in-place
 * @data_len: Length of data in bytes
 *
 * Returns: 0 on success, -1 on failure
 */
int32_t wlan_crypto_aes_ctr_encrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *nonce, uint8_t *data,
				    size_t data_len);

/**
 * wlan_crypto_aes_128_ctr_encrypt - AES-128 CTR mode encryption
 * @key: Key for encryption (key_len bytes)
 * @nonce: Nonce for counter mode (16 bytes)
 * @data: Data to encrypt in-place
 * @data_len: Length of data in bytes
 *
 * Returns: 0 on success, -1 on failure
 */
int32_t wlan_crypto_aes_128_ctr_encrypt(const uint8_t *key,
					const uint8_t *nonce, uint8_t *data,
					size_t data_len);
#endif /* WLAN_SUPPORT_FILS */

#endif /* end of _WLAN_CRYPTO_AES_CTR_I_H_ */


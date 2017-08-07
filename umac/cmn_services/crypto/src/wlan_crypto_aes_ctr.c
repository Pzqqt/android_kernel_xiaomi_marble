/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */

/*
 * AES-128/192/256 CTR
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef WLAN_SUPPORT_FILS

#include <qdf_crypto.h>
#include "wlan_crypto_aes_i.h"

int32_t wlan_crypto_aes_ctr_encrypt(const uint8_t *key, size_t key_len,
				    const uint8_t *nonce, uint8_t *data,
				    size_t data_len)
{
	void *ctx;
	size_t j, len, left = data_len;
	int32_t i;
	uint8_t *pos = data;
	uint8_t counter[AES_BLOCK_SIZE], buf[AES_BLOCK_SIZE];
	int32_t status = -1;

	ctx = wlan_crypto_aes_encrypt_init(key, key_len);
	if (!ctx)
		return status;

	qdf_mem_copy(counter, nonce, AES_BLOCK_SIZE);

	while (left > 0) {
		wlan_crypto_aes_encrypt(ctx, counter, buf);

		len = (left < AES_BLOCK_SIZE) ? left : AES_BLOCK_SIZE;
		for (j = 0; j < len; j++)
			pos[j] ^= buf[j];
		pos += len;
		left -= len;

		for (i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
			counter[i]++;
			if (counter[i])
				break;
		}
	}
	wlan_crypto_aes_encrypt_deinit(ctx);

	return 0;
}

int32_t wlan_crypto_aes_128_ctr_encrypt(const uint8_t *key,
					const uint8_t *nonce, uint8_t *data,
					size_t data_len)
{
	return wlan_crypto_aes_ctr_encrypt(key, 16, nonce, data, data_len);
}

#endif /* WLAN_SUPPORT_FILS */

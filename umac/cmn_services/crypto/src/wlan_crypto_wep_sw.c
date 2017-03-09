/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */
/*
 * Wired Equivalent Privacy (WEP)
 * Copyright (c) 2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include "wlan_crypto_aes_i.h"
#include "wlan_crypto_def_i.h"

void wlan_crypto_wep_crypt(uint8_t *key, uint8_t *buf, size_t plen)
{
	uint32_t i, j, k;
	uint8_t S[256];
#define S_SWAP(a, b) do { uint8_t t = S[a]; S[a] = S[b]; S[b] = t; } while (0)
	uint8_t *pos;

	/* Setup RC4 state */
	for (i = 0; i < 256; i++)
		S[i] = i;
	j = 0;
	for (i = 0; i < 256; i++) {
		j = (j + S[i] + key[i & 0x0f]) & 0xff;
		S_SWAP(i, j);
	}

	/* Apply RC4 to data */
	pos = buf;
	i = j = 0;
	for (k = 0; k < plen; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
		*pos ^= S[(S[i] + S[j]) & 0xff];
		pos++;
	}
}


static int wlan_crypto_try_wep(const uint8_t *key, size_t key_len,
				const uint8_t *data, size_t data_len,
				uint8_t *plain){
	uint32_t icv, rx_icv;
	uint8_t k[16];
	int i, j;
	int32_t status = -1;

	for (i = 0, j = 0; i < sizeof(k); i++) {
		k[i] = key[j];
		j++;
		if (j >= key_len)
			j = 0;
	}

	qdf_mem_copy(plain, data, data_len);
	wlan_crypto_wep_crypt(k, plain, data_len);
	icv = wlan_crypto_crc32(plain, data_len - 4);
	rx_icv = WPA_GET_LE32(plain + data_len - 4);
	if (icv != rx_icv)
		return status;

	return 0;
}

uint8_t *wlan_crypto_wep_decrypt(const uint8_t *key, uint16_t key_len,
				const struct ieee80211_hdr *hdr,
				const uint8_t *data, size_t data_len,
				size_t *decrypted_len){

	uint8_t *plain;
	int found = 0;
	uint8_t k[16];

	if (data_len < 4 + 4)
		return NULL;
	plain = qdf_mem_malloc(data_len - 4);

	if (plain == NULL) {
		qdf_print("%s[%d] mem alloc failed\n", __func__, __LINE__);
		return NULL;
	}

	qdf_mem_copy(k, data, 3);
	qdf_mem_copy(k + 3, key, key_len);

	if (wlan_crypto_try_wep(k, 3 + key_len, data + 4,
			data_len - 4, plain)  == 0)
		found = 1;

	if (!found) {
		qdf_mem_free(plain);
		return NULL;
	}

	*decrypted_len = data_len - 4 - 4;
	return plain;
}

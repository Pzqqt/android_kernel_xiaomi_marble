/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 */
/*
 * One-key CBC MAC (OMAC1) hash with AES
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include "wlan_crypto_aes_i.h"
#include "wlan_crypto_def_i.h"

static void gf_mulx(uint8_t *pad)
{
	int i, carry;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}


/**
 * omac1_aes_vector - One-Key CBC MAC (OMAC1) hash with AES
 * @key: Key for the hash operation
 * @key_len: Key length in octets
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_vector(const uint8_t *key, size_t key_len, size_t num_elem,
		     const uint8_t *addr[], const size_t *len, uint8_t *mac)
{
	void *ctx;
	const uint8_t *pos, *end;
	int32_t status = -1;
	size_t i, e, left, total_len;
	uint8_t cbc[AES_BLOCK_SIZE], pad[AES_BLOCK_SIZE];


	ctx = wlan_crypto_aes_encrypt_init(key, key_len);
	if (ctx == NULL)
		return status;

	total_len = 0;
	for (e = 0; e < num_elem; e++)
		total_len += len[e];
	left = total_len;

	qdf_mem_set(cbc, AES_BLOCK_SIZE, 0);

	e = 0;
	pos = addr[0];
	end = pos + len[0];

	while (left >= AES_BLOCK_SIZE) {
		for (i = 0; i < AES_BLOCK_SIZE; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				/*
				 * Stop if there are no more bytes to process
				 * since there are no more entries in the array.
				 */
				if (i + 1 == AES_BLOCK_SIZE &&
				    left == AES_BLOCK_SIZE)
					break;
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		if (left > AES_BLOCK_SIZE)
			wlan_crypto_aes_encrypt(ctx, cbc, cbc);
		left -= AES_BLOCK_SIZE;
	}

	qdf_mem_set(pad, AES_BLOCK_SIZE, 0);
	wlan_crypto_aes_encrypt(ctx, pad, pad);
	gf_mulx(pad);

	if (left || total_len == 0) {
		for (i = 0; i < left; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				/*
				 * Stop if there are no more bytes to process
				 * since there are no more entries in the array.
				 */
				if (i + 1 == left)
					break;
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		cbc[left] ^= 0x80;
		gf_mulx(pad);
	}

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		pad[i] ^= cbc[i];
	wlan_crypto_aes_encrypt(ctx, pad, mac);
	wlan_crypto_aes_encrypt_deinit(ctx);
	return 0;
}


/**
 * omac1_aes_128_vector - One-Key CBC MAC (OMAC1) hash with AES-128
 * @key: 128-bit key for the hash operation
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_128_vector(const uint8_t *key, size_t num_elem,
			 const uint8_t *addr[], const size_t *len, uint8_t *mac)
{
	return omac1_aes_vector(key, 16, num_elem, addr, len, mac);
}


/**
 * omac1_aes_128 - One-Key CBC MAC (OMAC1) hash with AES-128 (aka AES-CMAC)
 * @key: 128-bit key for the hash operation
 * @data: Data buffer for which a MAC is determined
 * @data_len: Length of data buffer in bytes
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_128(const uint8_t *key, const uint8_t *data,
			size_t data_len, uint8_t *mac){
	return omac1_aes_128_vector(key, 1, &data, &data_len, mac);
}


/**
 * omac1_aes_256 - One-Key CBC MAC (OMAC1) hash with AES-256 (aka AES-CMAC)
 * @key: 256-bit key for the hash operation
 * @data: Data buffer for which a MAC is determined
 * @data_len: Length of data buffer in bytes
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 */
int omac1_aes_256(const uint8_t *key, const uint8_t *data,
			size_t data_len, uint8_t *mac){
	return omac1_aes_vector(key, 32, 1, &data, &data_len, mac);
}

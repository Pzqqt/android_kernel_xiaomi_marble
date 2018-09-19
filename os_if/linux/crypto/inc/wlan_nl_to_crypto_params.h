/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_nl_to_crypto_params.h
 *
 * Conversion of NL param type to Crypto param type APIs implementation
 *
 */

/**
 * osif_nl_to_crypto_auth_type() - populate auth type for crypto
 * @auth_type: NL auth_type
 *
 * set the crypto auth type for corresponding auth type received
 * from NL
 *
 * Return: crypto auth type, negative value for failure
 */
int osif_nl_to_crypto_auth_type(enum nl80211_auth_type auth_type);

/**
 * osif_nl_to_crypto_akm_type() - populate akm type for crypto
 * @akm_type: NL akm_type
 *
 * set the crypto akm type for corresponding akm type received
 * from NL
 *
 * Return: crypto akm type, negative value for failure
 */
int osif_nl_to_crypto_akm_type(u32 key_mgmt);

/**
 * osif_nl_to_crypto_cipher_type() - populate cipher type for crypto
 * @cipher: NL cipher type
 *
 * set the crypto cipher type for corresponding cipher type received
 * from NL.
 *
 * Return: crypto cipher type
 */
enum wlan_crypto_cipher_type osif_nl_to_crypto_cipher_type(u32 cipher);

/**
 * osif_nl_to_crypto_cipher_len() - return the cipher length
 * @cipher: NL cipher type
 *
 * Check the cipher type and return the corresponding length
 *
 * Return: crypto cipher length, negative value for failure
 */
int osif_nl_to_crypto_cipher_len(u32 cipher);

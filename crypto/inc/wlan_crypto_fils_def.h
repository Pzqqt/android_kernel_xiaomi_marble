/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_CRYPTO_FILS_DEF_H_
#define _WLAN_CRYPTO_FILS_DEF_H_

/* Element ID Extension (EID 255) values */
#define WLAN_ELEMID_EXT_ASSOC_DELAY_INFO       (1)
#define WLAN_ELEMID_EXT_FILS_REQ_PARAMS        (2)
#define WLAN_ELEMID_EXT_FILS_KEY_CONFIRM       (3)
#define WLAN_ELEMID_EXT_FILS_SESSION           (4)
#define WLAN_ELEMID_EXT_FILS_HLP_CONTAINER     (5)
#define WLAN_ELEMID_EXT_FILS_IP_ADDR_ASSIGN    (6)
#define WLAN_ELEMID_EXT_KEY_DELIVERY           (7)
#define WLAN_ELEMID_EXT_FILS_WRAPPED_DATA      (8)
#define WLAN_ELEMID_EXT_FILS_PUBLIC_KEY        (12)
#define WLAN_ELEMID_EXT_FILS_NONCE             (13)

#define WLAN_MAX_WPA_KEK_LEN                   (64)
#define WLAN_FILS_NONCE_LEN                    (16)

/* FILS AAD Crypto key data */
struct wlan_crypto_fils_aad_key {
	/* FILS aad ANounce */
	uint8_t    a_nonce[WLAN_FILS_NONCE_LEN];
	/* FILS aad SNounce */
	uint8_t    s_nonce[WLAN_FILS_NONCE_LEN];
	/* FILS aad kek */
	uint8_t    kek[WLAN_MAX_WPA_KEK_LEN];
	/* FILS aad kek length */
	uint32_t   kek_len;
};
#endif /* end of _WLAN_CRYPTO_FILS_DEF_H_ */


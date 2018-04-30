/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 */
/*
 * AES (Rijndael) cipher
 * Copyright (c) 2003-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WLAN_CRYPTO_AES_I_H
#define WLAN_CRYPTO_AES_I_H

#include <wlan_cmn.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include "wlan_crypto_global_def.h"


#define CCMP_IV_SIZE (WLAN_CRYPTO_IV_LEN + WLAN_CRYPTO_KEYID_LEN \
				+ WLAN_CRYPTO_EXT_IV_LEN)

#define AES_BLOCK_SIZE 16
#define wpa_printf(args...) do { } while (0)
#define wpa_hexdump(l, t, b, le) do { } while (0)
#define wpa_hexdump_buf(l, t, b) do { } while (0)
#define wpa_hexdump_key(l, t, b, le) do { } while (0)
#define wpa_hexdump_buf_key(l, t, b) do { } while (0)
#define wpa_hexdump_ascii(l, t, b, le) do { } while (0)
#define wpa_hexdump_ascii_key(l, t, b, le) do { } while (0)
/* #define FULL_UNROLL */
#define AES_SMALL_TABLES

extern const uint32_t Te0[256];
extern const uint32_t Te1[256];
extern const uint32_t Te2[256];
extern const uint32_t Te3[256];
extern const uint32_t Te4[256];
extern const uint32_t Td0[256];
extern const uint32_t Td1[256];
extern const uint32_t Td2[256];
extern const uint32_t Td3[256];
extern const uint32_t Td4[256];
extern const uint32_t rcon[10];
extern const uint8_t Td4s[256];
extern const uint8_t rcons[10];

#ifndef AES_SMALL_TABLES

#define RCON(i) rcon[(i)]
static inline uint32_t rotr(uint32_t val, int bits)
{
	return (val >> bits) | (val << (32 - bits));
}

#define TE0(i) Te0[((i) >> 24) & 0xff]
#define TE1(i) Te1[((i) >> 16) & 0xff]
#define TE2(i) Te2[((i) >> 8) & 0xff]
#define TE3(i) Te3[(i) & 0xff]
#define TE41(i) (Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE42(i) (Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) (Te4[(i) & 0xff] & 0x000000ff)
#define TE421(i) (Te4[((i) >> 16) & 0xff] & 0xff000000)
#define TE432(i) (Te4[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (Te4[(i) & 0xff] & 0x0000ff00)
#define TE414(i) (Te4[((i) >> 24) & 0xff] & 0x000000ff)
#define TE411(i) (Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE422(i) (Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) (Te4[(i) & 0xff] & 0x000000ff)
#define TE4(i) (Te4[(i)] & 0x000000ff)

#define TD0(i) Td0[((i) >> 24) & 0xff]
#define TD1(i) Td1[((i) >> 16) & 0xff]
#define TD2(i) Td2[((i) >> 8) & 0xff]
#define TD3(i) Td3[(i) & 0xff]
#define TD41(i) (Td4[((i) >> 24) & 0xff] & 0xff000000)
#define TD42(i) (Td4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TD43(i) (Td4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TD44(i) (Td4[(i) & 0xff] & 0x000000ff)
#define TD0_(i) Td0[(i) & 0xff]
#define TD1_(i) Td1[(i) & 0xff]
#define TD2_(i) Td2[(i) & 0xff]
#define TD3_(i) Td3[(i) & 0xff]

#else /* AES_SMALL_TABLES */

#define RCON(i) (rcons[(i)] << 24)

static inline uint32_t rotr(uint32_t val, int bits)
{
	return (val >> bits) | (val << (32 - bits));
}

#define TE0(i) Te0[((i) >> 24) & 0xff]
#define TE1(i) rotr(Te0[((i) >> 16) & 0xff], 8)
#define TE2(i) rotr(Te0[((i) >> 8) & 0xff], 16)
#define TE3(i) rotr(Te0[(i) & 0xff], 24)
#define TE41(i) ((Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE42(i) (Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) ((Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE421(i) ((Te0[((i) >> 16) & 0xff] << 8) & 0xff000000)
#define TE432(i) (Te0[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (Te0[(i) & 0xff] & 0x0000ff00)
#define TE414(i) ((Te0[((i) >> 24) & 0xff] >> 8) & 0x000000ff)
#define TE411(i) ((Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE422(i) (Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) ((Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE4(i) ((Te0[(i)] >> 8) & 0x000000ff)

#define TD0(i) Td0[((i) >> 24) & 0xff]
#define TD1(i) rotr(Td0[((i) >> 16) & 0xff], 8)
#define TD2(i) rotr(Td0[((i) >> 8) & 0xff], 16)
#define TD3(i) rotr(Td0[(i) & 0xff], 24)
#define TD41(i) (Td4s[((i) >> 24) & 0xff] << 24)
#define TD42(i) (Td4s[((i) >> 16) & 0xff] << 16)
#define TD43(i) (Td4s[((i) >> 8) & 0xff] << 8)
#define TD44(i) (Td4s[(i) & 0xff])
#define TD0_(i) Td0[(i) & 0xff]
#define TD1_(i) rotr(Td0[(i) & 0xff], 8)
#define TD2_(i) rotr(Td0[(i) & 0xff], 16)
#define TD3_(i) rotr(Td0[(i) & 0xff], 24)

#endif /* AES_SMALL_TABLES */

#ifdef _MSC_VER

#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
#define GETU32(p) SWAP(*((uint32_t *)(p)))
#define PUTU32(ct, st) { *((uint32_t *)(ct)) = SWAP((st)); }
#else
#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)

#define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^\
		    ((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
#define PUTU32(ct, st) {\
			(ct)[0] = (u8)((st) >> 24);\
			(ct)[1] = (u8)((st) >> 16);\
			(ct)[2] = (u8)((st) >>  8);\
			(ct)[3] = (u8)(st); }
#endif

#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)

#define WLAN_ALEN               (6)

struct ieee80211_hdr {
	uint8_t frame_control[2];
	uint8_t duration_id[2];
	uint8_t addr1[WLAN_ALEN];
	uint8_t addr2[WLAN_ALEN];
	uint8_t addr3[WLAN_ALEN];
	uint8_t seq_ctrl[2];
} __packed;

struct ieee80211_hdr_addr4 {
	uint8_t frame_control[2];
	uint8_t duration_id[2];
	uint8_t addr1[WLAN_ALEN];
	uint8_t addr2[WLAN_ALEN];
	uint8_t addr3[WLAN_ALEN];
	uint8_t seq_ctrl[2];
	uint8_t addr4[WLAN_ALEN];
} __packed;

struct ieee80211_hdr_qos {
	uint8_t frame_control[2];
	uint8_t duration_id[2];
	uint8_t addr1[WLAN_ALEN];
	uint8_t addr2[WLAN_ALEN];
	uint8_t addr3[WLAN_ALEN];
	uint8_t seq_ctrl[2];
	uint8_t qos[2];
} __packed;

struct ieee80211_hdr_qos_addr4 {
	uint8_t frame_control[2];
	uint8_t duration_id[2];
	uint8_t addr1[WLAN_ALEN];
	uint8_t addr2[WLAN_ALEN];
	uint8_t addr3[WLAN_ALEN];
	uint8_t seq_ctrl[2];
	uint8_t addr4[WLAN_ALEN];
	uint8_t qos[2];
} __packed;

int wlan_crypto_rijndaelKeySetupEnc(uint32_t rk[], const uint8_t cipherKey[],
					int keyBits);

uint8_t *wlan_crypto_ccmp_encrypt(const uint8_t *key, uint8_t *frame,
					size_t len, size_t hdrlen);

uint8_t *wlan_crypto_ccmp_decrypt(const uint8_t *key,
				const struct ieee80211_hdr *hdr,
				uint8_t *data, size_t data_len);

uint8_t *wlan_crypto_tkip_encrypt(const uint8_t *key, uint8_t *frame,
					size_t len, size_t hdrlen);

uint8_t *wlan_crypto_tkip_decrypt(const uint8_t *key,
				const struct ieee80211_hdr *hdr,
				uint8_t *data, size_t data_len);

uint8_t *wlan_crypto_wep_encrypt(const uint8_t *key, uint16_t key_len,
				uint8_t *data, size_t data_len);
uint8_t *wlan_crypto_wep_decrypt(const uint8_t *key, uint16_t key_len,
				uint8_t *data, size_t data_len);

void wlan_crypto_wep_crypt(uint8_t *key, uint8_t *buf, size_t plen);

uint32_t wlan_crypto_crc32(const uint8_t *frame, size_t frame_len);

int wlan_crypto_aes_gcm_ae(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len,
			const uint8_t *plain, size_t plain_len,
			const uint8_t *aad, size_t aad_len,
			uint8_t *crypt, uint8_t *tag);

int wlan_crypto_aes_gcm_ad(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len,
			const uint8_t *crypt, size_t crypt_len,
			const uint8_t *aad, size_t aad_len,
			const uint8_t *tag,
			uint8_t *plain);

int wlan_crypto_aes_gmac(const uint8_t *key, size_t key_len,
			const uint8_t *iv, size_t iv_len,
			const uint8_t *aad, size_t aad_len, uint8_t *tag);
int wlan_crypto_aes_ccm_ae(const uint8_t *key, size_t key_len,
			const uint8_t *nonce, size_t M, const uint8_t *plain,
			size_t plain_len, const uint8_t *aad, size_t aad_len,
			uint8_t *crypt, uint8_t *auth);

int wlan_crypto_aes_ccm_ad(const uint8_t *key, size_t key_len,
			const uint8_t *nonce, size_t M, const uint8_t *crypt,
			size_t crypt_len, const uint8_t *aad, size_t aad_len,
			const uint8_t *auth, uint8_t *plain);

void *wlan_crypto_aes_encrypt_init(const uint8_t *key, size_t len);
void wlan_crypto_aes_encrypt(void *ctx, const uint8_t *plain, uint8_t *crypt);
void wlan_crypto_aes_encrypt_deinit(void *ctx);
void *wlan_crypto_aes_decrypt_init(const uint8_t *key, size_t len);
void wlan_crypto_aes_decrypt(void *ctx, const uint8_t *crypt, uint8_t *plain);
void wlan_crypto_aes_decrypt_deinit(void *ctx);
int omac1_aes_128(const uint8_t *key, const uint8_t *data,
				size_t data_len, uint8_t *mac);
int omac1_aes_256(const uint8_t *key, const uint8_t *data,
				size_t data_len, uint8_t *mac);
int omac1_aes_vector(const uint8_t *key, size_t key_len, size_t num_elem,
		     const uint8_t *addr[], const size_t *len, uint8_t *mac);
#endif /* WLAN_CRYPTO_AES_I_H */

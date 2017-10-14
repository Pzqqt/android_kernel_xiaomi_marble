/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

/*============================================================================
   FILE:         cds_utils.c

   OVERVIEW:     This source file contains definitions for CDS crypto APIs
   The four APIs mentioned in this file are used for
   initializing, and de-initializing a crypto context, and
   obtaining truly random data (for keys), as well as
   SHA1 HMAC, and AES encrypt and decrypt routines.

   The routines include:
   cds_crypto_init() - Initializes Crypto module
   cds_crypto_deinit() - De-initializes Crypto module
   cds_rand_get_bytes() - Generates random byte

   DEPENDENCIES:
   ============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#include "qdf_trace.h"
#include "cds_utils.h"
#include "qdf_mem.h"
#include "cds_crypto.h"

#include <linux/err.h>
#include <linux/random.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/completion.h>
#include <linux/ieee80211.h>
#include <crypto/hash.h>
#include <crypto/aes.h>

#include "cds_ieee80211_common.h"
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define AAD_LEN 20
#define IV_SIZE_AES_128 16
#define CMAC_IPN_LEN 6
#define CMAC_TLEN 8             /* CMAC TLen = 64 bits (8 octets) */

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
   Function Definitions and Documentation
 * -------------------------------------------------------------------------*/
#ifdef WLAN_FEATURE_11W
static inline void xor_128(const u8 *a, const u8 *b, u8 *out)
{
	u8 i;

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		out[i] = a[i] ^ b[i];
}

static inline void leftshift_onebit(const u8 *input, u8 *output)
{
	int i, overflow = 0;

	for (i = (AES_BLOCK_SIZE - 1); i >= 0; i--) {
		output[i] = input[i] << 1;
		output[i] |= overflow;
		overflow = (input[i] & 0x80) ? 1 : 0;
	}
	return;
}

static void generate_subkey(struct crypto_cipher *tfm, u8 *k1, u8 *k2)
{
	u8 l[AES_BLOCK_SIZE], tmp[AES_BLOCK_SIZE];
	u8 const_rb[AES_BLOCK_SIZE] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
	};
	u8 const_zero[AES_BLOCK_SIZE] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	crypto_cipher_encrypt_one(tfm, l, const_zero);

	if ((l[0] & 0x80) == 0) {       /* If MSB(l) = 0, then k1 = l << 1 */
		leftshift_onebit(l, k1);
	} else {                /* Else k1 = ( l << 1 ) (+) Rb */
		leftshift_onebit(l, tmp);
		xor_128(tmp, const_rb, k1);
	}

	if ((k1[0] & 0x80) == 0) {
		leftshift_onebit(k1, k2);
	} else {
		leftshift_onebit(k1, tmp);
		xor_128(tmp, const_rb, k2);
	}
}

static inline void padding(u8 *lastb, u8 *pad, u16 length)
{
	u8 j;

	/* original last block */
	for (j = 0; j < AES_BLOCK_SIZE; j++) {
		if (j < length)
			pad[j] = lastb[j];
		else if (j == length)
			pad[j] = 0x80;
		else
			pad[j] = 0x00;
	}
}

static void cds_cmac_calc_mic(struct crypto_cipher *tfm,
		u8 *m, u16 length, u8 *mac)
{
	u8 x[AES_BLOCK_SIZE], y[AES_BLOCK_SIZE];
	u8 m_last[AES_BLOCK_SIZE], padded[AES_BLOCK_SIZE];
	u8 k1[AES_KEYSIZE_128], k2[AES_KEYSIZE_128];
	int cmpBlk;
	int i, nBlocks = (length + 15) / AES_BLOCK_SIZE;

	generate_subkey(tfm, k1, k2);

	if (nBlocks == 0) {
		nBlocks = 1;
		cmpBlk = 0;
	} else {
		cmpBlk = ((length % AES_BLOCK_SIZE) == 0) ? 1 : 0;
	}

	if (cmpBlk) {           /* Last block is complete block */
		xor_128(&m[AES_BLOCK_SIZE * (nBlocks - 1)], k1, m_last);
	} else {                /* Last block is not complete block */
		padding(&m[AES_BLOCK_SIZE * (nBlocks - 1)], padded,
			length % AES_BLOCK_SIZE);
		xor_128(padded, k2, m_last);
	}

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		x[i] = 0;

	for (i = 0; i < (nBlocks - 1); i++) {
		xor_128(x, &m[AES_BLOCK_SIZE * i], y);  /* y = Mi (+) x */
		crypto_cipher_encrypt_one(tfm, x, y);   /* x = AES-128(KEY, y) */
	}

	xor_128(x, m_last, y);
	crypto_cipher_encrypt_one(tfm, x, y);

	memcpy(mac, x, CMAC_TLEN);
}
#endif

/*--------------------------------------------------------------------------

   \brief cds_crypto_init() - Initializes Crypto module

   The cds_crypto_init() function initializes Crypto module.

   \param phCryptProv - pointer to the Crypt handle

   \return QDF_STATUS_SUCCESS - Successfully generated random memory.

   QDF_STATUS_E_FAULT  - pbBuf is an invalid pointer.

   QDF_STATUS_E_FAILURE - default return value if it fails due to
   unknown reasons

   ***QDF_STATUS_E_RESOURCES - System resources (other than memory)
   are unavailable
   \sa

    ( *** return value not considered yet )
   --------------------------------------------------------------------------*/
QDF_STATUS cds_crypto_init(uint32_t *phCryptProv)
{
	QDF_STATUS uResult = QDF_STATUS_E_FAILURE;

	/* This implementation doesn't require a crypto context */
	*phCryptProv = 0;
	uResult = QDF_STATUS_SUCCESS;
	return uResult;
}

QDF_STATUS cds_crypto_deinit(uint32_t hCryptProv)
{
	QDF_STATUS uResult = QDF_STATUS_E_FAILURE;

	/* CryptReleaseContext succeeded */
	uResult = QDF_STATUS_SUCCESS;

	return uResult;
}

/*--------------------------------------------------------------------------

   \brief cds_rand_get_bytes() - Generates random byte

   The cds_rand_get_bytes() function generate random bytes.

   Buffer should be allocated before calling cds_rand_get_bytes().

   Attempting to initialize an already initialized lock results in
   a failure.

   \param lock - pointer to the opaque lock object to initialize

   \return QDF_STATUS_SUCCESS - Successfully generated random memory.

   QDF_STATUS_E_FAULT  - pbBuf is an invalid pointer.

   QDF_STATUS_E_FAILURE - default return value if it fails due to
   unknown reasons

  ***QDF_STATUS_E_RESOURCES - System resources (other than memory)
  are unavailable
   \sa

    ( *** return value not considered yet )
   --------------------------------------------------------------------------*/
QDF_STATUS
cds_rand_get_bytes(uint32_t cryptHandle, uint8_t *pbBuf, uint32_t numBytes)
{
	QDF_STATUS uResult = QDF_STATUS_E_FAILURE;

	/* check for invalid pointer */
	if (NULL == pbBuf) {
		uResult = QDF_STATUS_E_FAULT;
		return uResult;
	}

	get_random_bytes(pbBuf, numBytes);
	/* "Random sequence generated." */
	uResult = QDF_STATUS_SUCCESS;
	return uResult;
}

#ifdef WLAN_FEATURE_11W
uint8_t cds_get_mmie_size(void)
{
	return sizeof(struct ieee80211_mmie);
}

/*--------------------------------------------------------------------------

   \brief cds_increase_seq() - Increase the IPN aka Sequence number by one unit

   The cds_increase_seq() function increases the IPN by one unit.

   \param ipn - pointer to the IPN aka Sequence number [6 bytes]

   --------------------------------------------------------------------------*/
static void cds_increase_seq(uint8_t *ipn)
{
	uint64_t value = 0;

	if (ipn) {
		value = (0xffffffffffff) & (*((uint64_t *) ipn));
		value = value + 1;
		qdf_mem_copy(ipn, &value, IEEE80211_MMIE_IPNLEN);
	}
}

/*--------------------------------------------------------------------------

   \brief cds_attach_mmie() - attches the complete MMIE at the end of frame

   The cds_attach_mmie() calculates the entire MMIE and attaches at the end
   of Broadcast/Multicast robust management frames.

   \param igtk - pointer  group key which will be used to calculate
   the 8 byte MIC.
   \param ipn - pointer ipn, it is also known as sequence number
   \param key_id - key identication number
   \param frm - pointer to the start of the frame.
   \param efrm - pointer to the end of the frame.
   \param frmLen - size of the entire frame.

   \return - this function will return true on success and false on
   failure.

   --------------------------------------------------------------------------*/

bool
cds_attach_mmie(uint8_t *igtk, uint8_t *ipn, uint16_t key_id,
		uint8_t *frm, uint8_t *efrm, uint16_t frmLen)
{
	struct ieee80211_mmie *mmie;
	struct ieee80211_frame *wh;
	uint8_t aad[AAD_LEN], mic[CMAC_TLEN], *input = NULL;
	uint8_t previous_ipn[IEEE80211_MMIE_IPNLEN] = { 0 };
	uint16_t nBytes = 0;
	int ret = 0;
	struct crypto_cipher *tfm;

	/*  This is how received frame look like
	 *
	 *        <------------frmLen---------------------------->
	 *
	 *        +---------------+----------------------+-------+
	 *        | 802.11 HEADER | Management framebody | MMIE  |
	 *        +---------------+----------------------+-------+
	 *                                                       ^
	 *                                                       |
	 *                                                      efrm
	 *   This is how MMIE from above frame look like
	 *
	 *
	 *        <------------ 18 Bytes----------------------------->
	 *        +--------+---------+---------+-----------+---------+
	 *        |Element | Length  | Key id  |   IPN     |  MIC    |
	 *        |  id    |         |         |           |         |
	 *        +--------+---------+---------+-----------+---------+
	 * Octet     1         1         2         6            8
	 *
	 */

	/* Check if frame is invalid length */
	if (((efrm - frm) != frmLen) || (frmLen < sizeof(*wh))) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid frame length", __func__);
		return false;
	}
	mmie = (struct ieee80211_mmie *)(efrm - sizeof(*mmie));

	/* Copy Element id */
	mmie->element_id = IEEE80211_ELEMID_MMIE;

	/* Copy Length */
	mmie->length = sizeof(*mmie) - 2;

	/* Copy Key id */
	mmie->key_id = key_id;

	/*
	 * In case of error, revert back to original IPN
	 * to do that copy the original IPN into previous_ipn
	 */
	qdf_mem_copy(&previous_ipn[0], ipn, IEEE80211_MMIE_IPNLEN);
	cds_increase_seq(ipn);
	qdf_mem_copy(mmie->sequence_number, ipn, IEEE80211_MMIE_IPNLEN);

	/*
	 * Calculate MIC and then copy
	 */
	tfm = cds_crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		ret = PTR_ERR(tfm);
		tfm = NULL;
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: crypto_alloc_cipher failed (%d)", __func__, ret);
		goto err_tfm;
	}

	ret = crypto_cipher_setkey(tfm, igtk, AES_KEYSIZE_128);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: crypto_cipher_setkey failed (%d)", __func__,
			  ret);
		goto err_tfm;
	}

	/* Construct AAD */
	wh = (struct ieee80211_frame *)frm;

	/* Generate BIP AAD: FC(masked) || A1 || A2 || A3 */

	/* FC type/subtype */
	aad[0] = wh->i_fc[0];
	/* Mask FC Retry, PwrMgt, MoreData flags to zero */
	aad[1] = wh->i_fc[1] & ~(IEEE80211_FC1_RETRY | IEEE80211_FC1_PWR_MGT |
				 IEEE80211_FC1_MORE_DATA);
	/* A1 || A2 || A3 */
	qdf_mem_copy(aad + 2, wh->i_addr_all, 3 * IEEE80211_ADDR_LEN);

	/* MIC = AES-128-CMAC(IGTK, AAD || Management Frame Body || MMIE, 64) */
	nBytes = AAD_LEN + (frmLen - sizeof(struct ieee80211_frame));
	input = (uint8_t *) qdf_mem_malloc(nBytes);
	if (NULL == input) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Memory allocation failed", __func__);
		ret = QDF_STATUS_E_NOMEM;
		goto err_tfm;
	}

	/*
	 * Copy the AAD, Management frame body, and
	 * MMIE with 8 bit MIC zeroed out
	 */
	qdf_mem_copy(input, aad, AAD_LEN);
	/* Copy Management Frame Body and MMIE without MIC */
	qdf_mem_copy(input + AAD_LEN,
		     (uint8_t *) (efrm -
				  (frmLen - sizeof(struct ieee80211_frame))),
		     nBytes - AAD_LEN - CMAC_TLEN);

	cds_cmac_calc_mic(tfm, input, nBytes, mic);
	qdf_mem_free(input);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "CMAC(T)= %02X %02X %02X %02X %02X %02X %02X %02X",
		  mic[0], mic[1], mic[2], mic[3],
		  mic[4], mic[5], mic[6], mic[7]);
	qdf_mem_copy(mmie->mic, mic, IEEE80211_MMIE_MICLEN);

err_tfm:
	if (ret) {
		qdf_mem_copy(ipn, previous_ipn, IEEE80211_MMIE_IPNLEN);
	}

	if (tfm)
		cds_crypto_free_cipher(tfm);
	return !ret ? true : false;
}

bool
cds_is_mmie_valid(uint8_t *igtk, uint8_t *ipn, uint8_t *frm, uint8_t *efrm)
{
	struct ieee80211_mmie *mmie;
	struct ieee80211_frame *wh;
	uint8_t *rx_ipn, aad[AAD_LEN], mic[CMAC_TLEN], *input;
	uint16_t nBytes = 0;
	int ret = 0;
	struct crypto_cipher *tfm;

	/* Check if frame is invalid length */
	if ((efrm < frm) || ((efrm - frm) < sizeof(*wh))) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Invalid frame length");
		return false;
	}

	mmie = (struct ieee80211_mmie *)(efrm - sizeof(*mmie));

	/* Check Element ID */
	if ((mmie->element_id != IEEE80211_ELEMID_MMIE) ||
	    (mmie->length != (sizeof(*mmie) - 2))) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "IE is not Mgmt MIC IE or Invalid length");
		/* IE is not Mgmt MIC IE or invalid length */
		return false;
	}

	/* Validate IPN */
	rx_ipn = mmie->sequence_number;
	if (OS_MEMCMP(rx_ipn, ipn, CMAC_IPN_LEN) <= 0) {
		/* Replay error */
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Replay error mmie ipn %02X %02X %02X %02X %02X %02X"
			  " drvr ipn %02X %02X %02X %02X %02X %02X",
			  rx_ipn[0], rx_ipn[1], rx_ipn[2], rx_ipn[3], rx_ipn[4],
			  rx_ipn[5], ipn[0], ipn[1], ipn[2], ipn[3], ipn[4],
			  ipn[5]);
		return false;
	}
	tfm = cds_crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		ret = PTR_ERR(tfm);
		tfm = NULL;
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "crypto_alloc_cipher failed (%d)", ret);
		goto err_tfm;
	}

	ret = crypto_cipher_setkey(tfm, igtk, AES_KEYSIZE_128);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "crypto_cipher_setkey failed (%d)", ret);
		goto err_tfm;
	}

	/* Construct AAD */
	wh = (struct ieee80211_frame *)frm;

	/* Generate BIP AAD: FC(masked) || A1 || A2 || A3 */

	/* FC type/subtype */
	aad[0] = wh->i_fc[0];
	/* Mask FC Retry, PwrMgt, MoreData flags to zero */
	aad[1] = wh->i_fc[1] & ~(IEEE80211_FC1_RETRY | IEEE80211_FC1_PWR_MGT |
				 IEEE80211_FC1_MORE_DATA);
	/* A1 || A2 || A3 */
	qdf_mem_copy(aad + 2, wh->i_addr_all, 3 * IEEE80211_ADDR_LEN);

	/* MIC = AES-128-CMAC(IGTK, AAD || Management Frame Body || MMIE, 64) */
	nBytes = AAD_LEN + (efrm - (uint8_t *) (wh + 1));
	input = (uint8_t *) qdf_mem_malloc(nBytes);
	if (NULL == input) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Memory allocation failed");
		ret = QDF_STATUS_E_NOMEM;
		goto err_tfm;
	}

	/* Copy the AAD, MMIE with 8 bit MIC zeroed out */
	qdf_mem_copy(input, aad, AAD_LEN);
	qdf_mem_copy(input + AAD_LEN, (uint8_t *) (wh + 1),
		     nBytes - AAD_LEN - CMAC_TLEN);

	cds_cmac_calc_mic(tfm, input, nBytes, mic);
	qdf_mem_free(input);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "CMAC(T)= %02X %02X %02X %02X %02X %02X %02X %02X",
		  mic[0], mic[1], mic[2], mic[3],
		  mic[4], mic[5], mic[6], mic[7]);

	if (OS_MEMCMP(mic, mmie->mic, CMAC_TLEN) != 0) {
		/* MMIE MIC mismatch */
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "BC/MC MGMT frame MMIE MIC check Failed"
			  " rmic %02X %02X %02X %02X %02X %02X %02X %02X"
			  " cmic %02X %02X %02X %02X %02X %02X %02X %02X",
			  mmie->mic[0], mmie->mic[1], mmie->mic[2],
			  mmie->mic[3], mmie->mic[4], mmie->mic[5],
			  mmie->mic[6], mmie->mic[7], mic[0], mic[1], mic[2],
			  mic[3], mic[4], mic[5], mic[6], mic[7]);
		return false;
	}

	/* Update IPN */
	qdf_mem_copy(ipn, rx_ipn, CMAC_IPN_LEN);

err_tfm:
	if (tfm)
		cds_crypto_free_cipher(tfm);

	return !ret ? true : false;
}

#endif /* WLAN_FEATURE_11W */

uint32_t cds_chan_to_freq(uint8_t chan)
{
	if (chan < CDS_24_GHZ_CHANNEL_14)       /* ch 0 - ch 13 */
		return CDS_24_GHZ_BASE_FREQ + chan * CDS_CHAN_SPACING_5MHZ;
	else if (chan == CDS_24_GHZ_CHANNEL_14) /* ch 14 */
		return CDS_CHAN_14_FREQ;
	else if (chan < CDS_24_GHZ_CHANNEL_27)  /* ch 15 - ch 26 */
		return CDS_CHAN_15_FREQ +
		       (chan - CDS_24_GHZ_CHANNEL_15) * CDS_CHAN_SPACING_20MHZ;
	else if (chan == CDS_5_GHZ_CHANNEL_170)
		return CDS_CHAN_170_FREQ;
	else
		return CDS_5_GHZ_BASE_FREQ + chan * CDS_CHAN_SPACING_5MHZ;
}

uint8_t cds_freq_to_chan(uint32_t freq)
{
	uint8_t chan;

	if (freq > CDS_24_GHZ_BASE_FREQ && freq < CDS_CHAN_14_FREQ)
		chan = ((freq - CDS_24_GHZ_BASE_FREQ) / CDS_CHAN_SPACING_5MHZ);
	else if (freq == CDS_CHAN_14_FREQ)
		chan = CDS_24_GHZ_CHANNEL_14;
	else if ((freq > CDS_24_GHZ_BASE_FREQ) && (freq < CDS_5_GHZ_BASE_FREQ))
		chan = (((freq - CDS_CHAN_15_FREQ) / CDS_CHAN_SPACING_20MHZ) +
			CDS_24_GHZ_CHANNEL_15);
	else
		chan = (freq - CDS_5_GHZ_BASE_FREQ) / CDS_CHAN_SPACING_5MHZ;
	return chan;
}

void cds_upper_to_lower(uint8_t *txt, uint32_t length)
{
	int i;

	for (i = 0; i < length; i++) {
		if (txt[i] >= 'A' && txt[i] <= 'Z')
			txt[i] = txt[i] + 32;
	}
}

enum cds_band_type cds_chan_to_band(uint32_t chan)
{
	if (chan <= CDS_24_GHZ_CHANNEL_14)
		return CDS_BAND_2GHZ;

	return CDS_BAND_5GHZ;
}

void cds_copy_hlp_info(struct qdf_mac_addr *input_dst_mac,
		       struct qdf_mac_addr *input_src_mac,
		       uint16_t input_hlp_data_len,
		       uint8_t *input_hlp_data,
		       struct qdf_mac_addr *output_dst_mac,
		       struct qdf_mac_addr *output_src_mac,
		       uint16_t *output_hlp_data_len,
		       uint8_t *output_hlp_data)
{
	if (!input_hlp_data_len) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Input HLP data len zero\n");
		return;
	}

	qdf_copy_macaddr(output_dst_mac, input_dst_mac);
	qdf_copy_macaddr(output_src_mac, input_src_mac);
	*output_hlp_data_len = input_hlp_data_len;
	qdf_mem_copy(output_hlp_data, input_hlp_data, input_hlp_data_len);
}

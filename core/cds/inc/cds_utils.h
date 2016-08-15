/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#if !defined(__CDS_UTILS_H)
#define __CDS_UTILS_H

/**=========================================================================

   \file  cds_utils.h

   \brief Connectivity driver services (CDS) utility APIs

   Various utility functions

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_event.h>
#include "ani_global.h"

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#define CDS_DIGEST_SHA1_SIZE    (20)
#define CDS_DIGEST_MD5_SIZE     (16)

#define CDS_24_GHZ_BASE_FREQ   (2407)
#define CDS_5_GHZ_BASE_FREQ    (5000)
#define CDS_24_GHZ_CHANNEL_6   (6)
#define CDS_24_GHZ_CHANNEL_14  (14)
#define CDS_24_GHZ_CHANNEL_15  (15)
#define CDS_24_GHZ_CHANNEL_27  (27)
#define CDS_5_GHZ_CHANNEL_170  (170)
#define CDS_CHAN_SPACING_5MHZ  (5)
#define CDS_CHAN_SPACING_20MHZ (20)
#define CDS_CHAN_14_FREQ       (2484)
#define CDS_CHAN_15_FREQ       (2512)
#define CDS_CHAN_170_FREQ      (5852)

#define cds_log(level, args...) QDF_TRACE(QDF_MODULE_ID_QDF, level, ## args)
#define cds_logfl(level, format, args...) cds_log(level, FL(format), ## args)

#define cds_alert(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define cds_err(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define cds_warn(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define cds_notice(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define cds_info(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_INFO_HIGH, format, ## args)
#define cds_debug(format, args...) \
		cds_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

enum cds_band_type {
	CDS_BAND_2GHZ = 1,
	CDS_BAND_5GHZ = 2
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

QDF_STATUS cds_crypto_init(uint32_t *phCryptProv);

QDF_STATUS cds_crypto_deinit(uint32_t hCryptProv);

/**
 * cds_rand_get_bytes

 * FUNCTION:
 * Returns cryptographically secure pseudo-random bytes.
 *
 *
 * @param pbBuf - the caller allocated location where the bytes should be copied
 * @param numBytes the number of bytes that should be generated and
 * copied
 *
 * @return QDF_STATUS_SUCCSS if the operation succeeds
 */
QDF_STATUS cds_rand_get_bytes(uint32_t handle, uint8_t *pbBuf,
			      uint32_t numBytes);

/**
 * cds_sha1_hmac_str
 *
 * FUNCTION:
 * Generate the HMAC-SHA1 of a string given a key.
 *
 * LOGIC:
 * Standard HMAC processing from RFC 2104. The code is provided in the
 * appendix of the RFC.
 *
 * ASSUMPTIONS:
 * The RFC is correct.
 *
 * @param text text to be hashed
 * @param textLen length of text
 * @param key key to use for HMAC
 * @param keyLen length of key
 * @param digest holds resultant SHA1 HMAC (20B)
 *
 * @return QDF_STATUS_SUCCSS if the operation succeeds
 *
 */
QDF_STATUS cds_sha1_hmac_str(uint32_t cryptHandle,      /* Handle */
			     uint8_t *text,    /* pointer to data stream */
			     uint32_t textLen,  /* length of data stream */
			     uint8_t *key,     /* pointer to authentication key */
			     uint32_t keyLen,   /* length of authentication key */
			     uint8_t digest[CDS_DIGEST_SHA1_SIZE]);     /* caller digest to be filled in */

/**
 * cds_md5_hmac_str
 *
 * FUNCTION:
 * Generate the HMAC-MD5 of a string given a key.
 *
 * LOGIC:
 * Standard HMAC processing from RFC 2104. The code is provided in the
 * appendix of the RFC.
 *
 * ASSUMPTIONS:
 * The RFC is correct.
 *
 * @param text text to be hashed
 * @param textLen length of text
 * @param key key to use for HMAC
 * @param keyLen length of key
 * @param digest holds resultant MD5 HMAC (16B)
 *
 * @return QDF_STATUS_SUCCSS if the operation succeeds
 *
 */
QDF_STATUS cds_md5_hmac_str(uint32_t cryptHandle,       /* Handle */
			    uint8_t *text,     /* pointer to data stream */
			    uint32_t textLen,   /* length of data stream */
			    uint8_t *key,      /* pointer to authentication key */
			    uint32_t keyLen,    /* length of authentication key */
			    uint8_t digest[CDS_DIGEST_MD5_SIZE]);       /* caller digest to be filled in */

QDF_STATUS cds_encrypt_aes(uint32_t cryptHandle,        /* Handle */
			   uint8_t *pText,      /* pointer to data stream */
			   uint8_t *Encrypted, uint8_t *pKey);          /* pointer to authentication key */

QDF_STATUS cds_decrypt_aes(uint32_t cryptHandle,        /* Handle */
			   uint8_t *pText,      /* pointer to data stream */
			   uint8_t *pDecrypted, uint8_t *pKey);         /* pointer to authentication key */

uint32_t cds_chan_to_freq(uint8_t chan);
uint8_t cds_freq_to_chan(uint32_t freq);
enum cds_band_type cds_chan_to_band(uint32_t chan);
#ifdef WLAN_FEATURE_11W
bool cds_is_mmie_valid(uint8_t *key, uint8_t *ipn,
		       uint8_t *frm, uint8_t *efrm);
bool cds_attach_mmie(uint8_t *igtk, uint8_t *ipn, uint16_t key_id,
		     uint8_t *frm, uint8_t *efrm, uint16_t frmLen);
uint8_t cds_get_mmie_size(void);
#endif /* WLAN_FEATURE_11W */
QDF_STATUS sme_send_flush_logs_cmd_to_fw(tpAniSirGlobal pMac);
#endif /* #if !defined __CDS_UTILS_H */

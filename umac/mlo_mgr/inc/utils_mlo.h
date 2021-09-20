/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains MLO manager containing util public api's
 */
#ifndef _WLAN_UTILS_MLO_H_
#define _WLAN_UTILS_MLO_H_

#include <wlan_cmn_ieee80211.h>
#include "wlan_mlo_mgr_public_structs.h"
#include <wlan_cm_ucfg_api.h>
#include <wlan_objmgr_vdev_obj.h>
#ifdef WLAN_FEATURE_11BE_MLO

#define FC0_IEEE_MGMT_FRM 0x10
#define FC1_IEEE_MGMT_FRM 0x00

/**
 * util_gen_link_assoc_rsp - Generate link association response
 *
 * @frame: association response frame ptr
 * @len: length of assoc rsp frame
 * @link_addr: link mac addr
 * @new_ie: Generated Link assoc rsp
 *
 * Return: true if vdev is a link vdev, false otherwise
 */
QDF_STATUS
util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t len,
			struct qdf_mac_addr link_addr, uint8_t *new_ie);

/**
 * util_find_mlie - Find the first Multi-Link element or the start of the first
 * Multi-Link element fragment sequence in a given buffer containing elements,
 * if a Multi-Link element or element fragment sequence exists in the given
 * buffer.
 *
 * @buf: Buffer to be searched for the Multi-Link element or the start of the
 * Multi-Link element fragment sequence
 * @buflen: Length of the buffer
 * @mlieseq: Pointer to location where the starting address of the Multi-Link
 * element or Multi-Link element fragment sequence should be updated if found
 * in the given buffer. The value NULL will be updated to this location if the
 * element or element fragment sequence is not found. This should be ignored by
 * the caller if the function returns error.
 * @mlieseqlen: Pointer to location where the total length of the Multi-Link
 * element or Multi-Link element fragment sequence should be updated if found
 * in the given buffer. This should be ignored by the caller if the function
 * returns error, or if the function indicates that the element or element
 * fragment sequence was not found by providing a starting address of NULL.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure
 */
QDF_STATUS
util_find_mlie(uint8_t *buf, qdf_size_t buflen, uint8_t **mlieseq,
	       qdf_size_t *mlieseqlen);

/**
 * util_get_mlie_variant - Get the variant of the given Multi-Link element or
 * element fragment sequence.
 *
 * @mlieseq: Starting address of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @mlieseqlen: Total length of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @variant: Pointer to the location where the value of the variant should be
 * updated. On success, the value should be interpreted by the caller as a
 * member of enum wlan_ml_variant. (This enum is not directly used as an
 * argument, so that non-MLO code that happens to call this function does not
 * need to be aware of the definition of the enum, though such a call would
 * ultimately result in an error). The value should be ignored by the caller if
 * the function returns error.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure
 */
QDF_STATUS
util_get_mlie_variant(uint8_t *mlieseq, qdf_size_t mlieseqlen,
		      int *variant);
#else
static inline QDF_STATUS
util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t len,
			struct qdf_mac_addr link_addr, uint8_t *new_ie)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
util_find_mlie(uint8_t *buf, qdf_size_t buflen, uint8_t **mlieseq,
	       qdf_size_t *mlieseqlen)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
util_get_mlie_variant(uint8_t *mlieseq, qdf_size_t mlieseqlen,
		      int *variant)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_11BE_MLO */
#endif /* _WLAN_UTILS_MLO_H_ */

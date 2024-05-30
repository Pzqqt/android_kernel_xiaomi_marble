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

/**
 * util_gen_link_assoc_req() - Generate link specific assoc request
 * @frame: Pointer to original association request. This should not contain the
 * 802.11 header, and must start from the fixed fields in the association
 * request. This is required due to some caller semantics built into the end to
 * end design.
 * @frame_len: Length of original association request
 * @isreassoc: Whether this is a re-association request
 * @link_addr: Secondary link's MAC address
 * @link_frame: Generated secondary link specific association request. Note that
 * this will start from the 802.11 header (unlike the original association
 * request). This should be ignored in the case of failure.
 * @link_frame_maxsize: Maximum size of generated secondary link specific
 * association request
 * @link_frame_len: Pointer to location where populated length of generated
 * secondary link specific association request should be written. This should be
 * ignored in the case of failure.
 *
 * Generate a link specific logically equivalent association request for the
 * secondary link from the original association request containing a Multi-Link
 * element. This applies to both association and re-association requests.
 * Currently, only two link MLO is supported.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure.
 */
QDF_STATUS
util_gen_link_assoc_req(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len);

/**
 * util_gen_link_assoc_rsp() - Generate link specific assoc response
 * @frame: Pointer to original association response. This should not contain the
 * 802.11 header, and must start from the fixed fields in the association
 * response. This is required due to some caller semantics built into the end to
 * end design.
 * @frame_len: Length of original association response
 * @isreassoc: Whether this is a re-association response
 * @link_addr: Secondary link's MAC address
 * @link_frame: Generated secondary link specific association response. Note
 * that this will start from the 802.11 header (unlike the original association
 * response). This should be ignored in the case of failure.
 * @link_frame_maxsize: Maximum size of generated secondary link specific
 * association response
 * @link_frame_len: Pointer to location where populated length of generated
 * secondary link specific association response should be written. This should
 * be ignored in the case of failure.
 *
 * Generate a link specific logically equivalent association response for the
 * secondary link from the original association response containing a Multi-Link
 * element. This applies to both association and re-association responses.
 * Currently, only two link MLO is supported.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure.
 */
QDF_STATUS
util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len);

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

/**
 * util_get_bvmlie_mldmacaddr - Get the MLD MAC address from a given Basic
 * variant Multi-Link element or element fragment sequence.
 *
 * @mlieseq: Starting address of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @mlieseqlen: Total length of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @mldmacaddrfound: Pointer to the location where a boolean status should be
 * updated indicating whether the MLD MAC address was found or not. This should
 * be ignored by the caller if the function returns error.
 * @linkid: Pointer to the location where the MLD MAC address should be updated.
 * This should be ignored by the caller if the function returns error, or if the
 * function indicates that the MLD MAC address was not found.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure
 */
QDF_STATUS
util_get_bvmlie_mldmacaddr(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			   bool *mldmacaddrfound,
			   struct qdf_mac_addr *mldmacaddr);

/**
 * util_get_bvmlie_primary_linkid - Get the link identifier from a given Basic
 * variant Multi-Link element or element fragment sequence, of the AP that
 * transmits the Multi-Link element/element fragment sequence or the
 * nontransmitted BSSID in the same multiple BSSID set as the AP that transmits
 * the Multi-Link element/element fragment sequence and that is affiliated with
 * the MLD that is described in the Multi-Link element.
 *
 * @mlieseq: Starting address of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @mlieseqlen: Total length of the Multi-Link element or Multi-Link element
 * fragment sequence
 * @linkidfound: Pointer to the location where a boolean status should be
 * updated indicating whether the link identifier was found or not. This should
 * be ignored by the caller if the function returns error.
 * @linkid: Pointer to the location where the value of the link identifier
 * should be updated. This should be ignored by the caller if the function
 * returns error, or if the function indicates that the link identifier was not
 * found.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure
 */
QDF_STATUS
util_get_bvmlie_primary_linkid(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			       bool *linkidfound, uint8_t *linkid);

/**
 * util_get_bvmlie_persta_partner_info() - Get per-STA partner link information
 * @mlie: Starting address of the Multi-Link element
 * @mlielen: Total length of the Multi-Link element
 * @partner_info: Pointer to the location where the partner link information
 * should be updated. This should be ignored by the caller if the function
 * returns error. Note that success will be returned and the number of links in
 * this structure will be reported as 0, if no Link Info is found, or no per-STA
 * profile is found, or if none of the per-STA profiles includes a MAC address
 * in the STA Info field (assuming no errors are encountered).
 *
 * Get partner link information in the per-STA profiles present in a Basic
 * variant Multi-Link element. The partner link information is returned only for
 * those per-STA profiles which have a MAC address in the STA Info field. Also
 * note that the current implementation does not have Multi-Link element
 * fragmentation support - this will be added in an upcoming change after shared
 * helper utilities for the same are available. Similarly, the current
 * implementation does not have support for per-STA profile fragmentation - this
 * may be added once it is introduced in the IEEE802.11be standard.
 *
 * Return: QDF_STATUS_SUCCESS in the case of success, QDF_STATUS value giving
 * the reason for error in the case of failure
 */
QDF_STATUS
util_get_bvmlie_persta_partner_info(uint8_t *mlie, qdf_size_t mlielen,
				    struct mlo_partner_info *partner_info);
#else
static inline QDF_STATUS
util_gen_link_assoc_req(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len)
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

static inline QDF_STATUS
util_get_bvmlie_mldmacaddr(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			   bool *mldmacaddrfound,
			   struct qdf_mac_addr *mldmacaddr)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
util_get_bvmlie_primary_linkid(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			       bool *linkidfound, uint8_t *linkid)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
util_get_bvmlie_persta_partner_info(uint8_t *mlie, qdf_size_t mlielen,
				    struct mlo_partner_info *partner_info)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_11BE_MLO */
#endif /* _WLAN_UTILS_MLO_H_ */

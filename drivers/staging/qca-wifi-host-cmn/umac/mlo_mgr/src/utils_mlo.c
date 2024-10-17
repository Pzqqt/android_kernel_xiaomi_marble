/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains MLO manager util api's
 */
#include <wlan_cmn.h>
#include <wlan_mlo_mgr_sta.h>
#include <wlan_cm_public_struct.h>
#include <wlan_mlo_mgr_main.h>
#include <wlan_cm_api.h>
#include "wlan_scan_api.h"
#include "qdf_types.h"
#include "utils_mlo.h"
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_utility.h"

#ifdef WLAN_FEATURE_11BE_MLO

static uint8_t *util_find_eid(uint8_t eid, uint8_t *frame, qdf_size_t len)
{
	if (!frame)
		return NULL;

	while (len >= MIN_IE_LEN && len >= frame[TAG_LEN_POS] + MIN_IE_LEN) {
		if (frame[ID_POS] == eid)
			return frame;

		len -= frame[TAG_LEN_POS] + MIN_IE_LEN;
		frame += frame[TAG_LEN_POS] + MIN_IE_LEN;
	}

	return NULL;
}

static
uint8_t *util_find_extn_eid(uint8_t eid, uint8_t extn_eid,
			    uint8_t *frame, qdf_size_t len)
{
	if (!frame)
		return NULL;

	while (len > MIN_IE_LEN && len >= frame[TAG_LEN_POS] + MIN_IE_LEN) {
		if ((frame[ID_POS] == eid) &&
		    (frame[ELEM_ID_EXTN_POS] == extn_eid))
			return frame;

		len -= frame[TAG_LEN_POS] + MIN_IE_LEN;
		frame += frame[TAG_LEN_POS] + MIN_IE_LEN;
	}
	return NULL;
}

static
uint8_t *util_parse_multi_link_ctrl(uint8_t *element,
				    qdf_size_t len,
				    qdf_size_t *link_info_len)
{
	qdf_size_t parsed_ie_len = 0;
	struct wlan_ie_multilink *mlie_fixed;
	uint16_t mlcontrol;
	uint16_t presencebm;

	if (!element) {
		mlo_err("Pointer to element is NULL");
		return NULL;
	}

	if (!len) {
		mlo_err("Length is zero");
		return NULL;
	}

	if (len < sizeof(struct wlan_ie_multilink)) {
		mlo_err_rl("Length %zu octets is smaller than required for the fixed portion of Multi-Link element (%zu octets)",
			   len, sizeof(struct wlan_ie_multilink));
		return NULL;
	}

	mlie_fixed = (struct wlan_ie_multilink *)element;
	mlcontrol = le16toh(mlie_fixed->mlcontrol);
	presencebm = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_PBM_IDX,
				  WLAN_ML_CTRL_PBM_BITS);

	parsed_ie_len += sizeof(*mlie_fixed);

	/* Check if MLD MAC address is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_MLDMACADDR_P)
		parsed_ie_len += QDF_MAC_ADDR_SIZE;

	/* Check if Link ID info is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_LINKIDINFO_P)
		parsed_ie_len += WLAN_ML_BV_CINFO_LINKIDINFO_SIZE;

	/* Check if BSS parameter change count is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_BSSPARAMCHANGECNT_P)
		parsed_ie_len += WLAN_ML_BV_CINFO_BSSPARAMCHNGCNT_SIZE;

	/* Check if Medium Sync Delay Info is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_MEDIUMSYNCDELAYINFO_P)
		parsed_ie_len += WLAN_ML_BV_CINFO_MEDMSYNCDELAYINFO_SIZE;

	/* Check if EML cap is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_EMLCAP_P)
		parsed_ie_len += WLAN_ML_BV_CINFO_EMLCAP_SIZE;

	/* Check if MLD cap is present */
	if (presencebm & WLAN_ML_BV_CTRL_PBM_MLDCAP_P)
		parsed_ie_len += WLAN_ML_BV_CINFO_MLDCAP_SIZE;

	if (link_info_len) {
		*link_info_len = len - parsed_ie_len;
		mlo_debug("link_info_len:%zu, parsed_ie_len:%zu",
			  *link_info_len, parsed_ie_len);
	}

	return &element[parsed_ie_len];
}

static
uint8_t *util_parse_bvmlie_perstaprofile(uint8_t *subelement,
					 qdf_size_t len,
					 bool is_staprof_reqd,
					 qdf_size_t *staprof_len,
					 uint8_t *linkid,
					 bool *is_macaddr_valid,
					 struct qdf_mac_addr *macaddr)
{
	qdf_size_t subelement_len = 0;
	struct wlan_ml_bv_linfo_perstaprof *perstaprof_fixed;
	uint16_t stacontrol;
	uint8_t completeprofile;
	uint8_t nstrlppresent;
	enum wlan_ml_bv_linfo_perstaprof_stactrl_nstrbmsz nstrbmsz;

	if (!subelement) {
		mlo_err("Pointer to subelement is NULL");
		return NULL;
	}

	if (!len) {
		mlo_err("Length is zero");
		return NULL;
	}

	if (subelement[0] != WLAN_ML_BV_LINFO_SUBELEMID_PERSTAPROFILE) {
		mlo_err_rl("Pointer to subelement does not point to per-STA profile");
		return NULL;
	}

	if (len < sizeof(struct wlan_ml_bv_linfo_perstaprof)) {
		mlo_err_rl("len %zu octets is smaller than that required for the fixed portion of per-STA profile (%zu octets)",
			   len, sizeof(struct wlan_ml_bv_linfo_perstaprof));
		return NULL;
	}

	perstaprof_fixed = (struct wlan_ml_bv_linfo_perstaprof *)subelement;

	subelement_len = sizeof(*perstaprof_fixed);

	stacontrol = le16toh(perstaprof_fixed->stacontrol);

	if (linkid) {
		*linkid = QDF_GET_BITS(stacontrol,
				       WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_LINKID_IDX,
				       WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_LINKID_BITS);
	}

	/* Check if this a complete profile */
	completeprofile = QDF_GET_BITS(stacontrol,
				       WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_CMPLTPROF_IDX,
				       WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_CMPLTPROF_BITS);

	/*  We increment the measured length of the per-STA profile by checking
	 *  for the presence of individual fields. We validate this length
	 *  against the total length of the sublement only at the end, except in
	 *  cases where we are actually about to access a given field.
	 */

	if (is_macaddr_valid)
		*is_macaddr_valid = false;

	/* Check STA MAC address present bit */
	if (QDF_GET_BITS(stacontrol,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_MACADDRP_IDX,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_MACADDRP_BITS)) {
		if (macaddr) {
			/* Explicityly check if the length is sufficient to hold
			 * the STA MAC address, since we are about to attempt to
			 * access the STA MAC address.
			 */
			if (len < (subelement_len + QDF_MAC_ADDR_SIZE)) {
				mlo_err_rl("len %zu octets is smaller than min size of per-STA profile required to accommodate STA MAC address (%zu octets)",
					   len,
					   (subelement_len + QDF_MAC_ADDR_SIZE));
				return NULL;
			}

			qdf_mem_copy(macaddr->bytes,
				     subelement + subelement_len,
				     QDF_MAC_ADDR_SIZE);

			mlo_nofl_debug("Copied MAC address: " QDF_MAC_ADDR_FMT,
				       subelement + subelement_len);

			if (is_macaddr_valid)
				*is_macaddr_valid = true;
		}

		subelement_len += QDF_MAC_ADDR_SIZE;
	}

	/* Check Beacon Interval present bit */
	if (QDF_GET_BITS(stacontrol,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_BCNINTP_IDX,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_BCNINTP_BITS))
		subelement_len += WLAN_BEACONINTERVAL_LEN;

	/* Check DTIM Info present bit */
	if (QDF_GET_BITS(stacontrol,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_DTIMINFOP_IDX,
			 WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_DTIMINFOP_BITS))
		subelement_len +=
			sizeof(struct wlan_ml_bv_linfo_perstaprof_stainfo_dtiminfo);

	/* Check NTSR Link pair present bit */
	nstrlppresent =
		QDF_GET_BITS(stacontrol,
			     WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRLINKPRP_IDX,
			     WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRLINKPRP_BITS);

	if (completeprofile && nstrlppresent) {
		/* Check NTSR Bitmap Size bit */
		nstrbmsz =
			QDF_GET_BITS(stacontrol,
				     WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRBMSZ_IDX,
				     WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRBMSZ_BITS);

		if (nstrbmsz == WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRBMSZ_1_OCTET) {
			subelement_len += 1;
		} else if (nstrbmsz == WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRBMSZ_2_OCTETS) {
			subelement_len += 2;
		} else {
			/* Though an invalid value cannot occur if only 1 bit is
			 * used, we check for it in a generic manner in case the
			 * number of bits is increased in the future.
			 */
			mlo_err_rl("Invalid NSTR Bitmap size %u", nstrbmsz);
			return NULL;
		}
	}

	/* Note: Some implementation versions of hostapd/wpa_supplicant may
	 * provide a per-STA profile without STA profile. Let the caller
	 * indicate whether a STA profile is required to be found. This may be
	 * revisited as upstreaming progresses.
	 */
	if (!is_staprof_reqd) {
		if (len < subelement_len) {
			mlo_err_rl("len %zu < subelement_len %zu",
				   len,
				   subelement_len);
			return NULL;
		}

		return &subelement[subelement_len - 1];
	}

	if (len <= subelement_len) {
		mlo_err_rl("len %zu <= subelement_len %zu",
			   len,
			   subelement_len);
		return NULL;
	}

	if (staprof_len)
		*staprof_len = len - subelement_len;

	return &subelement[subelement_len];
}

static
uint8_t *util_get_successorfrag(uint8_t *currie, uint8_t *frame, qdf_size_t len)
{
	uint8_t *nextie;

	if (!currie || !frame || !len)
		return NULL;

	if ((currie + MIN_IE_LEN) > (frame + len))
		return NULL;

	/* Check whether there is sufficient space in the frame for the current
	 * IE, plus at least another MIN_IE_LEN bytes for the IE header of a
	 * fragment (if present) that would come just after the current IE.
	 */
	if ((currie + MIN_IE_LEN + currie[TAG_LEN_POS] + MIN_IE_LEN) >
			(frame + len))
		return NULL;

	nextie = currie + currie[TAG_LEN_POS] + MIN_IE_LEN;

	/* Check whether there is sufficient space in the frame for the next IE
	 */
	if ((nextie + MIN_IE_LEN + nextie[TAG_LEN_POS]) > (frame + len))
		return NULL;

	if (nextie[ID_POS] != WLAN_ELEMID_FRAGMENT)
		return NULL;

	return nextie;
}

static
QDF_STATUS util_parse_partner_info_from_linkinfo(uint8_t *linkinfo,
						 qdf_size_t linkinfo_len,
						 struct mlo_partner_info *partner_info)
{
	uint8_t linkid;
	struct qdf_mac_addr macaddr;
	bool is_macaddr_valid;
	uint8_t *currpos;
	qdf_size_t currlen;
	uint8_t *endofstainfo;

	if (!linkinfo) {
		mlo_err("linkinfo is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!linkinfo_len) {
		mlo_err("linkinfo_len is zero");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!partner_info) {
		mlo_err("ML partner info is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	partner_info->num_partner_links = 0;
	currpos = linkinfo;
	currlen = linkinfo_len;

	while (currlen) {
		if (currlen <  MIN_IE_LEN) {
			mlo_err_rl("currlen %zu is smaller than minimum IE length %u",
				   currlen, MIN_IE_LEN);
			return QDF_STATUS_E_PROTO;
		}

		if (currlen <  (MIN_IE_LEN + currpos[TAG_LEN_POS])) {
			mlo_err_rl("currlen %zu is smaller than length of current IE %u",
				   currlen, MIN_IE_LEN + currpos[TAG_LEN_POS]);
			return QDF_STATUS_E_PROTO;
		}

		if (currpos[ID_POS] ==
				WLAN_ML_BV_LINFO_SUBELEMID_PERSTAPROFILE) {
			is_macaddr_valid = false;

			/* Per-STA profile fragmentation support may be added
			 * once support for this is introduced in the standard.
			 */
			endofstainfo =
				util_parse_bvmlie_perstaprofile(currpos,
								currlen,
								false,
								NULL,
								&linkid,
								&is_macaddr_valid,
								&macaddr);

			if (!endofstainfo) {
				mlo_err_rl("Error in parsing per-STA profile");
				return QDF_STATUS_E_EMPTY;
			}

			if (is_macaddr_valid) {
				if (partner_info->num_partner_links >=
					QDF_ARRAY_SIZE(partner_info->partner_link_info)) {
					mlo_err_rl("Insufficient size %zu of array for partner link info",
						   QDF_ARRAY_SIZE(partner_info->partner_link_info));
					return QDF_STATUS_E_NOMEM;
				}

				partner_info->partner_link_info[partner_info->num_partner_links].link_id =
					linkid;
				qdf_mem_copy(&partner_info->partner_link_info[partner_info->num_partner_links].link_addr,
					     &macaddr,
					     sizeof(partner_info->partner_link_info[partner_info->num_partner_links].link_addr));

				partner_info->num_partner_links++;
			} else {
				mlo_warn_rl("MAC address not found in STA Info field of per-STA profile with link ID %u",
					    linkid);
			}
		}

		currlen -= (MIN_IE_LEN + currpos[TAG_LEN_POS]);
		currpos += (MIN_IE_LEN + currpos[TAG_LEN_POS]);
	}

	mlo_debug("Number of ML partner links found=%u",
		  partner_info->num_partner_links);

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS util_get_noninheritlists(uint8_t *buff, qdf_size_t buff_len,
				    uint8_t **ninherit_elemlist,
				    qdf_size_t *ninherit_elemlist_len,
				    uint8_t **ninherit_elemextlist,
				    qdf_size_t *ninherit_elemextlist_len)
{
	uint8_t *ninherit_ie;
	qdf_size_t unparsed_len;

	/* Note: This funtionality provided by this helper may be combined with
	 * other, older non-inheritance parsing helper functionality and exposed
	 * as a common API as part of future efforts once the older
	 * functionality can be made generic.
	 */

	if (!buff) {
		mlo_err("Pointer to buffer for IEs is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!buff_len) {
		mlo_err("IE buffer length is zero");
		return QDF_STATUS_E_INVAL;
	}

	if (!ninherit_elemlist) {
		mlo_err("Pointer to Non-Inheritance element ID list array is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!ninherit_elemlist_len) {
		mlo_err("Pointer to Non-Inheritance element ID list array length is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!ninherit_elemextlist) {
		mlo_err("Pointer to Non-Inheritance element ID extension list array is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!ninherit_elemextlist_len) {
		mlo_err("Pointer to Non-Inheritance element ID extension list array length is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	ninherit_ie = NULL;
	*ninherit_elemlist_len = 0;
	*ninherit_elemlist = NULL;
	*ninherit_elemextlist_len = 0;
	*ninherit_elemextlist = NULL;

	ninherit_ie =
		(uint8_t *)util_find_extn_eid(WLAN_ELEMID_EXTN_ELEM,
					      WLAN_EXTN_ELEMID_NONINHERITANCE,
					      buff,
					      buff_len);

	if (ninherit_ie) {
		if ((ninherit_ie + TAG_LEN_POS) > (buff + buff_len - 1)) {
			mlo_err_rl("Position of length field of Non-Inheritance element would exceed IE buffer boundary");
			return QDF_STATUS_E_PROTO;
		}

		if ((ninherit_ie + ninherit_ie[TAG_LEN_POS] + MIN_IE_LEN) >
				(buff + buff_len)) {
			mlo_err_rl("Non-Inheritance element with total length %u would exceed IE buffer boundary",
				   ninherit_ie[TAG_LEN_POS] + MIN_IE_LEN);
			return QDF_STATUS_E_PROTO;
		}

		if ((ninherit_ie[TAG_LEN_POS] + MIN_IE_LEN) <
				MIN_NONINHERITANCEELEM_LEN) {
			mlo_err_rl("Non-Inheritance element size %u is smaller than the minimum required %u",
				   ninherit_ie[TAG_LEN_POS] + MIN_IE_LEN,
				   MIN_NONINHERITANCEELEM_LEN);
			return QDF_STATUS_E_PROTO;
		}

		/* Track the number of unparsed octets, excluding the IE header.
		 */
		unparsed_len = ninherit_ie[TAG_LEN_POS];

		/* Mark the element ID extension as parsed */
		unparsed_len--;

		*ninherit_elemlist_len = ninherit_ie[ELEM_ID_LIST_LEN_POS];
		unparsed_len--;

		/* While checking if the Non-Inheritance element ID list length
		 * exceeds the remaining unparsed IE space, we factor in one
		 * octet for the element extension ID list length and subtract
		 * this from the unparsed IE space.
		 */
		if (*ninherit_elemlist_len > (unparsed_len - 1)) {
			mlo_err_rl("Non-Inheritance element ID list length %zu exceeds remaining unparsed IE space, minus an octet for element extension ID list length %zu",
				   *ninherit_elemlist_len, unparsed_len - 1);

			return QDF_STATUS_E_PROTO;
		}

		if (*ninherit_elemlist_len != 0) {
			*ninherit_elemlist = ninherit_ie + ELEM_ID_LIST_POS;
			unparsed_len -= *ninherit_elemlist_len;
		}

		*ninherit_elemextlist_len =
			ninherit_ie[ELEM_ID_LIST_LEN_POS + *ninherit_elemlist_len + 1];
		unparsed_len--;

		if (*ninherit_elemextlist_len > unparsed_len) {
			mlo_err_rl("Non-Inheritance element ID extension list length %zu exceeds remaining unparsed IE space %zu",
				   *ninherit_elemextlist_len, unparsed_len);

			return QDF_STATUS_E_PROTO;
		}

		if (*ninherit_elemextlist_len != 0) {
			*ninherit_elemextlist = ninherit_ie +
				ELEM_ID_LIST_LEN_POS + (*ninherit_elemlist_len)
				+ 2;
			unparsed_len -= *ninherit_elemextlist_len;
		}

		if (unparsed_len > 0) {
			mlo_err_rl("Unparsed length is %zu, expected 0",
				   unparsed_len);
			return QDF_STATUS_E_PROTO;
		}
	}

	/* If Non-Inheritance element is not found, we still return success,
	 * with the list lengths kept at zero.
	 */
	mlo_debug("Non-Inheritance element ID list array length=%zu",
		  *ninherit_elemlist_len);
	mlo_debug("Non-Inheritance element ID extension list array length=%zu",
		  *ninherit_elemextlist_len);

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS util_eval_ie_in_noninheritlist(uint8_t *ie, qdf_size_t total_ie_len,
					  uint8_t *ninherit_elemlist,
					  qdf_size_t ninherit_elemlist_len,
					  uint8_t *ninherit_elemextlist,
					  qdf_size_t ninherit_elemextlist_len,
					  bool *is_in_noninheritlist)
{
	int i;

	/* Evaluate whether the given IE is in the given Non-Inheritance element
	 * ID list or Non-Inheritance element ID extension list, and update the
	 * result into is_in_noninheritlist. If any list is empty, then the IE
	 * is considered to not be present in that list. Both lists can be
	 * empty.
	 *
	 * If QDF_STATUS_SUCCESS is returned, it means that the evaluation is
	 * successful, and that is_in_noninheritlist contains a valid value
	 * (which could be true or false). If a QDF_STATUS error value is
	 * returned, the value in is_in_noninheritlist is invalid and the caller
	 * should ignore it.
	 */

	/* Note: The funtionality provided by this helper may be combined with
	 * other, older non-inheritance parsing helper functionality and exposed
	 * as a common API as part of future efforts once the older
	 * functionality can be made generic.
	 */

	/* Except for is_in_noninheritlist and ie, other pointer arguments are
	 * permitted to be NULL if they are inapplicable. If they are
	 * applicable, they will be checked to ensure they are not NULL.
	 */

	if (!is_in_noninheritlist) {
		mlo_err("NULL pointer to flag that indicates if element is in a Non-Inheritance list");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* If ninherit_elemlist_len and ninherit_elemextlist_len are both zero
	 * as checked soon in this function, we won't be accessing the IE.
	 * However, we still check right-away if the pointer to the IE is
	 * non-NULL and whether the total IE length is sane enough to access the
	 * element ID and if applicable, the element ID extension, since it
	 * doesn't make sense to set the flag in is_in_noninheritlist for a NULL
	 * IE pointer or an IE whose total length is not sane enough to
	 * distinguish the identity of the IE.
	 */
	if (!ie) {
		mlo_err("NULL pointer to IE");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (total_ie_len < (ID_POS + 1)) {
		mlo_err("Total IE length %zu is smaller than minimum required to access element ID %u",
			total_ie_len, ID_POS + 1);
		return QDF_STATUS_E_INVAL;
	}

	if ((ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
	    (total_ie_len < (IDEXT_POS + 1))) {
		mlo_err("Total IE length %zu is smaller than minimum required to access element ID extension %u",
			total_ie_len, IDEXT_POS + 1);
		return QDF_STATUS_E_INVAL;
	}

	*is_in_noninheritlist = false;

	/* If both the Non-Inheritance element list and Non-Inheritance element
	 * ID extension list are empty, then return success since we can
	 * conclude immediately that the given element does not occur in any
	 * Non-Inheritance list. The is_in_noninheritlist remains set to false
	 * as required.
	 */
	if (!ninherit_elemlist_len && !ninherit_elemextlist_len)
		return QDF_STATUS_SUCCESS;

	if (ie[ID_POS] != WLAN_ELEMID_EXTN_ELEM) {
		if (!ninherit_elemlist_len)
			return QDF_STATUS_SUCCESS;

		if (!ninherit_elemlist) {
			mlo_err("NULL pointer to Non-Inheritance element ID list though length of element ID list is %zu",
				ninherit_elemlist_len);
			return QDF_STATUS_E_NULL_VALUE;
		}

		for (i = 0; i < ninherit_elemlist_len; i++) {
			if (ie[ID_POS] == ninherit_elemlist[i]) {
				*is_in_noninheritlist = true;
				return QDF_STATUS_SUCCESS;
			}
		}
	} else {
		if (!ninherit_elemextlist_len)
			return QDF_STATUS_SUCCESS;

		if (!ninherit_elemextlist) {
			mlo_err("NULL pointer to Non-Inheritance element ID extension list though length of element ID extension list is %zu",
				ninherit_elemextlist_len);
			return QDF_STATUS_E_NULL_VALUE;
		}

		for (i = 0; i < ninherit_elemextlist_len; i++) {
			if (ie[IDEXT_POS] == ninherit_elemextlist[i]) {
				*is_in_noninheritlist = true;
				return QDF_STATUS_SUCCESS;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS util_validate_reportingsta_ie(const uint8_t *reportingsta_ie,
					 const uint8_t *frame_iesection,
					 const qdf_size_t frame_iesection_len)
{
	qdf_size_t reportingsta_ie_size;

	if (!reportingsta_ie) {
		mlo_err("Pointer to reporting STA IE is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame_iesection) {
		mlo_err("Pointer to start of IE section in reporting frame is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame_iesection_len) {
		mlo_err("Length of IE section in reporting frame is zero");
		return QDF_STATUS_E_INVAL;
	}

	if ((reportingsta_ie + ID_POS) > (frame_iesection +
			frame_iesection_len - 1)) {
		mlo_err_rl("Position of element ID field of element for reporting STA would exceed frame IE section boundary");
		return QDF_STATUS_E_PROTO;
	}

	if ((reportingsta_ie + TAG_LEN_POS) > (frame_iesection +
			frame_iesection_len - 1)) {
		mlo_err_rl("Position of length field of element with element ID %u for reporting STA would exceed frame IE section boundary",
			   reportingsta_ie[ID_POS]);
		return QDF_STATUS_E_PROTO;
	}

	if ((reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
	    ((reportingsta_ie + IDEXT_POS) > (frame_iesection +
				frame_iesection_len - 1))) {
		mlo_err_rl("Position of element ID extension field of element would exceed frame IE section boundary");
		return QDF_STATUS_E_PROTO;
	}

	reportingsta_ie_size = reportingsta_ie[TAG_LEN_POS] + MIN_IE_LEN;

	if ((reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
	    (reportingsta_ie_size < (IDEXT_POS + 1))) {
		mlo_err_rl("Total length %zu of element for reporting STA is smaller than minimum required to access element ID extension %u",
			   reportingsta_ie_size, IDEXT_POS + 1);
		return QDF_STATUS_E_PROTO;
	}

	if ((reportingsta_ie[ID_POS] == WLAN_ELEMID_VENDOR) &&
	    (reportingsta_ie_size < (PAYLOAD_START_POS + OUI_LEN))) {
		mlo_err_rl("Total length %zu of element for reporting STA is smaller than minimum required of %u to access vendor EID",
			   reportingsta_ie_size, PAYLOAD_START_POS + OUI_LEN);
		return QDF_STATUS_E_PROTO;
	}

	if ((reportingsta_ie + reportingsta_ie_size) >
			(frame_iesection + frame_iesection_len)) {
		if (reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
			mlo_err_rl("Total size %zu octets of element with element ID %u element ID extension %u for reporting STA would exceed frame IE section boundary",
				   reportingsta_ie_size,
				   reportingsta_ie[ID_POS],
				   reportingsta_ie[IDEXT_POS]);
		} else {
			mlo_err_rl("Total size %zu octets of element with element ID %u for reporting STA would exceed frame IE section boundary",
				   reportingsta_ie_size,
				   reportingsta_ie[ID_POS]);
		}

		return QDF_STATUS_E_PROTO;
	}

	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS util_validate_sta_prof_ie(const uint8_t *sta_prof_ie,
				     const uint8_t *sta_prof_iesection,
				     const qdf_size_t sta_prof_iesection_len)
{
	qdf_size_t sta_prof_ie_size;

	if (!sta_prof_ie) {
		mlo_err("Pointer to STA profile IE is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!sta_prof_iesection) {
		mlo_err("Pointer to start of IE section in STA profile is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!sta_prof_iesection_len) {
		mlo_err("Length of IE section in STA profile is zero");
		return QDF_STATUS_E_INVAL;
	}

	if ((sta_prof_ie + ID_POS) > (sta_prof_iesection +
			sta_prof_iesection_len - 1)) {
		mlo_err_rl("Position of element ID field of STA profile element would exceed STA profile IE section boundary");
		return QDF_STATUS_E_PROTO;
	}

	if ((sta_prof_ie + TAG_LEN_POS) > (sta_prof_iesection +
			sta_prof_iesection_len - 1)) {
		mlo_err_rl("Position of length field of element with element ID %u in STA profile would exceed STA profile IE section boundary",
			   sta_prof_ie[ID_POS]);
		return QDF_STATUS_E_PROTO;
	}

	if ((sta_prof_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
	    ((sta_prof_ie + IDEXT_POS) > (sta_prof_iesection +
				sta_prof_iesection_len - 1))) {
		mlo_err_rl("Position of element ID extension field of element would exceed STA profile IE section boundary");
		return QDF_STATUS_E_PROTO;
	}

	sta_prof_ie_size = sta_prof_ie[TAG_LEN_POS] + MIN_IE_LEN;

	if ((sta_prof_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
	    (sta_prof_ie_size < (IDEXT_POS + 1))) {
		mlo_err_rl("Total length %zu of STA profile element is smaller than minimum required to access element ID extension %u",
			   sta_prof_ie_size, IDEXT_POS + 1);
		return QDF_STATUS_E_PROTO;
	}

	if ((sta_prof_ie + sta_prof_ie_size) >
			(sta_prof_iesection + sta_prof_iesection_len)) {
		if (sta_prof_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
			mlo_err_rl("Total size %zu octets of element with element ID %u element ID extension %u in STA profile would exceed STA profile IE section boundary",
				   sta_prof_ie_size,
				   sta_prof_ie[ID_POS],
				   sta_prof_ie[IDEXT_POS]);
		} else {
			mlo_err_rl("Total size %zu octets of element with element ID %u in STA profile would exceed STA profile IE section boundary",
				   sta_prof_ie_size,
				   sta_prof_ie[ID_POS]);
		}

		return QDF_STATUS_E_PROTO;
	}

	return QDF_STATUS_SUCCESS;
}

#define MLO_LINKSPECIFIC_ASSOC_REQ_FC0  0x00
#define MLO_LINKSPECIFIC_ASSOC_REQ_FC1  0x00
#define MLO_LINKSPECIFIC_ASSOC_RESP_FC0 0x10
#define MLO_LINKSPECIFIC_ASSOC_RESP_FC1 0x00

static
QDF_STATUS util_gen_link_assoc_reqrsp_cmn(uint8_t *frame, qdf_size_t frame_len,
					  uint8_t subtype,
					  struct qdf_mac_addr link_addr,
					  uint8_t *link_frame,
					  qdf_size_t link_frame_maxsize,
					  qdf_size_t *link_frame_len)
{
	/* Please see documentation for util_gen_link_assoc_req() and
	 * util_gen_link_assoc_resp() for information on the inputs to and
	 * output from this helper, since those APIs are essentially wrappers
	 * over this helper.
	 */

	/* Pointer to Multi-Link element */
	uint8_t *mlie;
	/* Total length of Multi-Link element sequence (including fragements if
	 * any)
	 */
	qdf_size_t mlieseqlen;
	/* Variant (i.e. type) of the Multi-Link element */
	enum wlan_ml_variant variant;
	/* Pointer to original copy of Multi-Link element */
	uint8_t *orig_mlie_copy;

	/* Length of the Link Info */
	qdf_size_t link_info_len;

	/* Pointer to the IE section that occurs after the fixed fields in the
	 * original frame for the reporting STA.
	 */
	uint8_t *frame_iesection;
	/* Offset to the start of the IE section in the original frame for the
	 * reporting STA.
	 */
	qdf_size_t frame_iesection_offset;
	/* Total length of the IE section in the original frame for the
	 * reporting STA.
	 */
	qdf_size_t frame_iesection_len;

	/* Pointer to the IEEE802.11 frame header in the link specific frame
	 * being generated for the reported STA.
	 */
	struct wlan_frame_hdr *link_frame_hdr;
	/* Current position in the link specific frame being generated for the
	 * reported STA.
	 */
	uint8_t *link_frame_currpos;
	/* Current length of the link specific frame being generated for the
	 * reported STA.
	 */
	qdf_size_t link_frame_currlen;

	/* Pointer to IE for reporting STA */
	const uint8_t *reportingsta_ie;
	/* Total size of IE for reporting STA, inclusive of the element header
	 */
	qdf_size_t reportingsta_ie_size;

	/* Pointer to current position in STA profile */
	uint8_t *sta_prof_currpos;
	/* Remaining length of STA profile */
	qdf_size_t sta_prof_remlen;
	/* Pointer to start of IE section in STA profile that occurs after fixed
	 * fields.
	 */
	uint8_t *sta_prof_iesection;
	/* Total length of IE section in STA profile */
	qdf_size_t sta_prof_iesection_len;
	/* Pointer to current position being processed in IE section in STA
	 * profile.
	 */
	uint8_t *sta_prof_iesection_currpos;
	/* Remaining length of IE section in STA profile */
	qdf_size_t sta_prof_iesection_remlen;

	/* Pointer to IE in STA profile, that occurs within IE section */
	uint8_t *sta_prof_ie;
	/* Total size of IE in STA profile, inclusive of the element header */
	qdf_size_t sta_prof_ie_size;

	/* Pointer to element ID list in Non-Inheritance IE */
	uint8_t *ninherit_elemlist;
	/* Length of element ID list in Non-Inheritance IE */
	qdf_size_t ninherit_elemlist_len;
	/* Pointer to element ID extension list in Non-Inheritance IE */
	uint8_t *ninherit_elemextlist;
	/* Length of element ID extension list in Non-Inheritance IE */
	qdf_size_t ninherit_elemextlist_len;
	/* Whether a given IE is in a non-inheritance list */
	bool is_in_noninheritlist;

	/* Whether MAC address of reported STA is valid */
	bool is_reportedmacaddr_valid;
	/* MAC address of reported STA */
	struct qdf_mac_addr reportedmacaddr;

	/* Other variables for temporary purposes */
	uint8_t *sub_copy;
	QDF_STATUS ret;

	if (!frame) {
		mlo_err("Pointer to original frame is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame_len) {
		mlo_err("Length of original frame is zero");
		return QDF_STATUS_E_INVAL;
	}

	if ((subtype != WLAN_FC0_STYPE_ASSOC_REQ) &&
	    (subtype != WLAN_FC0_STYPE_REASSOC_REQ) &&
	    (subtype != WLAN_FC0_STYPE_ASSOC_RESP) &&
	    (subtype != WLAN_FC0_STYPE_REASSOC_RESP)) {
		mlo_err("802.11 frame subtype %u is invalid", subtype);
		return QDF_STATUS_E_INVAL;
	}

	if (!link_frame) {
		mlo_err("Pointer to secondary link specific frame is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!link_frame_maxsize) {
		mlo_err("Maximum size of secondary link specific frame is zero");
		return QDF_STATUS_E_INVAL;
	}

	if (!link_frame_len) {
		mlo_err("Pointer to populated length of secondary link specific frame is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	frame_iesection_offset = 0;

	if (subtype == WLAN_FC0_STYPE_ASSOC_REQ) {
		frame_iesection_offset = WLAN_ASSOC_REQ_IES_OFFSET;
	} else if (subtype == WLAN_FC0_STYPE_REASSOC_REQ) {
		frame_iesection_offset = WLAN_REASSOC_REQ_IES_OFFSET;
	} else {
		/* This is a (re)association response */
		frame_iesection_offset = WLAN_ASSOC_RSP_IES_OFFSET;
	}

	if (frame_len < frame_iesection_offset) {
		/* The caller is supposed to have confirmed that this is a valid
		 * frame containing a Multi-Link element. Hence we treat this as
		 * a case of invalid argument being passed to us.
		 */
		mlo_err("Frame length %zu is smaller than the IE section offset %zu for subtype %u",
			frame_len, frame_iesection_offset, subtype);
		return QDF_STATUS_E_INVAL;
	}

	frame_iesection_len = frame_len - frame_iesection_offset;

	if (frame_iesection_len == 0) {
		/* The caller is supposed to have confirmed that this is a valid
		 * frame containing a Multi-Link element. Hence we treat this as
		 * a case of invalid argument being passed to us.
		 */
		mlo_err("No space left in frame for IE section");
		return QDF_STATUS_E_INVAL;
	}

	frame_iesection = frame + frame_iesection_offset;

	mlie = NULL;
	mlieseqlen = 0;

	ret = util_find_mlie(frame_iesection, frame_iesection_len, &mlie,
			     &mlieseqlen);
	if (QDF_IS_STATUS_ERROR(ret))
		return ret;

	if (!mlie) {
		/* The caller is supposed to have confirmed that a Multi-Link
		 * element is present in the frame. Hence we treat this as a
		 * case of invalid argument being passed to us.
		 */
		mlo_err("Invalid original frame since no Multi-Link element found");
		return QDF_STATUS_E_INVAL;
	}

	/* Sanity check the Multi-Link element sequence length */
	if (!mlieseqlen) {
		mlo_err("Length of Multi-Link element sequence is zero. Investigate.");
		return QDF_STATUS_E_FAILURE;
	}

	ret = util_get_mlie_variant(mlie, mlieseqlen, (int *)&variant);
	if (QDF_IS_STATUS_ERROR(ret))
		return ret;

	if (variant != WLAN_ML_VARIANT_BASIC) {
		mlo_err_rl("Unexpected variant %u of Multi-Link element.",
			   variant);
		return QDF_STATUS_E_PROTO;
	}

	/* Note: Multi-Link element fragmentation support will be added in a
	 * later change. As of now, we temporarily return error if the
	 * Multi-Link element sequence length is greater than the max length for
	 * an IE.
	 */
	if (mlieseqlen > (sizeof(struct ie_header) + WLAN_MAX_IE_LEN)) {
		mlo_err_rl("Element fragmentation is not yet supported for this API");
		return QDF_STATUS_E_NOSUPPORT;
	}

	sub_copy = qdf_mem_malloc(mlieseqlen);
	if (!sub_copy) {
		mlo_err_rl("Could not allocate memory for Multi-Link element copy");
		return QDF_STATUS_E_NOMEM;
	}

	orig_mlie_copy = sub_copy;
	qdf_mem_copy(sub_copy, mlie, mlieseqlen);

	sub_copy = util_parse_multi_link_ctrl(sub_copy,
					      mlieseqlen,
					      &link_info_len);

	/* As per the standard, the sender must include Link Info for
	 * association request/response. Throw an error if we are unable to
	 * obtain this.
	 */
	if (!sub_copy) {
		mlo_err_rl("Unable to successfully parse Multi-Link element control and obtain Link Info");
		qdf_mem_free(orig_mlie_copy);
		return QDF_STATUS_E_PROTO;
	}

	mlo_debug("Dumping hex after parsing Multi-Link element control");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_MLO, QDF_TRACE_LEVEL_DEBUG,
			   sub_copy, link_info_len);

	sta_prof_remlen = 0;
	is_reportedmacaddr_valid = false;

	/* Note: We may have a future change to skip subelements which are not
	 * Per-STA Profile, handle more than two links in MLO, handle cases
	 * where we unexpectedly find more Per-STA Profiles than expected, etc.
	 */

	/* Parse per-STA profile */
	sta_prof_currpos =
		util_parse_bvmlie_perstaprofile(sub_copy,
						link_info_len,
						true,
						&sta_prof_remlen,
						NULL,
						&is_reportedmacaddr_valid,
						&reportedmacaddr);

	/* If we do not successfully find a STA Profile, we return an error.
	 * This is because we need to get at least the expected fixed fields,
	 * even if there is an (improbable) total inheritance.
	 */
	if (!sta_prof_currpos) {
		mlo_err_rl("Unable to find STA profile");
		qdf_mem_free(orig_mlie_copy);
		return QDF_STATUS_E_PROTO;
	}

	/* As per the standard, the sender sets the MAC address in the per-STA
	 * profile in association request/response. Without this, we cannot
	 * generate the link specific frame.
	 */
	if (!is_reportedmacaddr_valid) {
		mlo_err_rl("Unable to get MAC address from per-STA profile");
		qdf_mem_free(orig_mlie_copy);
		return QDF_STATUS_E_PROTO;
	}

	link_frame_currpos = link_frame;
	*link_frame_len = 0;
	link_frame_currlen = 0;

	if (link_frame_maxsize < WLAN_MAC_HDR_LEN_3A) {
		mlo_err("Insufficent space in link specific frame for 802.11 header. Required: %u octets, available: %zu octets",
			WLAN_MAC_HDR_LEN_3A, link_frame_maxsize);

		qdf_mem_free(orig_mlie_copy);
		return QDF_STATUS_E_NOMEM;
	}

	link_frame_currpos += WLAN_MAC_HDR_LEN_3A;
	link_frame_currlen += WLAN_MAC_HDR_LEN_3A;

	if ((subtype == WLAN_FC0_STYPE_ASSOC_REQ) ||
	    (subtype == WLAN_FC0_STYPE_REASSOC_REQ)) {
		mlo_debug("Populating fixed fields for (re)assoc req in link specific frame");

		if (sta_prof_remlen < WLAN_CAPABILITYINFO_LEN) {
			mlo_err_rl("Remaining length of STA profile %zu octets is less than length of Capability Info %u",
				   sta_prof_remlen,
				   WLAN_CAPABILITYINFO_LEN);

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_PROTO;
		}

		/* Capability information is specific to the link. Copy this
		 * from the STA profile.
		 */

		if ((link_frame_maxsize - link_frame_currlen) <
				WLAN_CAPABILITYINFO_LEN) {
			mlo_err("Insufficent space in link specific frame for Capability Info field. Required: %u octets, available: %zu octets",
				WLAN_CAPABILITYINFO_LEN,
				(link_frame_maxsize - link_frame_currlen));

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_copy(link_frame_currpos, sta_prof_currpos,
			     WLAN_CAPABILITYINFO_LEN);
		link_frame_currpos += WLAN_CAPABILITYINFO_LEN;
		link_frame_currlen += WLAN_CAPABILITYINFO_LEN;
		mlo_debug("Added Capablity Info field (%u octets) to link specific frame",
			  WLAN_CAPABILITYINFO_LEN);

		sta_prof_currpos += WLAN_CAPABILITYINFO_LEN;
		sta_prof_remlen -= WLAN_CAPABILITYINFO_LEN;

		/* Listen Interval is common between all links. Copy this from
		 * the reporting section of the frame.
		 */

		if ((link_frame_maxsize - link_frame_currlen) <
				WLAN_LISTENINTERVAL_LEN) {
			mlo_err("Insufficent space in link specific frame for Listen Interval field. Required: %u octets, available: %zu octets",
				WLAN_LISTENINTERVAL_LEN,
				(link_frame_maxsize - link_frame_currlen));

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_copy(link_frame_currpos,
			     frame + WLAN_CAPABILITYINFO_LEN,
			     WLAN_LISTENINTERVAL_LEN);
		link_frame_currpos += WLAN_LISTENINTERVAL_LEN;
		link_frame_currlen += WLAN_LISTENINTERVAL_LEN;
		mlo_debug("Added Listen Interval field (%u octets) to link specific frame",
			  WLAN_LISTENINTERVAL_LEN);

		if (subtype == WLAN_FC0_STYPE_REASSOC_REQ) {
			/* Current AP address is common between all links. Copy
			 * this from the reporting section of the frame.
			 */
			if ((link_frame_maxsize - link_frame_currlen) <
				QDF_MAC_ADDR_SIZE) {
				mlo_err("Insufficent space in link specific frame for current AP address. Required: %u octets, available: %zu octets",
					QDF_MAC_ADDR_SIZE,
					(link_frame_maxsize -
						link_frame_currlen));

				qdf_mem_free(orig_mlie_copy);
				return QDF_STATUS_E_NOMEM;
			}

			qdf_mem_copy(link_frame_currpos,
				     frame + WLAN_CAPABILITYINFO_LEN +
						WLAN_LISTENINTERVAL_LEN,
				     QDF_MAC_ADDR_SIZE);
			link_frame_currpos += QDF_MAC_ADDR_SIZE;
			link_frame_currlen += QDF_MAC_ADDR_SIZE;
			mlo_debug("Reassoc req: Added Current AP address field (%u octets) to link specific frame",
				  QDF_MAC_ADDR_SIZE);
		}
	} else {
		/* This is a (re)association response */
		mlo_debug("Populating fixed fields for (re)assoc resp in link specific frame");

		if (sta_prof_remlen <
			(WLAN_CAPABILITYINFO_LEN + WLAN_STATUSCODE_LEN)) {
			mlo_err_rl("Remaining length of STA profile %zu octets is less than length of Capability Info + length of Status Code %u",
				   sta_prof_remlen,
				   WLAN_CAPABILITYINFO_LEN +
					WLAN_STATUSCODE_LEN);

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_PROTO;
		}

		/* Capability information and Status Code are specific to the
		 * link. Copy these from the STA profile.
		 */

		if ((link_frame_maxsize - link_frame_currlen) <
			(WLAN_CAPABILITYINFO_LEN + WLAN_STATUSCODE_LEN)) {
			mlo_err("Insufficent space in link specific frame for Capability Info and Status Code fields. Required: %u octets, available: %zu octets",
				WLAN_CAPABILITYINFO_LEN + WLAN_STATUSCODE_LEN,
				(link_frame_maxsize - link_frame_currlen));

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_copy(link_frame_currpos, sta_prof_currpos,
			     (WLAN_CAPABILITYINFO_LEN + WLAN_STATUSCODE_LEN));
		link_frame_currpos += (WLAN_CAPABILITYINFO_LEN +
						WLAN_STATUSCODE_LEN);
		link_frame_currlen += (WLAN_CAPABILITYINFO_LEN +
				WLAN_STATUSCODE_LEN);
		mlo_debug("Added Capability Info and Status Code fields (%u octets) to link specific frame",
			  WLAN_CAPABILITYINFO_LEN + WLAN_STATUSCODE_LEN);

		sta_prof_currpos += (WLAN_CAPABILITYINFO_LEN +
				WLAN_STATUSCODE_LEN);
		sta_prof_remlen -= (WLAN_CAPABILITYINFO_LEN +
				WLAN_STATUSCODE_LEN);

		/* AID is common between all links. Copy this from the original
		 * frame.
		 */

		if ((link_frame_maxsize - link_frame_currlen) < WLAN_AID_LEN) {
			mlo_err("Insufficent space in link specific frame for AID field. Required: %u octets, available: %zu octets",
				WLAN_AID_LEN,
				(link_frame_maxsize - link_frame_currlen));

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_copy(link_frame_currpos,
			     frame + WLAN_CAPABILITYINFO_LEN +
					WLAN_STATUSCODE_LEN,
			     WLAN_AID_LEN);
		link_frame_currpos += WLAN_AID_LEN;
		link_frame_currlen += WLAN_AID_LEN;
		mlo_debug("Added AID field (%u octets) to link specific frame",
			  WLAN_AID_LEN);
	}

	sta_prof_iesection = sta_prof_currpos;
	sta_prof_iesection_len = sta_prof_remlen;

	/* Populate non-inheritance lists if applicable */
	ninherit_elemlist_len = 0;
	ninherit_elemlist = NULL;
	ninherit_elemextlist_len = 0;
	ninherit_elemextlist = NULL;

	ret = util_get_noninheritlists(sta_prof_iesection,
				       sta_prof_iesection_len,
				       &ninherit_elemlist,
				       &ninherit_elemlist_len,
				       &ninherit_elemextlist,
				       &ninherit_elemextlist_len);
	if (QDF_IS_STATUS_ERROR(ret)) {
		qdf_mem_free(orig_mlie_copy);
		return ret;
	}

	/* Go through IEs of the reporting STA, and those in STA profile, merge
	 * them into link_frame (except for elements in the Non-Inheritance
	 * list).
	 *
	 * Note: Currently, only 2-link MLO is supported here. We may have a
	 * future change to expand to more links.
	 */
	reportingsta_ie = util_find_eid(WLAN_ELEMID_SSID, frame_iesection,
					frame_iesection_len);

	if ((subtype == WLAN_FC0_STYPE_ASSOC_REQ) ||
	    (subtype == WLAN_FC0_STYPE_REASSOC_REQ)) {
		/* Sanity check that the SSID element is present for the
		 * reporting STA. There is no stipulation in the standard for
		 * the STA profile in this regard, so we do not check the STA
		 * profile for the SSID element.
		 */
		if (!reportingsta_ie) {
			mlo_err_rl("SSID element not found for reporting STA for (re)association request.");
			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_PROTO;
		}
	} else {
		/* This is a (re)association response. Sanity check that the
		 * SSID element is present neither for the reporting STA nor in
		 * the STA profile.
		 */
		if (reportingsta_ie) {
			mlo_err_rl("SSID element found for reporting STA for (re)association response. It should not be present.");
			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_PROTO;
		}

		sta_prof_ie = util_find_eid(WLAN_ELEMID_SSID,
					    sta_prof_iesection,
					    sta_prof_iesection_len);

		if (sta_prof_ie) {
			mlo_err_rl("SSID element found in STA profile for (re)association response. It should not be present.");
			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_PROTO;
		}
	}

	reportingsta_ie = reportingsta_ie ? reportingsta_ie : frame_iesection;

	ret = util_validate_reportingsta_ie(reportingsta_ie, frame_iesection,
					    frame_iesection_len);
	if (QDF_IS_STATUS_ERROR(ret)) {
		qdf_mem_free(orig_mlie_copy);
		return ret;
	}

	reportingsta_ie_size = reportingsta_ie[TAG_LEN_POS] + MIN_IE_LEN;

	while (((reportingsta_ie + reportingsta_ie_size) - frame_iesection)
			<= frame_iesection_len) {
		/* Skip Multi-Link element */
		if ((reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) &&
		    (reportingsta_ie[IDEXT_POS] ==
				WLAN_EXTN_ELEMID_MULTI_LINK)) {
			if (((reportingsta_ie + reportingsta_ie_size) -
					frame_iesection) == frame_iesection_len)
				break;

			reportingsta_ie += reportingsta_ie_size;

			ret = util_validate_reportingsta_ie(reportingsta_ie,
							    frame_iesection,
							    frame_iesection_len);
			if (QDF_IS_STATUS_ERROR(ret)) {
				qdf_mem_free(orig_mlie_copy);
				return ret;
			}

			reportingsta_ie_size = reportingsta_ie[TAG_LEN_POS] +
				MIN_IE_LEN;

			continue;
		}

		sta_prof_ie = NULL;
		sta_prof_ie_size = 0;

		if (sta_prof_iesection_len) {
			if (reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
				sta_prof_ie = (uint8_t *)util_find_extn_eid(reportingsta_ie[ID_POS],
									    reportingsta_ie[IDEXT_POS],
									    sta_prof_iesection,
									    sta_prof_iesection_len);
			} else {
				sta_prof_ie = (uint8_t *)util_find_eid(reportingsta_ie[ID_POS],
								       sta_prof_iesection,
								       sta_prof_iesection_len);
			}
		}

		if (!sta_prof_ie) {
			/* IE is present for reporting STA, but not in STA
			 * profile.
			 */

			is_in_noninheritlist = false;

			ret = util_eval_ie_in_noninheritlist((uint8_t *)reportingsta_ie,
							     reportingsta_ie_size,
							     ninherit_elemlist,
							     ninherit_elemlist_len,
							     ninherit_elemextlist,
							     ninherit_elemextlist_len,
							     &is_in_noninheritlist);

			if (QDF_IS_STATUS_ERROR(ret)) {
				qdf_mem_free(orig_mlie_copy);
				return ret;
			}

			if (!is_in_noninheritlist) {
				if ((link_frame_currpos +
						reportingsta_ie_size) <=
					(link_frame + link_frame_maxsize)) {
					qdf_mem_copy(link_frame_currpos,
						     reportingsta_ie,
						     reportingsta_ie_size);

					link_frame_currpos +=
						reportingsta_ie_size;
					link_frame_currlen +=
						reportingsta_ie_size;

					if (reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
						mlo_debug("IE with element ID : %u extension element ID : %u (%zu octets) present for reporting STA but not in STA profile. Copied IE from reporting frame to link specific frame",
							  reportingsta_ie[ID_POS],
							  reportingsta_ie[IDEXT_POS],
							  reportingsta_ie_size);
					} else {
						mlo_debug("IE with element ID : %u (%zu octets) present for reporting STA but not in STA profile. Copied IE from reporting frame to link specific frame",
							  reportingsta_ie[ID_POS],
							  reportingsta_ie_size);
					}
				} else {
					if (reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u extension element ID : %u. Required: %zu octets, available: %zu octets",
							   reportingsta_ie[ID_POS],
							   reportingsta_ie[IDEXT_POS],
							   reportingsta_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);
					} else {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u. Required: %zu octets, available: %zu octets",
							   reportingsta_ie[ID_POS],
							   reportingsta_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);
					}

					qdf_mem_free(orig_mlie_copy);
					return QDF_STATUS_E_NOMEM;
				}
			} else {
				if (reportingsta_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
					mlo_debug("IE with element ID : %u extension element ID : %u (%zu octets) present for reporting STA but not in STA profile. However it is in Non-Inheritance list, hence ignoring.",
						  reportingsta_ie[ID_POS],
						  reportingsta_ie[IDEXT_POS],
						  reportingsta_ie_size);
				} else {
					mlo_debug("IE with element ID : %u (%zu octets) present for reporting STA but not in STA profile. However it is in Non-Inheritance list, hence ignoring.",
						  reportingsta_ie[ID_POS],
						  reportingsta_ie_size);
				}
			}
		} else {
			/* IE is present for reporting STA and also in STA
			 * profile, copy from STA profile and flag the IE in STA
			 * profile as copied (by setting EID field to 0). The
			 * SSID element (with EID 0) is processed first to
			 * enable this. For vendor IE, compare OUI + type +
			 * subType to determine if they are the same IE.
			 */
			/* Note: This may be revisited in a future change, to
			 * adhere to provisions in the standard for multiple
			 * occurrences of a given element ID/extension element
			 * ID.
			 */

			ret = util_validate_sta_prof_ie(sta_prof_ie,
							sta_prof_iesection,
							sta_prof_iesection_len);
			if (QDF_IS_STATUS_ERROR(ret)) {
				qdf_mem_free(orig_mlie_copy);
				return ret;
			}

			sta_prof_ie_size = sta_prof_ie[TAG_LEN_POS] +
				MIN_IE_LEN;

			sta_prof_iesection_remlen =
				sta_prof_iesection_len -
					(sta_prof_ie - sta_prof_iesection);

			if ((reportingsta_ie[ID_POS] == WLAN_ELEMID_VENDOR) &&
			    (sta_prof_iesection_remlen >= MIN_VENDOR_TAG_LEN)) {
				if (!qdf_mem_cmp(reportingsta_ie +
							PAYLOAD_START_POS,
						 sta_prof_ie +
							PAYLOAD_START_POS,
						 OUI_LEN)) {
					/* Same vendor IE, copy from STA profile
					 */
					if ((link_frame_currpos +
							sta_prof_ie_size) <=
						(link_frame +
							link_frame_maxsize)) {
						qdf_mem_copy(link_frame_currpos,
							     sta_prof_ie,
							     sta_prof_ie_size);

						link_frame_currpos +=
							sta_prof_ie_size;
						link_frame_currlen +=
							sta_prof_ie_size;

						mlo_debug("Vendor IE (%zu octets) for reporting STA also present in STA profile. Copied IE from STA profile to link specific frame",
							  sta_prof_ie_size);

						sta_prof_ie[0] = 0;
					} else {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u. Required: %zu octets, available: %zu octets",
							   sta_prof_ie[ID_POS],
							   sta_prof_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);

						qdf_mem_free(orig_mlie_copy);
						return QDF_STATUS_E_NOMEM;
					}
				} else {
					if ((link_frame_currpos +
							reportingsta_ie_size) <=
						(link_frame +
							link_frame_maxsize)) {
						qdf_mem_copy(link_frame_currpos,
							     reportingsta_ie,
							     reportingsta_ie_size);

						link_frame_currpos +=
							reportingsta_ie_size;
						link_frame_currlen +=
							reportingsta_ie_size;

						mlo_debug("Vendor IE (%zu octets) present for reporting STA but not present in STA profile. Copied IE from reporting frame to link specific frame",
							  reportingsta_ie_size);
					} else {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u. Required: %zu octets, available: %zu octets",
							   reportingsta_ie[ID_POS],
							   reportingsta_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);

						qdf_mem_free(orig_mlie_copy);
						return QDF_STATUS_E_NOMEM;
					}
				}
			} else {
				/* Copy IE from STA profile into link specific
				 * frame.
				 */
				if ((link_frame_currpos + sta_prof_ie_size) <=
					(link_frame + link_frame_maxsize)) {
					qdf_mem_copy(link_frame_currpos,
						     sta_prof_ie,
						     sta_prof_ie_size);

					link_frame_currpos += sta_prof_ie_size;
					link_frame_currlen +=
						sta_prof_ie_size;

					if (reportingsta_ie[ID_POS] ==
							WLAN_ELEMID_EXTN_ELEM) {
						mlo_debug("IE with element ID : %u extension element ID : %u (%zu octets) for reporting STA also present in STA profile. Copied IE from STA profile to link specific frame",
							  sta_prof_ie[ID_POS],
							  sta_prof_ie[IDEXT_POS],
							  sta_prof_ie_size);
					} else {
						mlo_debug("IE with element ID : %u (%zu octets) for reporting STA also present in STA profile. Copied IE from STA profile to link specific frame",
							  sta_prof_ie[ID_POS],
							  sta_prof_ie_size);
					}

					sta_prof_ie[0] = 0;
				} else {
					if (sta_prof_ie[ID_POS] ==
							WLAN_ELEMID_EXTN_ELEM) {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u extension element ID : %u. Required: %zu octets, available: %zu octets",
							   sta_prof_ie[ID_POS],
							   sta_prof_ie[IDEXT_POS],
							   sta_prof_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);
					} else {
						mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u. Required: %zu octets, available: %zu octets",
							   sta_prof_ie[ID_POS],
							   sta_prof_ie_size,
							   link_frame_maxsize -
							   link_frame_currlen);
					}

					qdf_mem_free(orig_mlie_copy);
					return QDF_STATUS_E_NOMEM;
				}
			}
		}

		if (((reportingsta_ie + reportingsta_ie_size) -
					frame_iesection) == frame_iesection_len)
			break;

		reportingsta_ie += reportingsta_ie_size;

		ret = util_validate_reportingsta_ie(reportingsta_ie,
						    frame_iesection,
						    frame_iesection_len);
		if (QDF_IS_STATUS_ERROR(ret)) {
			qdf_mem_free(orig_mlie_copy);
			return ret;
		}

		reportingsta_ie_size = reportingsta_ie[TAG_LEN_POS] +
			MIN_IE_LEN;
	}

	/* Go through the remaining unprocessed IEs in STA profile and copy them
	 * to the link specific frame. The processed ones are marked with 0 in
	 * the first octet. The first octet corresponds to the element ID. In
	 * the case of (re)association request, the element with actual ID
	 * WLAN_ELEMID_SSID(0) has already been copied to the link specific
	 * frame. In the case of (re)association response, it has been verified
	 * that the element with actual ID WLAN_ELEMID_SSID(0) is present
	 * neither for the reporting STA nor in the STA profile.
	 */
	sta_prof_iesection_currpos = sta_prof_iesection;
	sta_prof_iesection_remlen = sta_prof_iesection_len;

	while (sta_prof_iesection_remlen > 0) {
		sta_prof_ie = sta_prof_iesection_currpos;
		ret = util_validate_sta_prof_ie(sta_prof_ie,
						sta_prof_iesection_currpos,
						sta_prof_iesection_remlen);
		if (QDF_IS_STATUS_ERROR(ret)) {
			qdf_mem_free(orig_mlie_copy);
			return ret;
		}

		sta_prof_ie_size = sta_prof_ie[TAG_LEN_POS] + MIN_IE_LEN;

		if (!sta_prof_ie[0]) {
			/* Skip this, since it has already been processed */
			sta_prof_iesection_currpos += sta_prof_ie_size;
			sta_prof_iesection_remlen -= sta_prof_ie_size;
			continue;
		}

		/* Copy IE from STA profile into link specific frame. */
		if ((link_frame_currpos + sta_prof_ie_size) <=
			(link_frame + link_frame_maxsize)) {
			qdf_mem_copy(link_frame_currpos,
				     sta_prof_ie,
				     sta_prof_ie_size);

			link_frame_currpos += sta_prof_ie_size;
			link_frame_currlen +=
				sta_prof_ie_size;

			if (reportingsta_ie[ID_POS] ==
					WLAN_ELEMID_EXTN_ELEM) {
				mlo_debug("IE with element ID : %u extension element ID : %u (%zu octets) is present only in STA profile. Copied IE from STA profile to link specific frame",
					  sta_prof_ie[ID_POS],
					  sta_prof_ie[IDEXT_POS],
					  sta_prof_ie_size);
			} else {
				mlo_debug("IE with element ID : %u (%zu octets) is present only in STA profile. Copied IE from STA profile to link specific frame",
					  sta_prof_ie[ID_POS],
					  sta_prof_ie_size);
			}

			sta_prof_ie[0] = 0;
		} else {
			if (sta_prof_ie[ID_POS] == WLAN_ELEMID_EXTN_ELEM) {
				mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u extension element ID : %u. Required: %zu octets, available: %zu octets",
					   sta_prof_ie[ID_POS],
					   sta_prof_ie[IDEXT_POS],
					   sta_prof_ie_size,
					   link_frame_maxsize -
					   link_frame_currlen);
			} else {
				mlo_err_rl("Insufficent space in link specific frame for IE with element ID : %u. Required: %zu octets, available: %zu octets",
					   sta_prof_ie[ID_POS],
					   sta_prof_ie_size,
					   link_frame_maxsize -
					   link_frame_currlen);
			}

			qdf_mem_free(orig_mlie_copy);
			return QDF_STATUS_E_NOMEM;
		}

		sta_prof_iesection_currpos += sta_prof_ie_size;
		sta_prof_iesection_remlen -= sta_prof_ie_size;
	}

	/* Copy the link MAC addr */
	link_frame_hdr = (struct wlan_frame_hdr *)link_frame;

	if ((subtype == WLAN_FC0_STYPE_ASSOC_REQ) ||
	    (subtype == WLAN_FC0_STYPE_REASSOC_REQ)) {
		qdf_mem_copy(link_frame_hdr->i_addr3, &link_addr,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(link_frame_hdr->i_addr2, reportedmacaddr.bytes,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(link_frame_hdr->i_addr1, &link_addr,
			     QDF_MAC_ADDR_SIZE);

		link_frame_hdr->i_fc[0] = MLO_LINKSPECIFIC_ASSOC_REQ_FC0;
		link_frame_hdr->i_fc[1] = MLO_LINKSPECIFIC_ASSOC_REQ_FC1;
	} else {
		/* This is a (re)association response */

		qdf_mem_copy(link_frame_hdr->i_addr3, reportedmacaddr.bytes,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(link_frame_hdr->i_addr2, reportedmacaddr.bytes,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(link_frame_hdr->i_addr1, &link_addr,
			     QDF_MAC_ADDR_SIZE);

		link_frame_hdr->i_fc[0] = MLO_LINKSPECIFIC_ASSOC_RESP_FC0;
		link_frame_hdr->i_fc[1] = MLO_LINKSPECIFIC_ASSOC_RESP_FC1;
	}

	/* Seq num not used so not populated */

	qdf_mem_free(orig_mlie_copy);

	*link_frame_len = link_frame_currlen;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
util_gen_link_assoc_req(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len)
{
	return util_gen_link_assoc_reqrsp_cmn(frame, frame_len,
			(isreassoc ? WLAN_FC0_STYPE_REASSOC_REQ :
				WLAN_FC0_STYPE_ASSOC_REQ),
			link_addr, link_frame, link_frame_maxsize,
			link_frame_len);
}

QDF_STATUS
util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t frame_len, bool isreassoc,
			struct qdf_mac_addr link_addr,
			uint8_t *link_frame,
			qdf_size_t link_frame_maxsize,
			qdf_size_t *link_frame_len)
{
	return util_gen_link_assoc_reqrsp_cmn(frame, frame_len,
			(isreassoc ?  WLAN_FC0_STYPE_REASSOC_RESP :
				WLAN_FC0_STYPE_ASSOC_RESP),
			link_addr, link_frame, link_frame_maxsize,
			link_frame_len);
}

QDF_STATUS
util_find_mlie(uint8_t *buf, qdf_size_t buflen, uint8_t **mlieseq,
	       qdf_size_t *mlieseqlen)
{
	uint8_t *bufboundary;
	uint8_t *ieseq;
	qdf_size_t ieseqlen;
	uint8_t *currie;
	uint8_t *successorfrag;

	if (!buf || !buflen || !mlieseq || !mlieseqlen)
		return QDF_STATUS_E_NULL_VALUE;

	*mlieseq = NULL;
	*mlieseqlen = 0;

	/* Find Multi-Link element. In case a fragment sequence is present,
	 * this element will be the leading fragment.
	 */
	ieseq = util_find_extn_eid(WLAN_ELEMID_EXTN_ELEM,
				   WLAN_EXTN_ELEMID_MULTI_LINK, buf,
				   buflen);

	/* Even if the element is not found, we have successfully examined the
	 * buffer. The caller will be provided a NULL value for the starting of
	 * the Multi-Link element. Hence, we return success.
	 */
	if (!ieseq)
		return QDF_STATUS_SUCCESS;

	bufboundary = buf + buflen;

	if ((ieseq + MIN_IE_LEN) > bufboundary)
		return QDF_STATUS_E_INVAL;

	ieseqlen = MIN_IE_LEN + ieseq[TAG_LEN_POS];

	if (ieseqlen < sizeof(struct wlan_ie_multilink))
		return QDF_STATUS_E_PROTO;

	if ((ieseq + ieseqlen) > bufboundary)
		return QDF_STATUS_E_INVAL;

	/* In the next sequence of checks, if there is no space in the buffer
	 * for another element after the Multi-Link element/element fragment
	 * sequence, it could indicate an issue since non-MLO EHT elements
	 * would be expected to follow the Multi-Link element/element fragment
	 * sequence. However, this is outside of the purview of this function,
	 * hence we ignore it.
	 */

	currie = ieseq;
	successorfrag = util_get_successorfrag(currie, buf, buflen);

	/* Fragmentation definitions as of IEEE802.11be D1.0 and
	 * IEEE802.11REVme D0.2 are applied. Only the case where Multi-Link
	 * element is present in a buffer from the core frame is considered.
	 * Future changes to fragmentation, cases where the Multi-Link element
	 * is present in a subelement, etc. to be reflected here if applicable
	 * as and when the rules evolve.
	 */
	while (successorfrag) {
		/* We should not be seeing a successor fragment if the length
		 * of the current IE is lesser than the max.
		 */
		if (currie[TAG_LEN_POS] != WLAN_MAX_IE_LEN)
			return QDF_STATUS_E_PROTO;

		if (successorfrag[TAG_LEN_POS] == 0)
			return QDF_STATUS_E_PROTO;

		ieseqlen +=  (MIN_IE_LEN + successorfrag[TAG_LEN_POS]);

		currie = successorfrag;
		successorfrag = util_get_successorfrag(currie, buf, buflen);
	}

	*mlieseq = ieseq;
	*mlieseqlen = ieseqlen;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
util_get_mlie_variant(uint8_t *mlieseq, qdf_size_t mlieseqlen,
		      int *variant)
{
	struct wlan_ie_multilink *mlie_fixed;
	enum wlan_ml_variant var;
	uint16_t mlcontrol;

	if (!mlieseq || !mlieseqlen || !variant)
		return QDF_STATUS_E_NULL_VALUE;

	if (mlieseqlen < sizeof(struct wlan_ie_multilink))
		return QDF_STATUS_E_INVAL;

	mlie_fixed = (struct wlan_ie_multilink *)mlieseq;

	if ((mlie_fixed->elem_id != WLAN_ELEMID_EXTN_ELEM) ||
	    (mlie_fixed->elem_id_ext != WLAN_EXTN_ELEMID_MULTI_LINK))
		return QDF_STATUS_E_INVAL;

	mlcontrol = le16toh(mlie_fixed->mlcontrol);
	var = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_TYPE_IDX,
			   WLAN_ML_CTRL_TYPE_BITS);

	if (var >= WLAN_ML_VARIANT_INVALIDSTART)
		return QDF_STATUS_E_PROTO;

	*variant = var;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
util_get_bvmlie_mldmacaddr(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			   bool *mldmacaddrfound,
			   struct qdf_mac_addr *mldmacaddr)
{
	struct wlan_ie_multilink *mlie_fixed;
	enum wlan_ml_variant variant;
	uint16_t mlcontrol;
	uint16_t presencebitmap;

	if (!mlieseq || !mlieseqlen || !mldmacaddrfound || !mldmacaddr)
		return QDF_STATUS_E_NULL_VALUE;

	*mldmacaddrfound = false;
	qdf_mem_zero(mldmacaddr, sizeof(*mldmacaddr));

	if (mlieseqlen < sizeof(struct wlan_ie_multilink))
		return QDF_STATUS_E_INVAL;

	mlie_fixed = (struct wlan_ie_multilink *)mlieseq;

	if ((mlie_fixed->elem_id != WLAN_ELEMID_EXTN_ELEM) ||
	    (mlie_fixed->elem_id_ext != WLAN_EXTN_ELEMID_MULTI_LINK))
		return QDF_STATUS_E_INVAL;

	mlcontrol = le16toh(mlie_fixed->mlcontrol);

	variant = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_TYPE_IDX,
			       WLAN_ML_CTRL_TYPE_BITS);

	if (variant != WLAN_ML_VARIANT_BASIC)
		return QDF_STATUS_E_INVAL;

	presencebitmap = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_PBM_IDX,
				      WLAN_ML_CTRL_PBM_BITS);

	if (presencebitmap & WLAN_ML_BV_CTRL_PBM_MLDMACADDR_P) {
		/* Common Info starts at mlieseq + sizeof(struct
		 * wlan_ie_multilink). Check if there is sufficient space in
		 * Common Info for the MLD MAC address.
		 */
		if ((sizeof(struct wlan_ie_multilink) + QDF_MAC_ADDR_SIZE) >
				mlieseqlen)
			return QDF_STATUS_E_PROTO;

		*mldmacaddrfound = true;
		qdf_mem_copy(mldmacaddr->bytes,
			     mlieseq + sizeof(struct wlan_ie_multilink),
			     QDF_MAC_ADDR_SIZE);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
util_get_bvmlie_primary_linkid(uint8_t *mlieseq, qdf_size_t mlieseqlen,
			       bool *linkidfound, uint8_t *linkid)
{
	struct wlan_ie_multilink *mlie_fixed;
	enum wlan_ml_variant variant;
	uint16_t mlcontrol;
	uint16_t presencebitmap;
	uint8_t *commoninfo;
	qdf_size_t commoninfolen;
	uint8_t *linkidinfo;

	if (!mlieseq || !mlieseqlen || !linkidfound || !linkid)
		return QDF_STATUS_E_NULL_VALUE;

	*linkidfound = false;
	*linkid = 0;

	if (mlieseqlen < sizeof(struct wlan_ie_multilink))
		return QDF_STATUS_E_INVAL;

	mlie_fixed = (struct wlan_ie_multilink *)mlieseq;

	if ((mlie_fixed->elem_id != WLAN_ELEMID_EXTN_ELEM) ||
	    (mlie_fixed->elem_id_ext != WLAN_EXTN_ELEMID_MULTI_LINK))
		return QDF_STATUS_E_INVAL;

	mlcontrol = le16toh(mlie_fixed->mlcontrol);

	variant = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_TYPE_IDX,
			       WLAN_ML_CTRL_TYPE_BITS);

	if (variant != WLAN_ML_VARIANT_BASIC)
		return QDF_STATUS_E_INVAL;

	presencebitmap = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_PBM_IDX,
				      WLAN_ML_CTRL_PBM_BITS);

	commoninfo = mlieseq + sizeof(struct wlan_ie_multilink);
	commoninfolen = 0;

	if (presencebitmap & WLAN_ML_BV_CTRL_PBM_MLDMACADDR_P) {
		commoninfolen += QDF_MAC_ADDR_SIZE;

		if ((sizeof(struct wlan_ie_multilink) + commoninfolen) >
				mlieseqlen)
			return QDF_STATUS_E_PROTO;
	}

	if (presencebitmap & WLAN_ML_BV_CTRL_PBM_LINKIDINFO_P) {
		linkidinfo = commoninfo + commoninfolen;
		commoninfolen += WLAN_ML_BV_CINFO_LINKIDINFO_SIZE;

		if ((sizeof(struct wlan_ie_multilink) + commoninfolen) >
				mlieseqlen)
			return QDF_STATUS_E_PROTO;

		*linkidfound = true;
		*linkid = QDF_GET_BITS(linkidinfo[0],
				       WLAN_ML_BV_CINFO_LINKIDINFO_LINKID_IDX,
				       WLAN_ML_BV_CINFO_LINKIDINFO_LINKID_BITS);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
util_get_bvmlie_persta_partner_info(uint8_t *mlie, qdf_size_t mlielen,
				    struct mlo_partner_info *partner_info)
{
	struct wlan_ie_multilink *mlie_fixed;
	uint16_t mlcontrol;
	enum wlan_ml_variant variant;
	uint8_t *linkinfo;
	qdf_size_t linkinfo_len;
	struct mlo_partner_info pinfo = {0};
	QDF_STATUS ret;

	if (!mlie) {
		mlo_err("mlie is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mlielen) {
		mlo_err("mlielen is zero");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!partner_info) {
		mlo_err("partner_info is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	partner_info->num_partner_links = 0;

	if (mlielen < sizeof(struct wlan_ie_multilink)) {
		mlo_err_rl("mlielen %zu octets is smaller than required for the fixed portion of Multi-Link element (%zu octets)",
			   mlielen, sizeof(struct wlan_ie_multilink));
		return QDF_STATUS_E_INVAL;
	}

	/* Note: Multi-Link element fragmentation support will be added in a
	 * later change once shared helper utilities for the same are available.
	 * As of now, we temporarily return error if the mlielen is greater than
	 * the max length for an IE.
	 */
	if (mlielen > (sizeof(struct ie_header) + WLAN_MAX_IE_LEN)) {
		mlo_err_rl("Element fragmentation is not yet supported for this API");
		return QDF_STATUS_E_NOSUPPORT;
	}

	mlie_fixed = (struct wlan_ie_multilink *)mlie;

	if ((mlie_fixed->elem_id != WLAN_ELEMID_EXTN_ELEM) ||
	    (mlie_fixed->elem_id_ext != WLAN_EXTN_ELEMID_MULTI_LINK)) {
		mlo_err("The element is not a Multi-Link element");
		return QDF_STATUS_E_INVAL;
	}

	mlcontrol = le16toh(mlie_fixed->mlcontrol);

	variant = QDF_GET_BITS(mlcontrol, WLAN_ML_CTRL_TYPE_IDX,
			       WLAN_ML_CTRL_TYPE_BITS);

	if (variant != WLAN_ML_VARIANT_BASIC) {
		mlo_err("The variant value %u does not correspond to Basic Variant value %u",
			variant, WLAN_ML_VARIANT_BASIC);
		return QDF_STATUS_E_INVAL;
	}

	linkinfo_len = 0;
	linkinfo = util_parse_multi_link_ctrl(mlie, mlielen,
					      &linkinfo_len);

	if (!linkinfo) {
		mlo_err_rl("Error in parsing Multi-Link element control");
		return QDF_STATUS_E_INVAL;
	}

	/* In case Link Info is absent as indicated by the Link Info length
	 * being 0, return success. The number of partner links will remain 0.
	 */
	if (!linkinfo_len) {
		mlo_warn_rl("Link Info is absent");
		return QDF_STATUS_SUCCESS;
	}

	ret = util_parse_partner_info_from_linkinfo(linkinfo,
						    linkinfo_len,
						    &pinfo);

	if (QDF_IS_STATUS_ERROR(ret))
		return ret;

	qdf_mem_copy(partner_info, &pinfo, sizeof(*partner_info));
	return QDF_STATUS_SUCCESS;
}
#endif

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

	while (len >= MIN_IE_LEN && len >= frame[TAG_LEN_POS] + MIN_IE_LEN) {
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
uint8_t *util_parse_perstaprofile(uint8_t *subelement,
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
			endofstainfo = util_parse_perstaprofile(currpos,
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

QDF_STATUS util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t len,
				   struct qdf_mac_addr link_addr,
				   uint8_t *assoc_link_frame)
{
	uint8_t *tmp = NULL;
	const uint8_t *tmp_old, *rsn_ie;
	qdf_size_t sub_len, tmp_rem_len;
	qdf_size_t link_info_len, sta_prof_len = 0;
	uint8_t *subelement;
	uint8_t *pos;
	uint8_t *sub_copy, *orig_copy;
	bool is_bssid_valid;
	struct qdf_mac_addr bssid;
	struct wlan_frame_hdr *hdr;

	if (!frame || !len)
		return QDF_STATUS_E_NULL_VALUE;

	pos = assoc_link_frame;
	hdr = (struct wlan_frame_hdr *)pos;
	pos = pos + WLAN_MAC_HDR_LEN_3A;

	/* Assoc resp Capability(2) + AID(2) + Status Code(2) */
	qdf_mem_copy(pos, frame, WLAN_ASSOC_RSP_IES_OFFSET);
	pos = pos + WLAN_ASSOC_RSP_IES_OFFSET;

	rsn_ie = wlan_get_ie_ptr_from_eid(WLAN_ELEMID_RSN, frame, len);
	if (rsn_ie) {
		qdf_mem_copy(pos, rsn_ie, rsn_ie[1]);
		pos = pos + rsn_ie[1];
	}
	/* find MLO IE */
	subelement = util_find_extn_eid(WLAN_ELEMID_EXTN_ELEM,
					WLAN_EXTN_ELEMID_MULTI_LINK,
					frame,
					len);
	if (!subelement)
		return QDF_STATUS_E_FAILURE;

	/*  EID(1) + len (1) = 2 */
	sub_len = subelement[TAG_LEN_POS] + 2;
	sub_copy = qdf_mem_malloc(sub_len);
	if (!sub_copy)
		return QDF_STATUS_E_NOMEM;
	orig_copy = sub_copy;
	qdf_mem_copy(sub_copy, subelement, sub_len);

	/* parse ml ie */
	sub_copy = util_parse_multi_link_ctrl(sub_copy,
					      sub_len,
					      &link_info_len);

	if (!sub_copy)
		return QDF_STATUS_E_NULL_VALUE;

	mlo_debug("dumping hex after parsing multi link ctrl");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_MLO, QDF_TRACE_LEVEL_DEBUG,
			   sub_copy, link_info_len);

	is_bssid_valid = false;

	/* Parse per-STA profile */
	sub_copy = util_parse_perstaprofile(sub_copy,
					    link_info_len,
					    true,
					    &sta_prof_len,
					    NULL,
					    &is_bssid_valid,
					    &bssid);

	if (!is_bssid_valid)
		return QDF_STATUS_E_NULL_VALUE;

	if (!sub_copy) {
		qdf_mem_copy(pos, frame, len);
		pos += len - WLAN_MAC_HDR_LEN_3A;
		goto update_header;
	}

	/* go through IEs in frame and subelement,
	 * merge them into new_ie
	 */
	tmp_old = util_find_eid(WLAN_ELEMID_SSID, frame, len);
	tmp_old = (tmp_old) ? tmp_old + tmp_old[TAG_LEN_POS] + MIN_IE_LEN : frame;

	while (((tmp_old + tmp_old[TAG_LEN_POS] + MIN_IE_LEN) - frame) <= len) {
		tmp = (uint8_t *)util_find_eid(tmp_old[0],
					       sub_copy,
					       sta_prof_len);
		if (!tmp) {
			/* ie in old ie but not in subelement */
			if (tmp_old[2] != WLAN_EXTN_ELEMID_MULTI_LINK) {
				if ((pos + tmp_old[TAG_LEN_POS] + MIN_IE_LEN) <=
					(assoc_link_frame + len)) {
					qdf_mem_copy(pos, tmp_old,
						     (tmp_old[TAG_LEN_POS] +
						     MIN_IE_LEN));
					pos += tmp_old[TAG_LEN_POS] + MIN_IE_LEN;
				}
			}
		} else {
			/* ie in transmitting ie also in subelement,
			 * copy from subelement and flag the ie in subelement
			 * as copied (by setting eid field to 0xff). For
			 * vendor ie, compare OUI + type + subType to
			 * determine if they are the same ie.
			 */
			tmp_rem_len = sta_prof_len - (tmp - sub_copy);
			if (tmp_old[0] == WLAN_ELEMID_VENDOR &&
			    tmp_rem_len >= MIN_VENDOR_TAG_LEN) {
				if (!qdf_mem_cmp(tmp_old + PAYLOAD_START_POS,
						 tmp + PAYLOAD_START_POS,
						 OUI_LEN)) {
					/* same vendor ie, copy from
					 * subelement
					 */
					if ((pos + tmp[TAG_LEN_POS] + MIN_IE_LEN) <=
						(assoc_link_frame + len)) {
						qdf_mem_copy(pos, tmp,
							     tmp[TAG_LEN_POS] +
							     MIN_IE_LEN);
						pos += tmp[TAG_LEN_POS] + MIN_IE_LEN;
						tmp[0] = 0;
					}
				} else {
					if ((pos + tmp_old[TAG_LEN_POS] +
						 MIN_IE_LEN) <=
						(assoc_link_frame + len)) {
						qdf_mem_copy(pos, tmp_old,
							     tmp_old[TAG_LEN_POS] +
							     MIN_IE_LEN);
						pos += tmp_old[TAG_LEN_POS] +
							MIN_IE_LEN;
					}
				}
			} else if (tmp_old[0] == WLAN_ELEMID_EXTN_ELEM) {
				if (tmp_old[PAYLOAD_START_POS] ==
					tmp[PAYLOAD_START_POS]) {
					/* same ie, copy from subelement */
					if ((pos + tmp[TAG_LEN_POS] + MIN_IE_LEN) <=
						(assoc_link_frame + len)) {
						qdf_mem_copy(pos, tmp,
							     tmp[TAG_LEN_POS] +
							     MIN_IE_LEN);
						pos += tmp[TAG_LEN_POS] + MIN_IE_LEN;
						tmp[0] = 0;
					}
				} else {
					if ((pos + tmp_old[TAG_LEN_POS] + MIN_IE_LEN) <=
						(assoc_link_frame + len)) {
						qdf_mem_copy(pos, tmp_old,
							     tmp_old[TAG_LEN_POS] +
							     MIN_IE_LEN);
						pos += tmp_old[TAG_LEN_POS] +
							MIN_IE_LEN;
					}
				}
			} else {
				/* copy ie from subelement into new ie */
				if ((pos + tmp[TAG_LEN_POS] + MIN_IE_LEN) <=
					(assoc_link_frame + len)) {
					qdf_mem_copy(pos, tmp,
						     tmp[TAG_LEN_POS] + MIN_IE_LEN);
					pos += tmp[TAG_LEN_POS] + MIN_IE_LEN;
					tmp[0] = 0;
				}
			}
		}

		if (((tmp_old + tmp_old[TAG_LEN_POS] + MIN_IE_LEN) - frame) >= len)
			break;

		tmp_old += tmp_old[TAG_LEN_POS] + MIN_IE_LEN;
	}

update_header:
	/* Copy the link mac addr */
	qdf_mem_copy(hdr->i_addr3, bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->i_addr2, bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->i_addr1, &link_addr,
		     QDF_MAC_ADDR_SIZE);
	hdr->i_fc[0] = FC0_IEEE_MGMT_FRM;
	hdr->i_fc[1] = FC1_IEEE_MGMT_FRM;
	/* seq num not used so not populated */
	qdf_mem_free(orig_copy);

	return QDF_STATUS_SUCCESS;
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

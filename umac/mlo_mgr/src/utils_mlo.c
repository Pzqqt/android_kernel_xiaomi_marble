/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
uint8_t *util_parse_multi_link_ctrl_ie(uint8_t *subelement,
				       qdf_size_t len,
				       qdf_size_t *link_info_ie_len)
{
	qdf_size_t sub_ie_len = 0;

	if (!subelement || !len)
		return NULL;

	/* sta prof len = ML IE len + EID extn(1) + Multi Lnk ctrl(2) */
	sub_ie_len += TAG_LEN_POS + MIN_IE_LEN + 2;

	/* check if MLD MAC address present */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_MLDMACADDR_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_1]))
		sub_ie_len = sub_ie_len + QDF_MAC_ADDR_SIZE;

	/* check if Link ID info */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_LINKIDINFO_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_1]))
		sub_ie_len = sub_ie_len + TAG_LEN_POS;

	/* check if BSS parameter change count */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_BSSPARAMCHANGECNT_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_1]))
		sub_ie_len = sub_ie_len + TAG_LEN_POS;

	/* check if Medium Sync Delay Info present */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_MEDIUMSYNCDELAYINFO_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_1]))
		sub_ie_len = sub_ie_len + PAYLOAD_START_POS;

	/* check if EML cap present */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_EMLCAP_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_2]))
		sub_ie_len = sub_ie_len + SUBELEMENT_START_POS;

	/* check if MLD cap present */
	if (qdf_test_bit(WLAN_ML_CTRL_PBM_IDX |
			 WLAN_ML_BV_CTRL_PBM_MLDCAP_P,
			 (unsigned long *)&subelement[MULTI_LINK_CTRL_2]))
		sub_ie_len = sub_ie_len + PAYLOAD_START_POS;

	if (link_info_ie_len) {
		*link_info_ie_len = len - sub_ie_len;
		mlo_debug("link_info_ie_len:%zu, sub_ie_len:%zu",
			  *link_info_ie_len, sub_ie_len);
	}

	return &subelement[sub_ie_len];
}

static
uint8_t *util_parse_sta_profile_ie(uint8_t *subelement,
				   qdf_size_t len,
				   qdf_size_t *per_sta_prof_ie_len,
				   struct qdf_mac_addr *bssid)
{
	qdf_size_t tmp_len = 0;
	uint8_t *tmp = NULL;

	if (!subelement || !len)
		return NULL;

	if (subelement[0] == 0)
		tmp = subelement;
	if (!tmp)
		return NULL;

	/* tmp_len = 2 (sta ctrl) + 1 (sub EID) + 1 (len) */
	tmp_len = PAYLOAD_START_POS + 2;

	/* check DTIM info present bit */
	if (qdf_test_bit(WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_DTIMINFOP_IDX,
			 (unsigned long *)&subelement[STA_CTRL_1]))
		tmp_len = tmp_len + MIN_IE_LEN;

	/* check Beacon interval present bit */
	if (qdf_test_bit(WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_BCNINTP_IDX,
			 (unsigned long *)&subelement[STA_CTRL_1]))
		tmp_len = tmp_len + TAG_LEN_POS;

	/* check STA link mac addr info present bit */
	if (qdf_test_bit(WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_MACADDRP_IDX,
			 (unsigned long *)&subelement[STA_CTRL_1])) {
		tmp_len = tmp_len + QDF_MAC_ADDR_SIZE;
		mlo_debug("copied mac addr: " QDF_MAC_ADDR_FMT, &tmp[4]);
		qdf_mem_copy(&bssid->bytes, &tmp[4], QDF_MAC_ADDR_SIZE);
	}

	/* Add Link ID offset,as it will always be present in assoc rsp mlo ie */
	tmp_len = tmp_len + TAG_LEN_POS;

	/* check NTSR Link pair present bit */
	if (qdf_test_bit(WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRLINKPRP_IDX % 8,
			 (unsigned long *)&subelement[STA_CTRL_2]))
		tmp_len = tmp_len + MIN_IE_LEN;

	/* check NTSR bitmap size present bit */
	if (qdf_test_bit(WLAN_ML_BV_LINFO_PERSTAPROF_STACTRL_NSTRBMSZ_IDX % 8,
			 (unsigned long *)&subelement[STA_CTRL_2]))
		tmp_len = tmp_len + TAG_LEN_POS;

	if (len <= tmp_len) {
		mlo_err("len %zu <= tmp_len %zu, return", len, tmp_len);
		return NULL;
	}
	*per_sta_prof_ie_len = len - tmp_len;

	return &tmp[tmp_len];
}

QDF_STATUS util_gen_link_assoc_rsp(uint8_t *frame, qdf_size_t len,
				   struct qdf_mac_addr link_addr,
				   uint8_t *assoc_link_frame)
{
	uint8_t *tmp = NULL;
	const uint8_t *tmp_old, *rsn_ie;
	qdf_size_t sub_len, tmp_rem_len;
	qdf_size_t link_info_len, per_sta_prof_len = 0;
	uint8_t *subelement;
	uint8_t *pos;
	uint8_t *sub_copy, *orig_copy;
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
	sub_copy = util_parse_multi_link_ctrl_ie(sub_copy,
						 sub_len,
						 &link_info_len);

	if (!sub_copy)
		return QDF_STATUS_E_NULL_VALUE;

	mlo_debug("dumping hex after parsing multi link ctrl");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_MLO, QDF_TRACE_LEVEL_DEBUG,
			   sub_copy, link_info_len);

	/* parse sta profile ie */
	sub_copy = util_parse_sta_profile_ie(sub_copy,
					     link_info_len,
					     &per_sta_prof_len,
					     &bssid);

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
					       per_sta_prof_len);
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
			tmp_rem_len = per_sta_prof_len - (tmp - sub_copy);
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
#endif

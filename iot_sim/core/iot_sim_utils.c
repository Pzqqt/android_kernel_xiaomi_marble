/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#include <wlan_iot_sim_utils_api.h>
#include <qdf_module.h>
#include "../../core/iot_sim_cmn_api_i.h"
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#define IEEE80211_FRAME_BODY_OFFSET 0x18

QDF_STATUS iot_sim_frame_update(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t nbuf)
{
	uint8_t type, subtype, seq = 0;
	struct iot_sim_context *isc;
	uint8_t *buf = qdf_nbuf_data(nbuf), *frm = NULL;
	int fixed_param_length = 0;
	bool is_action_frm = false;
	uint8_t cat, cat_index;
	int auth_seq_index = 0, offset = 0;
	struct iot_sim_rule *piot_sim_rule = NULL;
	qdf_size_t buf_len = 0;

	type = (buf[0] & IEEE80211_FC0_TYPE_MASK) >> IEEE80211_FC0_TYPE_SHIFT;
	subtype = (buf[0] & IEEE80211_FC0_SUBTYPE_MASK);
	isc = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_IOT_SIM_COMP);
	if (!isc) {
		iot_sim_err("pdev IOT_SIM object is NULL!");
		return QDF_STATUS_SUCCESS;
	}

	if (type == IEEE80211_FC0_TYPE_MGT &&
	    subtype == IEEE80211_FC0_SUBTYPE_AUTH) {
	/* Authentication frame */
		auth_seq_index = IEEE80211_FRAME_BODY_OFFSET + 2;
		seq = le16toh(*(u_int16_t *)(buf + auth_seq_index));
	} else if (type == IEEE80211_FC0_TYPE_MGT &&
		   subtype == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
	/* Probe response frame */
		fixed_param_length = 12;
	else if (type == IEEE80211_FC0_TYPE_MGT &&
		 (subtype == IEEE80211_FC0_SUBTYPE_ASSOC_RESP ||
		  subtype == IEEE80211_FC0_SUBTYPE_REASSOC_RESP))
	/* Assoc/Reassoc response frame */
		fixed_param_length = 6;
	else if (type == IEEE80211_FC0_TYPE_MGT &&
		 subtype == IEEE80211_FC0_SUBTYPE_ACTION) {
	/* Action frame */
		frm = buf + IEEE80211_FRAME_BODY_OFFSET;

		is_action_frm = true;
		if (iot_sim_get_index_for_action_frm(frm, &cat, &cat_index)) {
			iot_sim_err("get_index_for_action_frm failed");
			return QDF_STATUS_SUCCESS;
		}
	}

	subtype >>= IEEE80211_FC0_SUBTYPE_SHIFT;
	iot_sim_debug("iot_sim: Change content operation for frame");
	iot_sim_debug("type:%d subtype:%d seq:%d, is_action_frm: %u",
		      type, subtype, seq, is_action_frm);

	/* Only broadcast peer is getting handled right now.
	 * Need to add support for peer based content modification
	 */
	qdf_spin_lock(&isc->bcast_peer.iot_sim_lock);
	if (!isc->bcast_peer.rule_per_seq[seq])
		goto norule;

	buf_len = qdf_nbuf_len(nbuf);

	if (is_action_frm)
		piot_sim_rule = isc->bcast_peer.rule_per_seq[seq]->
			rule_per_action_frm[cat][cat_index];
	else
		piot_sim_rule = isc->bcast_peer.rule_per_seq[seq]->
			rule_per_type[type][subtype];

	if (!piot_sim_rule)
		goto norule;

	if (!piot_sim_rule->frm_content || !piot_sim_rule->len)
		goto norule;

	if (piot_sim_rule->offset ==
			IEEE80211_FRAME_BODY_OFFSET) {
		offset = IEEE80211_FRAME_BODY_OFFSET;
	} else if (piot_sim_rule->offset == 0) {
		offset = 0;
	} else if (buf[piot_sim_rule->offset] ==
			piot_sim_rule->frm_content[0]) {
		offset = piot_sim_rule->offset;
	}  else {
		offset = IEEE80211_FRAME_BODY_OFFSET +
			fixed_param_length;
		while (((offset + 1) < buf_len) &&
		       (buf[offset] < piot_sim_rule->frm_content[0])) {
			offset += buf[offset + 1] + 2;
		}
	}

	if (offset <= buf_len) {
		buf += offset;
		qdf_mem_copy(buf, piot_sim_rule->frm_content,
			     piot_sim_rule->len);
		qdf_nbuf_set_pktlen(nbuf, offset +
				piot_sim_rule->len);
		iot_sim_debug("iot_sim: Content updated");
	} else {
		iot_sim_err("Failed to modify content");
	}
	qdf_spin_unlock(&isc->bcast_peer.iot_sim_lock);
	return QDF_STATUS_SUCCESS;

norule:
	iot_sim_debug("Rule not set for this frame");
	qdf_spin_unlock(&isc->bcast_peer.iot_sim_lock);
	return QDF_STATUS_SUCCESS;
}

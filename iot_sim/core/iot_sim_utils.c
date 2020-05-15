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

/*
 * iot_sim_apply_content_change_rule - function to apply content change rule
 *				       packet from upper stack will be modified
 *				       as per the user content.
 * @piot_sim_rule: iot_sim rule structure
 * @nbuf: skb coming from upper stack
 * @fixed_param_length: length of fixed parameters in frame body
 *
 * Return: QDF_STATUS_SUCCESS on successful content update or otherwise
 *	   QDF_STATUS_E_NOSUPPORT, no content change rule found for this frame
 */
QDF_STATUS
iot_sim_apply_content_change_rule(struct iot_sim_rule *piot_sim_rule,
				  qdf_nbuf_t nbuf,
				  int fixed_param_length)
{
	uint8_t *buf = NULL;
	qdf_size_t buf_len = 0;
	int offset = 0;

	if (!piot_sim_rule->frm_content || !piot_sim_rule->len)
		return QDF_STATUS_E_NOSUPPORT;

	buf_len = qdf_nbuf_len(nbuf);
	buf = qdf_nbuf_data(nbuf);

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

	return QDF_STATUS_SUCCESS;
}

/*
 * iot_sim_apply_drop_rule - function to apply drop rule. If set buffer will be
 *			     freed here and proper return value will be sent to
 *			     tgt layer.
 *
 * @piot_sim_rule: iot_sim rule structure
 * @nbuf: skb coming from upper stack
 *
 * Return: QDF_STATUS_SUCCESS on successful drop
 *	   QDF_STATUS_E_NULL_VALUE, when drop rule is applied
 */
QDF_STATUS
iot_sim_apply_drop_rule(struct iot_sim_rule *piot_sim_rule,
			qdf_nbuf_t nbuf)
{
	if (!piot_sim_rule->drop)
		return QDF_STATUS_E_NOSUPPORT;

	if (nbuf)
		qdf_nbuf_free(nbuf);

	iot_sim_debug("iot_sim: Drop rule applied");
	return QDF_STATUS_SUCCESS;
}

/*
 * iot_sim_frame_update - Function to parse input packet coming from upper
 *			  stack in Tx direction and to tgt layer in Rx
 *			  direction. This function will also check if rule
 *			  for that frame type/subtype is set or not and call
 *			  specific operation functions.
 *
 * @pdev: pdev object
 * @nbuf: input packet
 * @direction: direction to specify from where this packet is arrived
 *
 * Return: QDF_STATUS_SUCCESS in general
 *	   QDF_STATUS_E_NOSUPPORT, no content change rule found for this frame
 */
QDF_STATUS iot_sim_frame_update(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t nbuf,
				bool tx)
{
	uint8_t type, subtype, seq = 0;
	struct iot_sim_context *isc;
	uint8_t *buf = qdf_nbuf_data(nbuf), *frm = NULL;
	int fixed_param_length = 0;
	bool is_action_frm = false;
	uint8_t cat, cat_index;
	int auth_seq_index = 0;
	struct iot_sim_rule *piot_sim_rule = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	isc = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_IOT_SIM_COMP);
	if (!isc) {
		iot_sim_err("pdev IOT_SIM object is NULL!");
		return QDF_STATUS_SUCCESS;
	}

	type = (buf[0] & IEEE80211_FC0_TYPE_MASK) >> IEEE80211_FC0_TYPE_SHIFT;
	subtype = (buf[0] & IEEE80211_FC0_SUBTYPE_MASK);

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
	iot_sim_debug("iot_sim: type:%d subtype:%d seq:%d, action:%u dir:%s",
		      type, subtype, seq, is_action_frm,
		      tx ? "TX" : "RX");

	/* Only broadcast peer is getting handled right now.
	 * Need to add support for peer based content modification
	 */
	qdf_spin_lock(&isc->bcast_peer.iot_sim_lock);
	if (!isc->bcast_peer.rule_per_seq[seq])
		goto norule;

	if (is_action_frm)
		piot_sim_rule = isc->bcast_peer.rule_per_seq[seq]->
			rule_per_action_frm[cat][cat_index];
	else
		piot_sim_rule = isc->bcast_peer.rule_per_seq[seq]->
			rule_per_type[type][subtype];

	if (!piot_sim_rule)
		goto norule;

	if (tx) {
		status = iot_sim_apply_content_change_rule(piot_sim_rule,
							   nbuf,
							   fixed_param_length);
		if (status == QDF_STATUS_E_NOSUPPORT)
			goto norule;
	} else {
		status = iot_sim_apply_drop_rule(piot_sim_rule, nbuf);
		if (QDF_IS_STATUS_SUCCESS(status))
			status = QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_unlock(&isc->bcast_peer.iot_sim_lock);
	return status;

norule:
	iot_sim_debug("Rule not set for this frame");
	qdf_spin_unlock(&isc->bcast_peer.iot_sim_lock);
	return QDF_STATUS_SUCCESS;
}

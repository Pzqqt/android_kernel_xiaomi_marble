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
#include <qdf_delayed_work.h>
#include "../../core/iot_sim_cmn_api_i.h"
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#define IEEE80211_FRAME_BODY_OFFSET 0x18
#define IEEE80211_TSF_LEN       (8)

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
iot_sim_update_beacon_template_struct(qdf_nbuf_t nbuf,
				      struct beacon_tmpl_params *param)
{
	struct ie_header *ie = NULL;
	uint16_t offset = 0, ie_len = 0;
	struct ieee80211_ath_channelswitch_ie *csa = NULL;
	struct ieee80211_extendedchannelswitch_ie *ecsa = NULL;
	struct extn_ie_header *extn_ie = NULL;

	if (!param)
		return QDF_STATUS_E_NULL_VALUE;

	/**
	 * Skip fixed field
	 */
	offset += IEEE80211_TSF_LEN; /* TSF field */
	offset += 2; /* Beacon interval */
	offset += 2; /* Capability Information */

	ie_len = wbuf_get_pktlen(nbuf) -
		 sizeof(struct ieee80211_frame) - offset;
	ie = (struct ie_header *)((uint8_t *)qdf_nbuf_data(nbuf) +
				  sizeof(struct ieee80211_frame) + offset);

	while (ie_len >= sizeof(struct ie_header)) {
		ie_len -= sizeof(struct ie_header);
		if (!ie->ie_len) {
			ie += 1;
			continue;
		}

		if (ie_len < ie->ie_len) {
			iot_sim_err("Incomplete corrupted IE:%x", ie->ie_id);
			return QDF_STATUS_E_INVAL;
		}

		switch (ie->ie_id) {
		case WLAN_ELEMID_TIM:
			if (ie->ie_len < WLAN_TIM_IE_MIN_LENGTH) {
				iot_sim_err("Invalid TIM IE Length");
				goto err;
			}
			param->tim_ie_offset = ((uint8_t *)ie -
						(uint8_t *)qdf_nbuf_data(nbuf));
			break;
		case WLAN_ELEMID_CHANSWITCHANN:
			if (ie->ie_len != WLAN_CSA_IE_MAX_LEN) {
				iot_sim_err("Invalid CSA IE Length");
				goto err;
			}
			csa =
			(struct ieee80211_ath_channelswitch_ie *)ie;
			param->csa_switch_count_offset =
				(((uint8_t *)&csa->tbttcount) -
				 (uint8_t *)qdf_nbuf_data(nbuf));
			break;
		case WLAN_ELEMID_EXTCHANSWITCHANN:
			if (ie->ie_len != WLAN_XCSA_IE_MAX_LEN) {
				iot_sim_err("Invalid ECSA IE Length");
				goto err;
			}
			ecsa =
			(struct ieee80211_extendedchannelswitch_ie *)ie;
			param->ext_csa_switch_count_offset =
				(((uint8_t *)&ecsa->tbttcount) -
				 (uint8_t *)qdf_nbuf_data(nbuf));
			break;
		case WLAN_ELEMID_EXTN_ELEM:
			extn_ie = (struct extn_ie_header *)ie;
			switch (extn_ie->ie_extn_id) {
			case WLAN_EXTN_ELEMID_ESP:
				param->esp_ie_offset =
					((uint8_t *)ie -
					 (uint8_t *)qdf_nbuf_data(nbuf));
				break;
			case WLAN_EXTN_ELEMID_MUEDCA:
				param->mu_edca_ie_offset =
					((uint8_t *)ie -
					 (uint8_t *)qdf_nbuf_data(nbuf));
				break;
			default:
				break;
			}
			break;
		case WLAN_ELEMID_MULTIPLE_BSSID:
			offset = ((uint8_t *)ie -
				 (uint8_t *)qdf_nbuf_data(nbuf));
			param->mbssid_ie_offset = offset;
			break;
		default:
			break;
		}
		/* Consume info element */
		ie_len -= ie->ie_len;
		/* Go to next IE */
		ie = (struct ie_header *)((uint8_t *)ie +
					  sizeof(struct ie_header) +
					  ie->ie_len);
	}
	param->tmpl_len = wbuf_get_pktlen(nbuf);
	param->tmpl_len_aligned = roundup(param->tmpl_len,
					  sizeof(uint32_t));
	param->frm = (uint8_t *)qdf_nbuf_data(nbuf);
	return QDF_STATUS_SUCCESS;
err:
	return QDF_STATUS_E_INVAL;
}

QDF_STATUS
iot_sim_apply_content_change_rule(struct wlan_objmgr_pdev *pdev,
				  struct iot_sim_rule *piot_sim_rule,
				  qdf_nbuf_t nbuf,
				  int fixed_param_length,
				  struct beacon_tmpl_params *param)
{
	uint8_t *buf = NULL;
	qdf_size_t buf_len = 0;
	int offset = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

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

	if (IEEE80211_IS_BEACON((struct ieee80211_frame *)qdf_nbuf_data(nbuf)))
		status = iot_sim_update_beacon_template_struct(nbuf, param);
		if (QDF_IS_STATUS_ERROR(status))
			iot_sim_err("Failed to update beacon param");

	return QDF_STATUS_SUCCESS;
}

/*
 * iot_sim_apply_delay_drop_rule - function to apply delay or drop rule.
 *                                 If drop rule is set, buffer will be freed
 *                                 here and proper return value will be sent to
 *			           tgt layer. In case of delay rule, delayed
 *                                 workqueue will be scheduled for rx frame
 *                                 processing
 *
 * @piot_sim_rule: iot_sim rule structure
 * @nbuf: skb coming from upper stack
 *
 * Return: QDF_STATUS_SUCCESS on successful drop
 *	   QDF_STATUS_E_NULL_VALUE, when drop rule is applied
 */
QDF_STATUS
iot_sim_apply_delay_drop_rule(struct iot_sim_rule *piot_sim_rule,
			      qdf_nbuf_t nbuf,
			      struct mgmt_rx_event_params *param,
			      struct iot_sim_context *isc,
			      struct qdf_mac_addr *mac_addr)
{
	struct mgmt_rx_event_params *rx_param;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(isc->pdev_obj);
	struct wlan_objmgr_peer **peer = &piot_sim_rule->peer;

	if (!piot_sim_rule->drop &&
	    !piot_sim_rule->delay_dur)
		return QDF_STATUS_E_NOSUPPORT;

	if (piot_sim_rule->drop && nbuf) {
		qdf_nbuf_free(nbuf);
		iot_sim_debug("iot_sim: Drop rule applied");
	} else if (piot_sim_rule->delay_dur) {
		if (nbuf == piot_sim_rule->sec_buf) {
			iot_sim_debug("iot_sim: rx frame process after delay");
			return QDF_STATUS_E_NOSUPPORT;
		}

		if (piot_sim_rule->nbuf_list[0]) {
			if (!qdf_delayed_work_stop(piot_sim_rule->
						   dwork)) {
				piot_sim_rule->nbuf_list[1] = nbuf;
				return QDF_STATUS_SUCCESS;
			}

			if (*peer) {
				wlan_objmgr_peer_release_ref(*peer,
							     WLAN_IOT_SIM_ID);
				*peer = NULL;
			}

			qdf_nbuf_free(piot_sim_rule->nbuf_list[0]);
			qdf_mem_free(piot_sim_rule->rx_param->rx_params);
			qdf_mem_free(piot_sim_rule->rx_param);
		}

		rx_param = qdf_mem_malloc(sizeof(struct mgmt_rx_event_params));
		if (!rx_param) {
			iot_sim_err("rx_param alloc failed");
			return QDF_STATUS_E_NOSUPPORT;
		}

		qdf_mem_copy(rx_param, param,
			     sizeof(struct mgmt_rx_event_params));
		rx_param->rx_params = qdf_mem_malloc(RX_STATUS_SIZE);
		if (!rx_param->rx_params) {
			iot_sim_err("rx_param->rx_params alloc failed");
			qdf_mem_free(rx_param);
			return QDF_STATUS_E_NOSUPPORT;
		}

		*peer = wlan_objmgr_get_peer(psoc, param->pdev_id,
					     (uint8_t *)mac_addr,
					     WLAN_IOT_SIM_ID);
		qdf_mem_copy(rx_param->rx_params,
			     param->rx_params, RX_STATUS_SIZE);
		piot_sim_rule->rx_param = rx_param;
		piot_sim_rule->nbuf_list[0] = nbuf;
		if (!qdf_delayed_work_start(piot_sim_rule->dwork,
					    piot_sim_rule->delay_dur)) {
			iot_sim_err("delayed_work_start failed");
			qdf_mem_free(rx_param->rx_params);
			qdf_mem_free(rx_param);
			if (*peer) {
				wlan_objmgr_peer_release_ref(*peer,
							     WLAN_IOT_SIM_ID);
				*peer = NULL;
			}
			return QDF_STATUS_E_NOSUPPORT;
		}

		iot_sim_err("iot_sim: Delay rule applied");
	}

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
 * @param: beacon template cmd parameter
 * @tx: tx or not
 * @rx_param: mgmt_rx_event_params
 *
 * Return: QDF_STATUS_SUCCESS in general
 *	   QDF_STATUS_E_NOSUPPORT, no content change rule found for this frame
 */
QDF_STATUS iot_sim_frame_update(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t nbuf,
				struct beacon_tmpl_params *param,
				bool tx, struct mgmt_rx_event_params *rx_param)
{
	uint8_t type, subtype, seq = 0;
	struct iot_sim_context *isc;
	uint8_t *buf = qdf_nbuf_data(nbuf), *frm = NULL;
	int fixed_param_len = 0;
	bool is_action_frm = false;
	uint8_t cat, cat_index;
	int auth_seq_index = 0;
	struct iot_sim_rule *piot_sim_rule = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct iot_sim_rule_per_peer *peer_rule;
	struct ieee80211_frame *wh = (struct ieee80211_frame *)buf;
	struct qdf_mac_addr *mac_addr;
	bool deauth_disassoc = false;

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
		   (subtype == IEEE80211_FC0_SUBTYPE_PROBE_RESP ||
		    subtype == IEEE80211_FC0_SUBTYPE_BEACON))
	/* Probe response frame */
		fixed_param_len = 12;
	else if (type == IEEE80211_FC0_TYPE_MGT &&
		 (subtype == IEEE80211_FC0_SUBTYPE_ASSOC_RESP ||
		  subtype == IEEE80211_FC0_SUBTYPE_REASSOC_RESP))
	/* Assoc/Reassoc response frame */
		fixed_param_len = 6;
	else if (type == IEEE80211_FC0_TYPE_MGT &&
		 (subtype == IEEE80211_FC0_SUBTYPE_DEAUTH ||
		  subtype == IEEE80211_FC0_SUBTYPE_DISASSOC))
		deauth_disassoc = true;
	else if (type == IEEE80211_FC0_TYPE_MGT &&
		 subtype == IEEE80211_FC0_SUBTYPE_ACTION) {
	/* Action frame */
		frm = buf + IEEE80211_FRAME_BODY_OFFSET;

		is_action_frm = true;
		if (iot_sim_get_index_for_action_frm(frm, &cat,
						     &cat_index, !tx)) {
			iot_sim_err("get_index_for_action_frm failed");
			return QDF_STATUS_SUCCESS;
		}
	}

	subtype >>= IEEE80211_FC0_SUBTYPE_SHIFT;
	iot_sim_debug("iot_sim: type:%d subtype:%d seq:%d, action:%u dir:%s",
		      type, subtype, seq, is_action_frm,
		      tx ? "TX" : "RX");

	if (tx)
		mac_addr = (struct qdf_mac_addr *)wh->i_addr1;
	else
		mac_addr = (struct qdf_mac_addr *)wh->i_addr2;

	peer_rule = iot_sim_find_peer_from_mac(isc, mac_addr);
	if (!peer_rule)
		peer_rule = &isc->bcast_peer;

	if (!peer_rule)
		goto no_peer_rule;

	qdf_spin_lock_bh(&isc->iot_sim_lock);

	if (!peer_rule->rule_per_seq[seq])
		goto norule;

	if (is_action_frm)
		piot_sim_rule = peer_rule->rule_per_seq[seq]->
			rule_per_action_frm[cat][cat_index];
	else
		piot_sim_rule = peer_rule->rule_per_seq[seq]->
			rule_per_type[type][subtype];

	if (!piot_sim_rule)
		goto norule;

	if (tx) {
		if (IEEE80211_IS_BEACON((struct ieee80211_frame *)
					qdf_nbuf_data(nbuf))) {
			if (isc->bcn_buf)
				qdf_nbuf_free(isc->bcn_buf);
			isc->bcn_buf = qdf_nbuf_copy(nbuf);
			status =
			   iot_sim_apply_content_change_rule(pdev,
							     piot_sim_rule,
							     isc->bcn_buf,
							     fixed_param_len,
							     param);
		} else {
			status =
			   iot_sim_apply_content_change_rule(pdev,
							     piot_sim_rule,
							     nbuf,
							     fixed_param_len,
							     param);
		}

		if (status == QDF_STATUS_E_NOSUPPORT) {
			if (deauth_disassoc && piot_sim_rule->drop) {
				status = QDF_STATUS_E_NULL_VALUE;
				iot_sim_debug("iot_sim: frame to be dropped");
			} else
				goto norule;
		}
	} else {
		status = iot_sim_apply_delay_drop_rule(piot_sim_rule,
						       nbuf, rx_param,
						       isc, mac_addr);
		if (QDF_IS_STATUS_SUCCESS(status))
			status = QDF_STATUS_E_NULL_VALUE;
		else
			status = QDF_STATUS_SUCCESS;
	}

	qdf_spin_unlock_bh(&isc->iot_sim_lock);
	return status;

norule:
	iot_sim_debug("Rule not set for this frame");
	qdf_spin_unlock_bh(&isc->iot_sim_lock);
	return QDF_STATUS_SUCCESS;
no_peer_rule:
	iot_sim_debug("Rule not set for this peer");
	return QDF_STATUS_SUCCESS;
}

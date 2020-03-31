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
#include <wlan_iot_sim_tgt_api.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#define IEEE80211_FRAME_BODY_OFFSET 0x18

QDF_STATUS iot_sim_cmd_handler(struct wlan_objmgr_vdev *vdev, qdf_nbuf_t nbuf)
{
	struct wlan_objmgr_pdev *pdev = vdev->vdev_objmgr.wlan_pdev;
	u8 type, subtype, seq = 0;
	struct iot_sim_context *isc;
	u8 *buf = qdf_nbuf_data(nbuf);

	type = (buf[0] & IEEE80211_FC0_TYPE_MASK) >> IEEE80211_FC0_TYPE_SHIFT;
	subtype = (buf[0] & IEEE80211_FC0_SUBTYPE_MASK);
	subtype >>= IEEE80211_FC0_SUBTYPE_SHIFT;
	isc = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_IOT_SIM_COMP);

	if (!isc) {
		iot_sim_err("pdev IOT_SIM object is NULL!");
		return QDF_STATUS_SUCCESS;
	}

	if (type == 0x00 && subtype == 0xb0) {
	/* Authentication frame */
		int auth_seq_index = IEEE80211_FRAME_BODY_OFFSET + 2;

		seq = le16toh(*(u_int16_t *)(buf + auth_seq_index));
	}

	/* Only broadcast peer is getting handled right now.
	 * Need to add support for peer based content modification
	 */
	if ((isc->bcast_peer.rule_per_seq[seq]) &&
	    (isc->bcast_peer.rule_per_seq[seq]->rule_per_type[type][subtype])) {
		if (isc->bcast_peer.rule_per_seq[seq]->
		    rule_per_type[type][subtype]) {
			struct iot_sim_rule *piot_sim_rule =
			isc->bcast_peer.rule_per_seq[seq]->
			rule_per_type[type][subtype];
			qdf_size_t buf_len = qdf_nbuf_len(nbuf);

			if (piot_sim_rule->frm_content && piot_sim_rule->len) {
				int offset;

				if (piot_sim_rule->offset ==
				    IEEE80211_FRAME_BODY_OFFSET) {
					offset = IEEE80211_FRAME_BODY_OFFSET;
				} else if (buf[piot_sim_rule->offset] ==
					   piot_sim_rule->frm_content[0]) {
					offset = piot_sim_rule->offset;
				} else if (piot_sim_rule->offset == 0) {
					offset = 0;
				}  else {
					offset = IEEE80211_FRAME_BODY_OFFSET;
					while (((offset + 1) < buf_len) &&
					       (buf[offset] <
					       piot_sim_rule->frm_content[0])) {
						offset += buf[offset + 1] + 2;
					}
				}

				if (offset <= buf_len) {
					buf += offset;
					qdf_mem_copy(buf,
						     piot_sim_rule->
						     frm_content,
						     piot_sim_rule->len);
					qdf_nbuf_set_pktlen(nbuf,
							    offset +
							    piot_sim_rule->len);
					iot_sim_err("Content updated  for peer");
					iot_sim_err("frame type:%d, subtype:%d",
						    type, subtype);
					iot_sim_err("seq:%d", seq);
				} else {
					iot_sim_err("Failed to modify content");
					iot_sim_err("type:%d, subtype:%d",
						    type, subtype);
					iot_sim_err("seq:%d", seq);
				}
			} else {
				iot_sim_err("Content update rule not set");
				iot_sim_err("frame type:%d, subtype:%d, seq:%d",
					    type, subtype, seq);
			}
		} else {
			iot_sim_err("Content update rule not set");
			iot_sim_err("frame type:%d, subtype:%d, seq:%d",
				    type, subtype, seq);
		}
	} else {
		iot_sim_err("Content update rule not set for peer frame");
		iot_sim_err("type:%d, subtype:%d, seq:%d", type, subtype, seq);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_iot_sim_init(void)
{
	if (wlan_objmgr_register_pdev_create_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_destroy_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_iot_sim_deinit(void)
{
	if (wlan_objmgr_unregister_pdev_create_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_IOT_SIM_COMP,
		wlan_iot_sim_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wlan_lmac_if_iot_sim_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_iot_sim_rx_ops *iot_sim_ops =
						&rx_ops->iot_sim_rx_ops;

	iot_sim_ops->iot_sim_cmd_handler = iot_sim_cmd_handler;
}

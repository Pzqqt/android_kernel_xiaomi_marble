/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This implementation of init/deint functions for FTM services.
 */

#include <wlan_ftm_ucfg_api.h>
#include <wlan_cfg80211_ftm.h>
#include "../../core/src/wlan_ftm_svc_i.h"
#include <wlan_cmn.h>
#include <qdf_module.h>

QDF_STATUS ucfg_wlan_ftm_testmode_cmd(struct wlan_objmgr_pdev *pdev,
					uint8_t *data, uint32_t len)
{
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;
	uint8_t pdev_id;

	ftm_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							WLAN_UMAC_COMP_FTM);
	if (!ftm_pdev_obj) {
		ftm_err("Failed to get ftm pdev component");
		return QDF_STATUS_E_FAILURE;
	}

	ftm_pdev_obj->length = 0;
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	return wlan_ftm_cmd_send(pdev, data, len, pdev_id);
}

QDF_STATUS
wlan_ftm_process_utf_event(struct wlan_objmgr_pdev *pdev,
			    uint8_t *event_buf, uint32_t len)
{
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;
	uint32_t utf_datalen;
	uint8_t *utf_data;
	struct ftm_seg_hdr_info seghdr_info;
	u_int8_t total_segments, current_seq;

	ftm_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_FTM);
	if (!ftm_pdev_obj) {
		ftm_err("Failed to get ftm pdev component");
		return QDF_STATUS_E_FAILURE;
	}

	utf_data = event_buf;
	seghdr_info = *(struct ftm_seg_hdr_info *)(event_buf);
	ftm_pdev_obj->current_seq = (seghdr_info.segment_info & 0xF);

	current_seq = (seghdr_info.segment_info & 0xF);
	total_segments = (seghdr_info.segment_info >> 4) & 0xF;

	utf_datalen = len - sizeof(seghdr_info);

	if (current_seq == 0) {
		ftm_pdev_obj->expected_seq = 0;
		ftm_pdev_obj->offset = 0;
	} else {
		if (ftm_pdev_obj->expected_seq != current_seq) {
			ftm_debug("seq mismatch exp Seq %d got seq %d\n",
				ftm_pdev_obj->expected_seq, current_seq);
		}
	}

	if ((len > FTM_CMD_MAX_BUF_LENGTH) ||
	    (ftm_pdev_obj->offset > (FTM_CMD_MAX_BUF_LENGTH - utf_datalen))) {
		ftm_err("Invalid utf data len :%d", len);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(&ftm_pdev_obj->data[ftm_pdev_obj->offset],
			&utf_data[sizeof(seghdr_info)], utf_datalen);

	ftm_pdev_obj->offset = ftm_pdev_obj->offset + utf_datalen;
	ftm_pdev_obj->expected_seq++;

	if (ftm_pdev_obj->expected_seq == total_segments) {
		if (ftm_pdev_obj->offset != seghdr_info.len) {
			ftm_debug("len mismatch len %zu total len %d\n",
				ftm_pdev_obj->offset, seghdr_info.len);
		}

		ftm_pdev_obj->length = ftm_pdev_obj->offset;

		/**
		 * If the repsonse is for a command from FTM daemon,
		 * send this repsonse data to cfg80211
		 */
		if (ftm_pdev_obj->cmd_type == WIFI_FTM_CMD_NL80211) {
			if (wlan_cfg80211_ftm_rx_event(pdev, ftm_pdev_obj->data,
				ftm_pdev_obj->length) != QDF_STATUS_SUCCESS) {
				return QDF_STATUS_E_FAILURE;
			}
			ftm_pdev_obj->cmd_type = WIFI_FTM_CMD_UNKNOWN;
		}
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_WIFI_FTM_IOCTL
QDF_STATUS ucfg_wlan_ftm_testmode_rsp(struct wlan_objmgr_pdev *pdev,
				      uint8_t *data)
{
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;
	uint32_t *len;

	ftm_pdev_obj =
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
						      WLAN_UMAC_COMP_FTM);
	if (!ftm_pdev_obj) {
		ftm_err("Failed to get ftm pdev component");
		return QDF_STATUS_E_FAILURE;
	}

	if (ftm_pdev_obj->length) {
		len = (uint32_t *)data;
		*len = ftm_pdev_obj->length;
		qdf_mem_copy((data + 4), ftm_pdev_obj->data,
			     ftm_pdev_obj->length);

		ftm_pdev_obj->length = 0;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#endif

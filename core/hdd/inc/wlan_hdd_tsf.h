/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined WLAN_HDD_TSF_H
#define WLAN_HDD_TSF_H

/**
 * enum hdd_tsf_get_state - status of get tsf action
 * @TSF_RETURN:                   get tsf
 * @TSF_STA_NOT_CONNECTED_NO_TSF: sta not connected to ap
 * @TSF_NOT_RETURNED_BY_FW:       fw not returned tsf
 * @TSF_CURRENT_IN_CAP_STATE:     driver in capture state
 * @TSF_CAPTURE_FAIL:             capture fail
 * @TSF_GET_FAIL:                 get fail
 * @TSF_RESET_GPIO_FAIL:          GPIO reset fail
 * @TSF_SAP_NOT_STARTED_NO_TSF    SAP not started
 */
enum hdd_tsf_get_state {
	TSF_RETURN = 0,
	TSF_STA_NOT_CONNECTED_NO_TSF,
	TSF_NOT_RETURNED_BY_FW,
	TSF_CURRENT_IN_CAP_STATE,
	TSF_CAPTURE_FAIL,
	TSF_GET_FAIL,
	TSF_RESET_GPIO_FAIL,
	TSF_SAP_NOT_STARTED_NO_TSF
};

/**
 * enum hdd_tsf_capture_state - status of capture
 * @TSF_IDLE:      idle
 * @TSF_CAP_STATE: current is in capture state
 */
enum hdd_tsf_capture_state {
	TSF_IDLE = 0,
	TSF_CAP_STATE
};

#ifdef WLAN_FEATURE_TSF
void wlan_hdd_tsf_init(struct hdd_context_s *hdd_ctx);
int hdd_capture_tsf(struct hdd_adapter_s *adapter, uint32_t *buf, int len);
int hdd_indicate_tsf(struct hdd_adapter_s *adapter, uint32_t *buf, int len);
int wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len);
int hdd_get_tsf_cb(void *pcb_cxt, struct stsf *ptsf);
#else
static inline void wlan_hdd_tsf_init(struct hdd_context_s *hdd_ctx)
{
	return;
}

static inline int hdd_indicate_tsf(struct hdd_adapter_s *adapter, uint32_t *buf,
				int len)
{
	return -ENOTSUPP;
}

static inline int
hdd_capture_tsf(struct hdd_adapter_s *adapter, uint32_t *buf, int len)
{
	return -ENOTSUPP;
}

static inline int wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	return -ENOTSUPP;
}
static inline int hdd_get_tsf_cb(void *pcb_cxt, struct stsf *ptsf)
{
	return -ENOTSUPP;
}

#endif

#endif

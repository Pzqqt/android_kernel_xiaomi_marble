/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_wifi_pos.c
 * This file defines the important functions pertinent to wifi positioning
 * component's os_if layer.
 */

#include "wlan_nlink_srv.h"
#include "wlan_ptt_sock_svc.h"
#include "wlan_nlink_common.h"
#include "os_if_wifi_pos.h"
#include "wifi_pos_api.h"
#include "wlan_cfg80211.h"
#ifdef CNSS_GENL
#include <net/cnss_nl.h>
#endif

/**
 * os_if_wifi_pos_callback() - callback registered with NL service socket to
 * process wifi pos request
 * @skb: request message sk_buff
 *
 * Return: status of operation
 */
#ifdef CNSS_GENL
static void os_if_wifi_pos_callback(const void *data, int data_len,
				    void *ctx, int pid)
{
	QDF_STATUS status;
	struct wifi_pos_req_msg req = {0};
	struct wlan_objmgr_psoc *psoc = wifi_pos_get_psoc();

	if (!psoc) {
		cfg80211_err("global psoc object not registered yet.");
		return;
	}

	wlan_objmgr_psoc_get_ref(psoc, WLAN_WIFI_POS_ID);

	/* implemention is TBD */
	status = ucfg_wifi_pos_process_req(psoc, &req, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		cfg80211_err("ucfg_wifi_pos_process_req failed. status: %d",
				status);

	wlan_objmgr_psoc_release_ref(psoc, WLAN_WIFI_POS_ID);
}

int os_if_wifi_pos_register_nl(void)
{
	int ret = register_cld_cmd_cb(WLAN_NL_MSG_OEM,
				os_if_wifi_pos_callback, NULL);
	if (ret)
		cfg80211_err("register_cld_cmd_cb failed")

	return ret;
}

int os_if_wifi_pos_deregister_nl(void)
{
	int ret = deregister_cld_cmd_cb(WLAN_NL_MSG_OEM);
	if (ret)
		cfg80211_err("deregister_cld_cmd_cb failed")

	return ret;
}
#else
int os_if_wifi_pos_register_nl(void)
{
	return 0;
}

int os_if_wifi_pos_deregister_nl(void)
{
	return 0;
}
#endif

void os_if_wifi_pos_send_peer_status(struct qdf_mac_addr *peer_mac,
				uint8_t peer_status,
				uint8_t peer_timing_meas_cap,
				uint8_t session_id,
				struct wifi_pos_ch_info *chan_info,
				enum tQDF_ADAPTER_MODE dev_mode)
{
	/* implemention TBD */
}

int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				   struct wifi_pos_driver_caps *caps)
{
	if (!psoc || !caps) {
		cfg80211_err("psoc or caps buffer is null");
		return -EINVAL;
	}

	return qdf_status_to_os_return(wifi_pos_populate_caps(psoc, caps));
}

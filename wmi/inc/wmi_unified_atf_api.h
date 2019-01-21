/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for ATF
 */

#ifndef _WMI_UNIFIED_ATF_API_H_
#define _WMI_UNIFIED_ATF_API_H_

#include "wmi_unified_atf_param.h"

QDF_STATUS wmi_unified_set_bwf_cmd_send(void *wmi_hdl,
				struct set_bwf_params *param);

#ifdef WLAN_ATF_ENABLE
QDF_STATUS
wmi_unified_set_atf_cmd_send(void *wmi_hdl,
			     struct set_atf_params *param);

QDF_STATUS
wmi_send_atf_peer_request_cmd(void *wmi_hdl,
			      struct atf_peer_request_params *param);

QDF_STATUS
wmi_send_set_atf_grouping_cmd(void *wmi_hdl,
			      struct atf_grouping_params *param);

QDF_STATUS
wmi_send_set_atf_group_ac_cmd(void *wmi_hdl,
			      struct atf_group_ac_params *param);

QDF_STATUS
wmi_extract_atf_peer_stats_ev(void *wmi_hdl, void *evt_buf,
			      wmi_host_atf_peer_stats_event *ev);

QDF_STATUS
wmi_extract_atf_token_info_ev(void *wmi_hdl, void *evt_buf, uint8_t idx,
			      wmi_host_atf_peer_stats_info *atf_token_info);
#endif

void wmi_atf_attach_tlv(struct wmi_unified *wmi_handle);
#endif /* _WMI_UNIFIED_ATF_API_H_ */

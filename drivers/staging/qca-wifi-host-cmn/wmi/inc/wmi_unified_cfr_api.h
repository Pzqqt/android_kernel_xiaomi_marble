/*
 * Copyright (c) 2019, 2021 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_CFR_API_H_
#define _WMI_UNIFIED_CFR_API_H_

#include "wmi_unified_param.h"
#include "wmi_unified_cfr_param.h"

#ifdef WLAN_CFR_ENABLE
/**
 * wmi_unified_send_peer_cfr_capture_cmd() - WMI function to start CFR capture
 * for a peer
 * @wmi_handle: WMI handle
 * @param: configuration params for capture
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_unified_send_peer_cfr_capture_cmd(wmi_unified_t wmi_handle,
				      struct peer_cfr_params *param);
/**
 * wmi_extract_cfr_peer_tx_event_param() - WMI function to extract cfr tx event
 * for a peer
 * @wmi_handle: WMI handle
 * @evt_buf: Buffer holding event data
 * @peer_tx_event: pointer to hold tx event data
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_extract_cfr_peer_tx_event_param(wmi_unified_t wmi_handle, void *evt_buf,
				    wmi_cfr_peer_tx_event_param *peer_tx_event);

#ifdef WLAN_ENH_CFR_ENABLE
/**
 * wmi_unified_send_cfr_rcc_cmd() - WMI function to send CFR RCC param
 * @wmi_handle: WMI handle
 * @cfg: pointer to RCC param
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS wmi_unified_send_cfr_rcc_cmd(wmi_unified_t wmi_handle,
					struct cfr_rcc_param *cfg);

/**
 * wmi_extract_cfr_pdev_phase_delta_event() - WMI function to extract the
 * phase delta information.
 * @wmi_handle: WMI handle
 * @evt_buf: Buffer holding the event data
 * @param: phase delta params to be updated from event
 *
 * Return: QDF_STATUS_SUCCESS if success, else returns proper error code.
 */
QDF_STATUS
wmi_extract_cfr_pdev_phase_delta_event(wmi_unified_t wmi_handle,
				       void *evt_buf,
				       struct wmi_cfr_phase_delta_param *param);
#endif
#endif /* WLAN_CFR_ENABLE */
#endif /* _WMI_UNIFIED_CFR_API_H_ */

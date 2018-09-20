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
/**
 * DOC: Implement API's specific to P2P component.
 */

#ifndef _WMI_UNIFIED_P2P_API_H_
#define _WMI_UNIFIED_P2P_API_H_

#include <wlan_p2p_public_struct.h>

/**
 * wmi_unified_set_p2pgo_oppps_req() - send p2p go opp power save request to fw
 * @wmi_hdl: wmi handle
 * @opps: p2p opp power save parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_p2pgo_oppps_req(void *wmi_hdl,
					   struct p2p_ps_params *oppps);

/**
 * wmi_unified_set_p2pgo_noa_req_cmd() - send p2p go noa request to fw
 * @wmi_hdl: wmi handle
 * @noa: p2p power save parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_p2pgo_noa_req_cmd(void *wmi_hdl,
					     struct p2p_ps_params *noa);

/**
 * wmi_extract_p2p_noa_ev_param() - extract p2p noa param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold p2p noa param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_p2p_noa_ev_param(void *wmi_hdl,
					void *evt_buf,
					struct p2p_noa_info *param);

/**
 * wmi_send_set_mac_addr_rx_filter_cmd() - set mac addr rx filter cmd
 * @wmi_hdl: wmi handle
 * @param: Pointer to set mac filter struct
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_mac_addr_rx_filter_cmd(void *wmi_hdl,
				    struct p2p_set_mac_filter *param);

/**
 * wmi_extract_mac_addr_rx_filter_evt_param() - extract mac addr rx filter evt
 * @wmi_hdl: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to extracted evt info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mac_addr_rx_filter_evt_param(void *wmi_hdl, void *evt_buf,
					 struct p2p_set_mac_filter_evt *param);

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
/**
 * wmi_unified_p2p_lo_start_cmd() - send p2p lo start request to fw
 * @wmi_hdl: wmi handle
 * @param: p2p listen offload start parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_lo_start_cmd(void *wmi_hdl,
					struct p2p_lo_start *param);

/**
 * wmi_unified_p2p_lo_stop_cmd() - send p2p lo stop request to fw
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_p2p_lo_stop_cmd(void *wmi_hdl, uint8_t vdev_id);

/**
 * wmi_extract_p2p_lo_stop_ev_param() - extract p2p lo stop param from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @param: Pointer to hold listen offload stop param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_p2p_lo_stop_ev_param(void *wmi_hdl,
					    void *evt_buf,
					    struct p2p_lo_event *param);
#endif /* FEATURE_P2P_LISTEN_OFFLOAD */

#endif /* _WMI_UNIFIED_P2P_API_H_ */

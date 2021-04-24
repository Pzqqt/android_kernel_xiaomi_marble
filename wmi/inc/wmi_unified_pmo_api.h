/*
 * Copyright (c) 2017-2019, 2021 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) which are specific to Power management offloads (PMO).
 */

#ifndef _WMI_UNIFIED_PMO_API_H_
#define _WMI_UNIFIED_PMO_API_H_

#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_arp_public_struct.h"
#include "wlan_pmo_ns_public_struct.h"
#include "wlan_pmo_gtk_public_struct.h"
#include "wlan_pmo_wow_public_struct.h"
#include "wlan_pmo_pkt_filter_public_struct.h"

#ifdef FEATURE_WLAN_D0WOW
/**
 *  wmi_unified_d0wow_enable_send() - WMI d0 wow enable function
 *  @wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_d0wow_enable_send(wmi_unified_t wmi_handle,
					 uint8_t mac_id);

/**
 *  wmi_unified_d0wow_disable_send() - WMI d0 wow disable function
 *  @wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_d0wow_disable_send(wmi_unified_t wmi_handle,
					  uint8_t mac_id);
#endif /* FEATURE_WLAN_D0WOW */

/**
 * wmi_unified_add_wow_wakeup_event_cmd() -  Configures wow wakeup events.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_wow_wakeup_event_cmd(wmi_unified_t wmi_handle,
						uint32_t vdev_id,
						uint32_t *bitmap,
						bool enable);

/**
 * wmi_unified_wow_patterns_to_fw_cmd() - Sends WOW patterns to FW.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @ptrn_id: pattern id
 * @ptrn: pattern
 * @ptrn_len: pattern length
 * @ptrn_offset: pattern offset
 * @mask: mask
 * @mask_len: mask length
 * @user: true for user configured pattern and false for default pattern
 * @default_patterns: default patterns
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_patterns_to_fw_cmd(
				wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns);

/**
 * wmi_unified_add_clear_mcbc_filter_cmd() - set mcast filter command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @multicastAddr: mcast address
 * @clear_list: clear list flag
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_clear_mcbc_filter_cmd(
					wmi_unified_t wmi_handle,
					uint8_t vdev_id,
					struct qdf_mac_addr multicast_addr,
					bool clear_list);

/**
 * wmi_unified_multiple_add_clear_mcbc_filter_cmd() - send multiple mcast
 *						      filter command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @filter_params: mcast filter params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_multiple_add_clear_mcbc_filter_cmd(
				wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				struct pmo_mcast_filter_params *filter_param);

#ifdef FEATURE_WLAN_RA_FILTERING
/**
 * wmi_unified_wow_sta_ra_filter_cmd() - set RA filter pattern in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @default_pattern: default pattern id
 * @rate_limit_interval: rate limit interval
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_sta_ra_filter_cmd(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     uint8_t default_pattern,
					     uint16_t rate_limit_interval);
#endif

/**
 * wmi_unified_enable_enhance_multicast_offload() - enhance multicast offload
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @action: true for enable else false
 *
 * To configure enhance multicast offload in to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_enhance_multicast_offload_cmd(
		wmi_unified_t wmi_handle, uint8_t vdev_id, bool action);

/**
 * wmi_extract_gtk_rsp_event() - extract gtk rsp params from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @gtk_rsp_param: Pointer to gtk rsp parameters
 * @ len: len of gtk rsp event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_gtk_rsp_event(
	wmi_unified_t wmi_hdl, void *evt_buf,
	struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len);

/**
 * wmi_unified_process_gtk_offload_getinfo_cmd() - send GTK offload cmd to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @offload_req_opcode: gtk offload flag
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_gtk_offload_getinfo_cmd(
				wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				uint64_t offload_req_opcode);

/**
 * wmi_unified_action_frame_patterns_cmd() - send action filter wmi cmd
 * @wmi_handle: wmi handler
 * @action_params: pointer to action_params
 *
 * Return: 0 for success, otherwise appropriate error code
 */
QDF_STATUS wmi_unified_action_frame_patterns_cmd(
			wmi_unified_t wmi_handle,
			struct pmo_action_wakeup_set_params *action_params);

/**
 * wmi_unified_send_gtk_offload_cmd() - send GTK offload command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @params: GTK offload parameters
 * @enable_offload: flag to enable offload
 * @gtk_offload_opcode: gtk offload flag
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_gtk_offload_cmd(wmi_unified_t wmi_handle,
					    uint8_t vdev_id,
					    struct pmo_gtk_req *params,
					    bool enable_offload,
					    uint32_t gtk_offload_opcode);

/**
 * wmi_unified_enable_arp_ns_offload_cmd() - enable ARP NS offload
 * @wmi_hdl: wmi handle
 * @arp_offload_req: arp offload request
 * @ns_offload_req: ns offload request
 * @vdev_id: vdev id
 *
 * To configure ARP NS off load data to firmware
 * when target goes to wow mode.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_arp_ns_offload_cmd(
			wmi_unified_t wmi_handle,
			struct pmo_arp_offload_params *arp_offload_req,
			struct pmo_ns_offload_params *ns_offload_req,
			uint8_t vdev_id);

/**
 * wmi_unified_conf_hw_filter_cmd() - Configure hardware filter in DTIM mode
 * @wmi_handle: wmi handle
 * @req: request parameters to configure to firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_conf_hw_filter_cmd(wmi_unified_t wmi_handle,
					  struct pmo_hw_filter_params *req);

#ifdef FEATURE_WLAN_LPHB
/**
 * wmi_unified_lphb_config_hbenable_cmd() - enable command of LPHB configuration
 * @wmi_handle: wmi handle
 * @params: configuration info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_hbenable_cmd(
		wmi_unified_t wmi_handle,
		wmi_hb_set_enable_cmd_fixed_param *params);

/**
 * wmi_unified_lphb_config_tcp_params_cmd() - set tcp params of LPHB config req
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_tcp_params_cmd(
		wmi_unified_t wmi_handle,
		wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req);

/**
 * wmi_unified_lphb_config_tcp_pkt_filter_cmd() - config LPHB tcp packet filter
 * @wmi_handle: wmi handle
 * @g_hb_tcp_filter_fp: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_tcp_pkt_filter_cmd(
		wmi_unified_t wmi_handle,
		wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp);

/**
 * wmi_unified_lphb_config_udp_params_cmd() - configure LPHB udp param command
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_udp_params_cmd(
		wmi_unified_t wmi_handle,
		wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req);

/**
 * wmi_unified_lphb_config_udp_pkt_filter_cmd() - configure LPHB udp pkt filter
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_udp_pkt_filter_cmd(
		wmi_unified_t wmi_handle,
		wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req);
#endif /* FEATURE_WLAN_LPHB */

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * wmi_unified_enable_disable_packet_filter_cmd() - enable/disable packet filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @enable: Flag to enable/disable packet filter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_enable_disable_packet_filter_cmd(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     bool enable);

/**
 * wmi_unified_config_packet_filter_cmd() - configure packet filter in target
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @rcv_filter_param: Packet filter parameters
 * @filter_id: Filter id
 * @enable: Flag to add/delete packet filter configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_config_packet_filter_cmd(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable);
#endif /* WLAN_FEATURE_PACKET_FILTERING */

/**
 * wmi_unified_wow_delete_pattern_cmd() - delete wow pattern in target
 * @wmi_handle: wmi handle
 * @ptrn_id: pattern id
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_delete_pattern_cmd(wmi_unified_t wmi_handle,
					      uint8_t ptrn_id,
					      uint8_t vdev_id);

/**
 * wmi_unified_host_wakeup_ind_to_fw_cmd() - send wakeup ind to fw
 * @wmi_handle: wmi handle
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_host_wakeup_ind_to_fw_cmd(wmi_unified_t wmi_handle);

/**
 * wmi_unified_wow_timer_pattern_cmd() - set timer pattern tlv, so that
 *					 firmware will wake up host after
 *					 specified time is elapsed
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @cookie: value to identify reason why host set up wake call.
 * @time: time in ms
 *
 * Return: QDF status
 */
QDF_STATUS wmi_unified_wow_timer_pattern_cmd(wmi_unified_t wmi_handle,
					     uint8_t vdev_id,
					     uint32_t cookie, uint32_t time);

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/**
 * wmi_unified_enable_ext_wow_cmd() - enable ext wow in fw
 * @wmi_handle: wmi handle
 * @params: ext wow params
 *
 * Return:QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_ext_wow_cmd(wmi_unified_t wmi_handle,
					  struct ext_wow_params *params);

/**
 * wmi_unified_set_app_type2_params_in_fw_cmd() - set app type2 params in fw
 * @wmi_handle: wmi handle
 * @appType2Params: app type2 params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_app_type2_params_in_fw_cmd(
				wmi_unified_t wmi_handle,
				struct app_type2_params *appType2Params);

/**
 * wmi_unified_app_type1_params_in_fw_cmd() - set app type1 params in fw
 * @wmi_hdl: wmi handle
 * @app_type1_params: app type1 params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_app_type1_params_in_fw_cmd(
				wmi_unified_t wmi_handle,
				struct app_type1_params *app_type1_params);
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

#ifdef WLAN_FEATURE_IGMP_OFFLOAD
/**
 * wmi_unified_send_igmp_offload_cmd() - send igmp offload cmd to fw
 * @wmi_hdl: wmi handle
 * @pmo_igmp_req: igmp params
 *
 * Return: Success or failure
 */
QDF_STATUS
wmi_unified_send_igmp_offload_cmd(wmi_unified_t wmi_handle,
				  struct pmo_igmp_offload_req *pmo_igmp_req);
#endif
#endif /* _WMI_UNIFIED_PMO_API_H_ */

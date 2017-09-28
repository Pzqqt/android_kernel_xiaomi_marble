/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * wmi_unified_add_wow_wakeup_event_cmd() -  Configures wow wakeup events.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_wow_wakeup_event_cmd(void *wmi_hdl,
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
QDF_STATUS wmi_unified_wow_patterns_to_fw_cmd(void *wmi_hdl,
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
 * @clearList: clear list flag
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_add_clear_mcbc_filter_cmd(void *wmi_hdl,
					 uint8_t vdev_id,
					 struct qdf_mac_addr multicast_addr,
					 bool clearList);

/**
 * wmi_unified_multiple_add_clear_mcbc_filter_cmd() - send multiple mcast
 *						      filter command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @mcast_filter_params: mcast filter params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_multiple_add_clear_mcbc_filter_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				struct pmo_mcast_filter_params *filter_param);

/**
 * wmi_unified_wow_sta_ra_filter_cmd() - set RA filter pattern in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_wow_sta_ra_filter_cmd(void *wmi_hdl,
			  uint8_t vdev_id, uint8_t default_pattern,
			  uint16_t rate_limit_interval);

/**
 * wmi_unified_enable_enhance_multicast_offload() - enhance multicast offload
 * @wmi_hdl: wmi handle
 * @vdev_id: vdev id
 * @action: true for enable else false
 *
 * To configure enhance multicast offload in to firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_enhance_multicast_offload_cmd(
		void *wmi_hdl, uint8_t vdev_id, bool action);

/**
 * wmi_extract_gtk_rsp_event() - extract gtk rsp params from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @gtk_rsp_param: Pointer to gtk rsp parameters
 * @ len: len of gtk rsp event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_gtk_rsp_event(void *wmi_hdl, void *evt_buf,
	struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len);

/**
 * wmi_unified_process_gtk_offload_getinfo_cmd() - send GTK offload cmd to fw
 * @wmi_handle: wmi handle
 * @params: GTK offload params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_process_gtk_offload_getinfo_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				uint64_t offload_req_opcode);

/**
 * wmi_unified_action_frame_patterns_cmd() - send action filter wmi cmd
 * @wmi_handle: wmi handler
 * @action_params: pointer to action_params
 *
 * Return: 0 for success, otherwise appropriate error code
 */
QDF_STATUS wmi_unified_action_frame_patterns_cmd(void *wmi_hdl,
			struct pmo_action_wakeup_set_params *action_params);

/**
 * wmi_unified_send_gtk_offload_cmd() - send GTK offload command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @params: GTK offload parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_send_gtk_offload_cmd(void *wmi_hdl, uint8_t vdev_id,
					   struct pmo_gtk_req *params,
					   bool enable_offload,
					   uint32_t gtk_offload_opcode);

/**
 * wmi_unified_enable_arp_ns_offload_cmd() - enable ARP NS offload
 * @wmi_hdl: wmi handle
 * @param: offload request
 *
 * To configure ARP NS off load data to firmware
 * when target goes to wow mode.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_arp_ns_offload_cmd(void *wmi_hdl,
			   struct pmo_arp_offload_params *arp_offload_req,
			   struct pmo_ns_offload_params *ns_offload_req,
			   uint8_t vdev_id);

/**
 * wmi_unified_conf_hw_filter_cmd() - Configure hardware filter in DTIM mode
 * @opaque_wmi: wmi handle
 * @req: request parameters to configure to firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_unified_conf_hw_filter_cmd(void *opaque_wmi,
					  struct pmo_hw_filter_params *req);

/**
 * wmi_unified_lphb_config_hbenable_cmd() - enable command of LPHB configuration
 * @wmi_handle: wmi handle
 * @lphb_conf_req: configuration info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_hbenable_cmd(void *wmi_hdl,
				wmi_hb_set_enable_cmd_fixed_param *params);

/**
 * wmi_unified_lphb_config_tcp_params_cmd() - set tcp params of LPHB config req
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_tcp_params_cmd(void *wmi_hdl,
		    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req);

/**
 * wmi_unified_lphb_config_tcp_pkt_filter_cmd() - config LPHB tcp packet filter
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_tcp_pkt_filter_cmd(void *wmi_hdl,
		wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp);

/**
 * wmi_unified_lphb_config_udp_params_cmd() - configure LPHB udp param command
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_udp_params_cmd(void *wmi_hdl,
		    wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req);

/**
 * wmi_unified_lphb_config_udp_pkt_filter_cmd() - configure LPHB udp pkt filter
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_lphb_config_udp_pkt_filter_cmd(void *wmi_hdl,
		wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req);

/**
 * wmi_unified_enable_disable_packet_filter_cmd() - enable/disable packet filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @enable: Flag to enable/disable packet filter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_enable_disable_packet_filter_cmd(void *wmi_hdl,
		uint8_t vdev_id, bool enable);

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
QDF_STATUS wmi_unified_config_packet_filter_cmd(void *wmi_hdl,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable);

#endif /* _WMI_UNIFIED_PMO_API_H_ */

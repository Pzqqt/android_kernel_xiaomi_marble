/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to PMO component.
 */

#include "ol_if_athvar.h"
#include "ol_defines.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_pmo_api.h"
#include "wlan_pmo_hw_filter_public_struct.h"

#ifdef FEATURE_WLAN_D0WOW
QDF_STATUS wmi_unified_d0wow_enable_send(void *wmi_hdl,
					 uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_d0wow_enable_cmd)
		return wmi_handle->ops->send_d0wow_enable_cmd(wmi_handle,
							      mac_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_d0wow_disable_send(void *wmi_hdl,
					  uint8_t mac_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_d0wow_disable_cmd)
		return wmi_handle->ops->send_d0wow_disable_cmd(wmi_handle,
							       mac_id);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_D0WOW */

QDF_STATUS wmi_unified_add_wow_wakeup_event_cmd(void *wmi_hdl,
						uint32_t vdev_id,
						uint32_t *bitmap,
						bool enable)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->send_add_wow_wakeup_event_cmd)
		return wmi_handle->ops->send_add_wow_wakeup_event_cmd(
				wmi_handle, vdev_id, bitmap, enable);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_wow_patterns_to_fw_cmd(void *wmi_hdl,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *) wmi_hdl;

	if (wmi_handle->ops->send_wow_patterns_to_fw_cmd)
		return wmi_handle->ops->send_wow_patterns_to_fw_cmd(wmi_handle,
				vdev_id, ptrn_id, ptrn,
				ptrn_len, ptrn_offset, mask,
				mask_len, user, default_patterns);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_enable_arp_ns_offload_cmd(void *wmi_hdl,
			   struct pmo_arp_offload_params *arp_offload_req,
			   struct pmo_ns_offload_params *ns_offload_req,
			   uint8_t vdev_id)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_enable_arp_ns_offload_cmd)
		return wmi_handle->ops->send_enable_arp_ns_offload_cmd(
				wmi_handle,
				arp_offload_req, ns_offload_req, vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_add_clear_mcbc_filter_cmd(void *wmi_hdl,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_add_clear_mcbc_filter_cmd)
		return wmi_handle->ops->send_add_clear_mcbc_filter_cmd(
				wmi_handle, vdev_id, multicast_addr, clearList);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_multiple_add_clear_mcbc_filter_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				struct pmo_mcast_filter_params *filter_param)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_multiple_add_clear_mcbc_filter_cmd)
		return wmi_handle->ops->send_multiple_add_clear_mcbc_filter_cmd(
				wmi_handle, vdev_id, filter_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_conf_hw_filter_cmd(void *opaque_wmi,
					  struct pmo_hw_filter_params *req)
{
	struct wmi_unified *wmi = opaque_wmi;

	if (!wmi->ops->send_conf_hw_filter_cmd)
		return QDF_STATUS_E_NOSUPPORT;

	return wmi->ops->send_conf_hw_filter_cmd(wmi, req);
}

QDF_STATUS wmi_unified_send_gtk_offload_cmd(void *wmi_hdl, uint8_t vdev_id,
					    struct pmo_gtk_req  *params,
					    bool enable_offload,
					    uint32_t gtk_offload_opcode)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_gtk_offload_cmd)
		return wmi_handle->ops->send_gtk_offload_cmd(wmi_handle,
				vdev_id, params, enable_offload,
				gtk_offload_opcode);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_process_gtk_offload_getinfo_cmd(void *wmi_hdl,
				uint8_t vdev_id,
				uint64_t offload_req_opcode)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_process_gtk_offload_getinfo_cmd)
		return wmi_handle->ops->send_process_gtk_offload_getinfo_cmd(
				wmi_handle, vdev_id, offload_req_opcode);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_enable_enhance_multicast_offload_cmd(
		void *wmi_hdl, uint8_t vdev_id, bool action)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;
	struct wmi_ops *ops;

	ops = wmi_handle->ops;
	if (ops && ops->send_enable_enhance_multicast_offload_cmd)
		return ops->send_enable_enhance_multicast_offload_cmd(
			wmi_handle, vdev_id, action);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_gtk_rsp_event(void *wmi_hdl, void *evt_buf,
	struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->extract_gtk_rsp_event)
		return wmi_handle->ops->extract_gtk_rsp_event(wmi_handle,
				evt_buf, gtk_rsp_param, len);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_RA_FILTERING
QDF_STATUS wmi_unified_wow_sta_ra_filter_cmd(void *wmi_hdl,
				uint8_t vdev_id, uint8_t default_pattern,
				uint16_t rate_limit_interval)
{

	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_wow_sta_ra_filter_cmd)
		return wmi_handle->ops->send_wow_sta_ra_filter_cmd(wmi_handle,
			    vdev_id, default_pattern, rate_limit_interval);

	return QDF_STATUS_E_FAILURE;

}
#endif /* FEATURE_WLAN_RA_FILTERING */

QDF_STATUS wmi_unified_action_frame_patterns_cmd(void *wmi_hdl,
		struct pmo_action_wakeup_set_params *action_params)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_action_frame_patterns_cmd)
		return wmi_handle->ops->send_action_frame_patterns_cmd(
				wmi_handle, action_params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_LPHB
QDF_STATUS wmi_unified_lphb_config_hbenable_cmd(void *wmi_hdl,
				wmi_hb_set_enable_cmd_fixed_param *params)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_lphb_config_hbenable_cmd)
		return wmi_handle->ops->send_lphb_config_hbenable_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lphb_config_tcp_params_cmd(void *wmi_hdl,
		    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_lphb_config_tcp_params_cmd)
		return wmi_handle->ops->send_lphb_config_tcp_params_cmd(
				wmi_handle, lphb_conf_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lphb_config_tcp_pkt_filter_cmd(void *wmi_hdl,
		wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_lphb_config_tcp_pkt_filter_cmd)
		return wmi_handle->ops->send_lphb_config_tcp_pkt_filter_cmd(
				wmi_handle, g_hb_tcp_filter_fp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lphb_config_udp_params_cmd(void *wmi_hdl,
			wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_lphb_config_udp_params_cmd)
		return wmi_handle->ops->send_lphb_config_udp_params_cmd(
				wmi_handle, lphb_conf_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_lphb_config_udp_pkt_filter_cmd(void *wmi_hdl,
		wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->send_lphb_config_udp_pkt_filter_cmd)
		return wmi_handle->ops->send_lphb_config_udp_pkt_filter_cmd(
				wmi_handle, lphb_conf_req);

	return QDF_STATUS_E_FAILURE;
}
#endif /* FEATURE_WLAN_LPHB */

#ifdef WLAN_FEATURE_PACKET_FILTERING
QDF_STATUS wmi_unified_enable_disable_packet_filter_cmd(void *wmi_hdl,
		uint8_t vdev_id, bool enable)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_enable_disable_packet_filter_cmd)
		return wmi_handle->ops->send_enable_disable_packet_filter_cmd(
				wmi_handle, vdev_id, enable);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_config_packet_filter_cmd(void *wmi_hdl,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_config_packet_filter_cmd)
		return wmi_handle->ops->send_config_packet_filter_cmd(
		wmi_handle, vdev_id, rcv_filter_param, filter_id, enable);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_PACKET_FILTERING */

QDF_STATUS wmi_unified_wow_delete_pattern_cmd(void *wmi_hdl, uint8_t ptrn_id,
					uint8_t vdev_id)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_delete_pattern_cmd)
		return wmi_handle->ops->send_wow_delete_pattern_cmd(wmi_handle,
								    ptrn_id,
								    vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_host_wakeup_ind_to_fw_cmd(void *wmi_hdl)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_host_wakeup_ind_to_fw_cmd)
		return wmi_handle->ops->send_host_wakeup_ind_to_fw_cmd(wmi_handle);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_wow_timer_pattern_cmd(void *wmi_hdl, uint8_t vdev_id,
					     uint32_t cookie, uint32_t time)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_wow_timer_pattern_cmd)
		return wmi_handle->ops->send_wow_timer_pattern_cmd(wmi_handle,
							vdev_id, cookie, time);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
QDF_STATUS wmi_unified_enable_ext_wow_cmd(void *wmi_hdl,
					  struct ext_wow_params *params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_enable_ext_wow_cmd)
		return wmi_handle->ops->send_enable_ext_wow_cmd(wmi_handle,
								params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_app_type2_params_in_fw_cmd(void *wmi_hdl,
				struct app_type2_params *appType2Params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_app_type2_params_in_fw_cmd)
		return wmi_handle->ops->send_set_app_type2_params_in_fw_cmd(
				wmi_handle, appType2Params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_app_type1_params_in_fw_cmd(void *wmi_hdl,
				   struct app_type1_params *app_type1_params)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_app_type1_params_in_fw_cmd)
		return wmi_handle->ops->send_app_type1_params_in_fw_cmd(
				wmi_handle, app_type1_params);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */


/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */
#include <htc_api.h>
#include <htc_api.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_api.h>
#include <qdf_module.h>
#include <qdf_list.h>
#include <qdf_platform.h>
#ifdef WMI_EXT_DBG
#include <qdf_list.h>
#endif

#ifndef WMI_NON_TLV_SUPPORT
#include <wmi_tlv_helper.h>
#endif

#ifdef TGT_IF_VDEV_MGR_CONV

#include <linux/debugfs.h>
#include <wmi_unified_param.h>
#include <cdp_txrx_cmn_struct.h>
#include <wmi_unified_vdev_api.h>

QDF_STATUS
send_vdev_config_ratemask_cmd_tlv(struct wmi_unified *wmi_handle,
				  struct config_ratemask_params *param)
{
	wmi_vdev_config_ratemask_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_config_ratemask_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_config_ratemask_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_vdev_config_ratemask_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->type = param->type;
	cmd->mask_lower32 = param->lower32;
	cmd->mask_higher32 = param->higher32;
	cmd->mask_lower32_2 = param->lower32_2;
	WMI_LOGI("Setting vdev ratemask vdev id = 0x%X, type = 0x%X mask_l32 = 0x%X mask_h32 = 0x%X mask_l32_2 = 0x%X",
		 param->vdev_id, param->type, param->lower32,
		 param->higher32, param->lower32_2);

	wmi_mtrace(WMI_VDEV_RATEMASK_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_RATEMASK_CMDID)) {
		WMI_LOGE("Seting vdev ratemask failed");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
send_beacon_send_cmd_tlv(struct wmi_unified *wmi_handle,
			 struct beacon_params *param)
{
	QDF_STATUS ret;
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	qdf_dma_addr_t dma_addr;
	uint32_t dtim_flag = 0;

	wmi_buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmi_buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	if (param->is_dtim_count_zero) {
		dtim_flag |= WMI_BCN_SEND_DTIM_ZERO;
		if (param->is_bitctl_reqd) {
			/* deliver CAB traffic in next DTIM beacon */
			dtim_flag |= WMI_BCN_SEND_DTIM_BITCTL_SET;
		}
	}
	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
				(wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = qdf_nbuf_len(param->wbuf);
	cmd->frame_ctrl = param->frame_ctrl;
	cmd->dtim_flag = dtim_flag;
	dma_addr = qdf_nbuf_get_frag_paddr(param->wbuf, 0);
	cmd->frag_ptr_lo = qdf_get_lower_32_bits(dma_addr);
#if defined(HTT_PADDR64)
	cmd->frag_ptr_hi = qdf_get_upper_32_bits(dma_addr) & 0x1F;
#endif
	cmd->bcn_antenna = param->bcn_txant;

	wmi_mtrace(WMI_PDEV_SEND_BCN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf, sizeof(*cmd),
				   WMI_PDEV_SEND_BCN_CMDID);
	if (ret != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to send bcn: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

QDF_STATUS
extract_tbttoffset_num_vdevs_tlv(struct wmi_unified *wmi_handle, void *evt_buf,
				 uint32_t *num_vdevs)
{
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	uint32_t vdev_map;

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		qdf_print("Invalid tbtt update ext event buffer");
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_event = param_buf->fixed_param;
	vdev_map = tbtt_offset_event->vdev_map;
	*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_extract_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle, void *evt_buf,
				 uint32_t *num_vdevs)
{
	if (wmi_handle->ops->extract_tbttoffset_num_vdevs)
		return wmi_handle->ops->extract_tbttoffset_num_vdevs(wmi_handle,
								     evt_buf,
								     num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
send_vdev_set_neighbour_rx_cmd_tlv(struct wmi_unified *wmi_handle,
				   uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				   struct set_neighbour_rx_params *param)
{
	wmi_vdev_filter_nrp_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_filter_nrp_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_filter_nrp_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_vdev_filter_nrp_config_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->bssid_idx = param->idx;
	cmd->action = param->action;
	cmd->type = param->type;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->addr);
	cmd->flag = 0;

	wmi_mtrace(WMI_VDEV_FILTER_NEIGHBOR_RX_PACKETS_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_FILTER_NEIGHBOR_RX_PACKETS_CMDID)) {
		WMI_LOGE("Failed to set neighbour rx param");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
extract_vdev_start_resp_tlv(struct wmi_unified *wmi_handle, void *evt_buf,
			    wmi_host_vdev_start_resp *vdev_rsp)
{
	WMI_VDEV_START_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_start_response_event_fixed_param *ev;

	param_buf = (WMI_VDEV_START_RESP_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		qdf_print("Invalid start response event buffer");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		qdf_print("Invalid start response event buffer");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_zero(vdev_rsp, sizeof(*vdev_rsp));

	vdev_rsp->vdev_id = ev->vdev_id;
	vdev_rsp->requestor_id = ev->requestor_id;
	switch (ev->resp_type) {
	case WMI_VDEV_START_RESP_EVENT:
		vdev_rsp->resp_type = WMI_HOST_VDEV_START_RESP_EVENT;
		break;
	case WMI_VDEV_RESTART_RESP_EVENT:
		vdev_rsp->resp_type = WMI_HOST_VDEV_RESTART_RESP_EVENT;
		break;
	default:
		qdf_print("Invalid start response event buffer");
		break;
	};
	vdev_rsp->status = ev->status;
	vdev_rsp->chain_mask = ev->chain_mask;
	vdev_rsp->smps_mode = ev->smps_mode;
	vdev_rsp->mac_id = ev->mac_id;
	vdev_rsp->cfgd_tx_streams = ev->cfgd_tx_streams;
	vdev_rsp->cfgd_rx_streams = ev->cfgd_rx_streams;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
extract_vdev_delete_resp_tlv(struct wmi_unified *wmi_handle, void *evt_buf,
			     struct wmi_host_vdev_delete_resp *delete_rsp)
{
	WMI_VDEV_DELETE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_delete_resp_event_fixed_param *ev;

	param_buf = (WMI_VDEV_DELETE_RESP_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid vdev delete response event buffer");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("Invalid vdev delete response event");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_zero(delete_rsp, sizeof(*delete_rsp));
	delete_rsp->vdev_id = ev->vdev_id;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
extract_vdev_stopped_param_tlv(struct wmi_unified *wmi_handle,
			       void *evt_buf, uint32_t *vdev_id)
{
	WMI_VDEV_STOPPED_EVENTID_param_tlvs *param_buf;
	wmi_vdev_stopped_event_fixed_param *resp_event;

	param_buf = (WMI_VDEV_STOPPED_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid event buffer");
		return QDF_STATUS_E_INVAL;
	}
	resp_event = param_buf->fixed_param;
	*vdev_id = resp_event->vdev_id;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wmi_unified_send_multiple_vdev_restart_req_cmd(
				struct wmi_unified *wmi_handle,
				struct multiple_vdev_restart_params *param)
{
	if (wmi_handle->ops->send_multiple_vdev_restart_req_cmd)
		return wmi_handle->ops->send_multiple_vdev_restart_req_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_beacon_send_cmd(struct wmi_unified *wmi_handle,
				       struct beacon_params *param)
{
	if (wmi_handle->ops->send_beacon_send_cmd)
		return wmi_handle->ops->send_beacon_send_cmd(wmi_handle,
							     param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_start_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			    wmi_host_vdev_start_resp *vdev_rsp)
{
	if (wmi_handle->ops->extract_vdev_start_resp)
		return wmi_handle->ops->extract_vdev_start_resp(wmi_handle,
								evt_buf,
								vdev_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_delete_resp(struct wmi_unified *wmi_handle, void *evt_buf,
			     struct wmi_host_vdev_delete_resp *delete_rsp)
{
	if (wmi_handle->ops->extract_vdev_delete_resp)
		return wmi_handle->ops->extract_vdev_delete_resp(wmi_handle,
								 evt_buf,
								 delete_rsp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_vdev_peer_delete_all_response_event(
	struct wmi_unified *wmi_handle,
	void *evt_buf,
	struct wmi_host_vdev_peer_delete_all_response_event *delete_rsp)
{
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_vdev_stopped_param(struct wmi_unified *wmi_handle, void *evt_buf,
			       uint32_t *vdev_id)
{
	if (wmi_handle->ops->extract_vdev_stopped_param)
		return wmi_handle->ops->extract_vdev_stopped_param(wmi_handle,
								   evt_buf,
								   vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_ext_tbttoffset_num_vdevs(struct wmi_unified *wmi_handle,
				     void *evt_buf, uint32_t *num_vdevs)
{
	if (wmi_handle->ops->extract_ext_tbttoffset_num_vdevs)
		return wmi_handle->ops->extract_ext_tbttoffset_num_vdevs(
							wmi_handle,
							evt_buf, num_vdevs);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_vdev_config_ratemask_cmd_send(struct wmi_unified *wmi_handle,
					  struct config_ratemask_params *param)
{
	if (wmi_handle->ops->send_vdev_config_ratemask_cmd)
		return wmi_handle->ops->send_vdev_config_ratemask_cmd(
							wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_vdev_set_neighbour_rx_cmd_send(
				struct wmi_unified *wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct set_neighbour_rx_params *param)
{
	if (wmi_handle->ops->send_vdev_set_neighbour_rx_cmd)
		return wmi_handle->ops->send_vdev_set_neighbour_rx_cmd(
							wmi_handle,
							macaddr, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

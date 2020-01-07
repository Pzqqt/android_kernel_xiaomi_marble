/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include <qdf_module.h>
#include <qdf_list.h>
#include <qdf_platform.h>
#include <wmi_unified_vdev_tlv.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>

static QDF_STATUS
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

	wmi_mtrace(WMI_VDEV_RATEMASK_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_RATEMASK_CMDID)) {
		WMI_LOGE("Seting vdev ratemask failed");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
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

static QDF_STATUS
extract_tbttoffset_num_vdevs_tlv(struct wmi_unified *wmi_handle, void *evt_buf,
				 uint32_t *num_vdevs)
{
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	uint32_t vdev_map;

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid tbtt update ext event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_event = param_buf->fixed_param;
	vdev_map = tbtt_offset_event->vdev_map;
	*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
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

static QDF_STATUS
extract_vdev_start_resp_tlv(struct wmi_unified *wmi_handle, void *evt_buf,
			    wmi_host_vdev_start_resp *vdev_rsp)
{
	WMI_VDEV_START_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_start_response_event_fixed_param *ev;

	param_buf = (WMI_VDEV_START_RESP_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid start response event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("%s: Invalid start response event buffer", __func__);
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
		WMI_LOGE("Invalid start response event buffer");
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

static QDF_STATUS
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

static QDF_STATUS extract_vdev_peer_delete_all_response_event_tlv(
		wmi_unified_t wmi_hdl,
		void *evt_buf,
		struct wmi_host_vdev_peer_delete_all_response_event *param)
{
	WMI_VDEV_DELETE_ALL_PEER_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_delete_all_peer_resp_event_fixed_param *ev;

	param_buf = (WMI_VDEV_DELETE_ALL_PEER_RESP_EVENTID_param_tlvs *)evt_buf;

	ev = (wmi_vdev_delete_all_peer_resp_event_fixed_param *)
							param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("%s: Invalid peer_delete all response", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	param->vdev_id = ev->vdev_id;
	param->status = ev->status;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
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

static QDF_STATUS extract_ext_tbttoffset_num_vdevs_tlv(
						wmi_unified_t wmi_hdl,
						void *evt_buf,
						uint32_t *num_vdevs)
{
	WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_ext_event_fixed_param *tbtt_offset_ext_event;

	param_buf = (WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s Invalid tbtt update ext event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_ext_event = param_buf->fixed_param;

	*num_vdevs = tbtt_offset_ext_event->num_vdevs;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_tbttoffset_update_params_tlv(
					wmi_unified_t wmi_hdl,
					void *evt_buf, uint8_t idx,
					struct tbttoffset_params *tbtt_param)
{
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	uint32_t vdev_map;

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid tbtt update event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	tbtt_offset_event = param_buf->fixed_param;
	vdev_map = tbtt_offset_event->vdev_map;
	tbtt_param->vdev_id = wmi_vdev_map_to_vdev_id(vdev_map, idx);
	if (tbtt_param->vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;
	tbtt_param->tbttoffset =
		param_buf->tbttoffset_list[tbtt_param->vdev_id];
	if (param_buf->tbtt_qtime_low_us_list)
		tbtt_param->vdev_tbtt_qtime_lo =
		    param_buf->tbtt_qtime_low_us_list[tbtt_param->vdev_id];
	if (param_buf->tbtt_qtime_high_us_list)
		tbtt_param->vdev_tbtt_qtime_hi =
		    param_buf->tbtt_qtime_high_us_list[tbtt_param->vdev_id];

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ext_tbttoffset_update_params_tlv(
					wmi_unified_t wmi_hdl,
					void *evt_buf, uint8_t idx,
					struct tbttoffset_params *tbtt_param)
{
	WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_info *tbtt_offset_info;

	param_buf = (WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid tbtt update event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_info = &param_buf->tbtt_offset_info[idx];

	tbtt_param->vdev_id = tbtt_offset_info->vdev_id;
	tbtt_param->tbttoffset = tbtt_offset_info->tbttoffset;
	tbtt_param->vdev_tbtt_qtime_lo = tbtt_offset_info->tbtt_qtime_low_us;
	tbtt_param->vdev_tbtt_qtime_hi = tbtt_offset_info->tbtt_qtime_high_us;

	return QDF_STATUS_SUCCESS;
}

void wmi_vdev_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *wmi_ops;

	if (!wmi_handle) {
		WMI_LOGP("%s: null wmi handle", __func__);
		return;
	}

	wmi_ops = wmi_handle->ops;
	wmi_ops->extract_vdev_delete_resp = extract_vdev_delete_resp_tlv;
	wmi_ops->extract_vdev_stopped_param = extract_vdev_stopped_param_tlv;
	wmi_ops->extract_vdev_start_resp = extract_vdev_start_resp_tlv;
	wmi_ops->extract_vdev_peer_delete_all_response_event =
				extract_vdev_peer_delete_all_response_event_tlv;
	wmi_ops->extract_tbttoffset_num_vdevs =
				extract_tbttoffset_num_vdevs_tlv;
	wmi_ops->extract_tbttoffset_update_params =
				extract_tbttoffset_update_params_tlv;
	wmi_ops->extract_ext_tbttoffset_update_params =
				extract_ext_tbttoffset_update_params_tlv;
	wmi_ops->extract_ext_tbttoffset_num_vdevs =
				extract_ext_tbttoffset_num_vdevs_tlv;
	wmi_ops->send_vdev_set_neighbour_rx_cmd =
				send_vdev_set_neighbour_rx_cmd_tlv;
	wmi_ops->send_beacon_send_cmd = send_beacon_send_cmd_tlv;
	wmi_ops->send_vdev_config_ratemask_cmd =
				send_vdev_config_ratemask_cmd_tlv;
}

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
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_dbr_param.h"
#include "wmi_unified_dbr_api.h"

/**
 * send_dbr_cfg_cmd_tlv() - configure DMA rings for Direct Buf RX
 * @wmi_handle: wmi handle
 * @data_len: len of dma cfg req
 * @data: dma cfg req
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS send_dbr_cfg_cmd_tlv(wmi_unified_t wmi_handle,
				struct direct_buf_rx_cfg_req *cfg)
{
	wmi_buf_t buf;
	wmi_dma_ring_cfg_req_fixed_param *cmd;
	QDF_STATUS ret;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_dma_ring_cfg_req_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_dma_ring_cfg_req_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_dma_ring_cfg_req_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_host_pdev_id_to_target(
						wmi_handle,
						cfg->pdev_id);
	cmd->mod_id = cfg->mod_id;
	cmd->base_paddr_lo = cfg->base_paddr_lo;
	cmd->base_paddr_hi = cfg->base_paddr_hi;
	cmd->head_idx_paddr_lo = cfg->head_idx_paddr_lo;
	cmd->head_idx_paddr_hi = cfg->head_idx_paddr_hi;
	cmd->tail_idx_paddr_lo = cfg->tail_idx_paddr_lo;
	cmd->tail_idx_paddr_hi = cfg->tail_idx_paddr_hi;
	cmd->num_elems = cfg->num_elems;
	cmd->buf_size = cfg->buf_size;
	cmd->num_resp_per_event = cfg->num_resp_per_event;
	cmd->event_timeout_ms = cfg->event_timeout_ms;

	WMI_LOGD("%s: wmi_dma_ring_cfg_req_fixed_param pdev id %d mod id %d"
		  "base paddr lo %x base paddr hi %x head idx paddr lo %x"
		  "head idx paddr hi %x tail idx paddr lo %x"
		  "tail idx addr hi %x num elems %d buf size %d num resp %d"
		  "event timeout %d", __func__, cmd->pdev_id,
		  cmd->mod_id, cmd->base_paddr_lo, cmd->base_paddr_hi,
		  cmd->head_idx_paddr_lo, cmd->head_idx_paddr_hi,
		  cmd->tail_idx_paddr_lo, cmd->tail_idx_paddr_hi,
		  cmd->num_elems, cmd->buf_size, cmd->num_resp_per_event,
		  cmd->event_timeout_ms);
	wmi_mtrace(WMI_PDEV_DMA_RING_CFG_REQ_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_DMA_RING_CFG_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL(":wmi cmd send failed"));
		wmi_buf_free(buf);
	}

	return ret;
}

static QDF_STATUS extract_scaling_params_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t idx,
			struct wlan_psoc_host_spectral_scaling_params *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	wmi_spectral_bin_scaling_params *spectral_bin_scaling_params;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	spectral_bin_scaling_params = &param_buf->wmi_bin_scaling_params[idx];

	param->pdev_id = wmi_handle->ops->convert_target_pdev_id_to_host(
					wmi_handle,
					spectral_bin_scaling_params->pdev_id);
	param->low_level_offset = spectral_bin_scaling_params->low_level_offset;
	param->formula_id = spectral_bin_scaling_params->formula_id;
	param->high_level_offset =
		spectral_bin_scaling_params->high_level_offset;
	param->rssi_thr = spectral_bin_scaling_params->rssi_thr;
	param->default_agc_max_gain =
		spectral_bin_scaling_params->default_agc_max_gain;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_dbr_buf_release_fixed_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, struct direct_buf_rx_rsp *param)
{
	WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *param_buf;
	wmi_dma_buf_release_fixed_param *ev;

	param_buf = (WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	param->pdev_id = wmi_handle->ops->convert_target_pdev_id_to_host(
								wmi_handle,
								ev->pdev_id);
	param->mod_id = ev->mod_id;
	param->num_buf_release_entry = ev->num_buf_release_entry;
	param->num_meta_data_entry = ev->num_meta_data_entry;
	WMI_LOGD("%s:pdev id %d mod id %d num buf release entry %d", __func__,
		 param->pdev_id, param->mod_id, param->num_buf_release_entry);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_dbr_buf_release_entry_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, uint8_t idx, struct direct_buf_rx_entry *param)
{
	WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *param_buf;
	wmi_dma_buf_release_entry *entry;

	param_buf = (WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	entry = &param_buf->entries[idx];

	if (!entry) {
		WMI_LOGE("%s: Entry is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("%s: paddr_lo[%d] = %x", __func__, idx, entry->paddr_lo);

	param->paddr_lo = entry->paddr_lo;
	param->paddr_hi = entry->paddr_hi;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_dbr_buf_metadata_tlv(
		wmi_unified_t wmi_handle, uint8_t *event,
		uint8_t idx, struct direct_buf_rx_metadata *param)
{
	WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *param_buf;
	wmi_dma_buf_release_spectral_meta_data *entry;

	param_buf = (WMI_PDEV_DMA_RING_BUF_RELEASE_EVENTID_param_tlvs *)event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	entry = &param_buf->meta_data[idx];

	if (!entry) {
		WMI_LOGE("%s: Entry is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(param->noisefloor, entry->noise_floor,
		     qdf_min(sizeof(entry->noise_floor),
			     sizeof(param->noisefloor)));
	param->reset_delay = entry->reset_delay;

	return QDF_STATUS_SUCCESS;
}

void wmi_dbr_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_dbr_cfg_cmd = send_dbr_cfg_cmd_tlv;
	ops->extract_dbr_buf_release_entry = extract_dbr_buf_release_entry_tlv;
	ops->extract_dbr_buf_metadata = extract_dbr_buf_metadata_tlv;
	ops->extract_dbr_buf_release_fixed = extract_dbr_buf_release_fixed_tlv;
	ops->extract_scaling_params_service_ready_ext =
			extract_scaling_params_service_ready_ext_tlv;
}

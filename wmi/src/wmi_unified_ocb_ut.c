/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_version.h"
#include "wmi_unified_priv.h"
#include <wlan_defs.h>
#include "target_if.h"
#include "wma.h"
#include "wlan_ocb_ucfg_api.h"
#include "wlan_ocb_main.h"

void wmi_ocb_ut_attach(struct wmi_unified *wmi_handle);

static inline struct wlan_ocb_rx_ops *
target_if_ocb_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_pdev *pdev;
	struct ocb_pdev_obj *pdev_obj;

	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0,
					  WLAN_OCB_SB_ID);
	pdev_obj = (struct ocb_pdev_obj *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_OCB);
	return &pdev_obj->ocb_rxops;
}

/**
 * fake_vdev_create_cmd_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_vdev_create_cmd_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				 struct vdev_create_params *param)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_vdev_delete_cmd_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_vdev_delete_cmd_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_ocb_set_utc_time_cmd_tlv() - send the UTC time to the firmware
 * @wmi_handle: pointer to the wmi handle
 * @utc: pointer to the UTC time struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS fake_ocb_set_utc_time_cmd_tlv(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_ocb_get_tsf_timer_cmd_tlv() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @request: pointer to the request
 *
 * Return: 0 on succes
 */
static QDF_STATUS fake_ocb_get_tsf_timer_cmd_tlv(wmi_unified_t wmi_handle,
			  uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_ocb_rx_ops *ocb_rx_ops;
	struct ocb_get_tsf_timer_response response;
	ol_scn_t scn = (ol_scn_t) wmi_handle->scn_handle;

	WMI_LOGP("%s : called", __func__);
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		WMI_LOGP("null psoc");
		return -EINVAL;
	}
	response.vdev_id = vdev_id;
	response.timer_high = 0x1234;
	response.timer_low = 0x5678;

	ocb_rx_ops = target_if_ocb_get_rx_ops(psoc);
	if (ocb_rx_ops->ocb_tsf_timer) {
		status = ocb_rx_ops->ocb_tsf_timer(psoc, &response);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGP("ocb_tsf_timer failed.");
			return -EINVAL;
		}
	} else {
		WMI_LOGP("No ocb_tsf_timer callback");
		return -EINVAL;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_dcc_clear_stats_cmd_tlv() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 * @dcc_stats_bitmap: dcc status bitmap
 *
 * Return: 0 on succes
 */
static QDF_STATUS fake_dcc_clear_stats_cmd_tlv(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

wmi_dcc_ndl_stats_per_channel chan1_info[2] = {
	[0] = {.chan_info = 5860,
	.tx_power_datarate = 23 | (10 << 8),
	.carrier_sense_est_comm_range = 107 | (198 << 13),
	.dcc_stats = 78 | (56 << 8) | (345 << 16),
	.packet_stats = 1278 | (789 << 14),
	.channel_busy_time = 1389,
	},
	[1] = {.chan_info = 5880,
	.tx_power_datarate = 53 | (17 << 8),
	.carrier_sense_est_comm_range = 137 | (198 << 13),
	.dcc_stats = 78 | (66 << 8) | (245 << 16),
	.packet_stats = 1278 | (889 << 14),
	.channel_busy_time = 2389,
	},
};

/**
 * fake_dcc_get_stats_cmd_tlv() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: 0 on succes
 */
static QDF_STATUS fake_dcc_get_stats_cmd_tlv(wmi_unified_t wmi_handle,
		     struct ocb_dcc_get_stats_param *get_stats_param)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_ocb_rx_ops *ocb_rx_ops;
	ol_scn_t scn = (ol_scn_t) wmi_handle->scn_handle;
	struct ocb_dcc_get_stats_response *response;
	WMI_LOGP("%s : called", __func__);

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		WMI_LOGP("null psoc");
		return -EINVAL;
	}
	response = qdf_mem_malloc(sizeof(*response) + 2 *
			sizeof(wmi_dcc_ndl_stats_per_channel));
	response->num_channels = 2;
	response->channel_stats_array_len = 2 *
			sizeof(wmi_dcc_ndl_stats_per_channel);
	response->vdev_id = get_stats_param->vdev_id;
	response->channel_stats_array = (uint8_t *)response + sizeof(*response);
	qdf_mem_copy(response->channel_stats_array,
		     &chan1_info,
		     2 * sizeof(wmi_dcc_ndl_stats_per_channel));
	WMI_LOGP("channel1 freq %d, channel2 freq %d", chan1_info[0].chan_info,
			chan1_info[1].chan_info);
	ocb_rx_ops = target_if_ocb_get_rx_ops(psoc);
	if (ocb_rx_ops->ocb_dcc_stats_indicate) {
		status = ocb_rx_ops->ocb_dcc_stats_indicate(psoc,
						response, true);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGP("dcc_stats_indicate failed.");
			status = -EINVAL;
		} else {
			status = 0;
		}
	} else {
		WMI_LOGP("No dcc_stats_indicate callback");
		status = -EINVAL;
	}

	qdf_mem_free(response);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_dcc_update_ndl_cmd_tlv() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: 0 on success
 */
static QDF_STATUS fake_dcc_update_ndl_cmd_tlv(wmi_unified_t wmi_handle,
		       struct ocb_dcc_update_ndl_param *update_ndl_param)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_ocb_rx_ops *ocb_rx_ops;
	ol_scn_t scn = (ol_scn_t) wmi_handle->scn_handle;
	struct ocb_dcc_update_ndl_response *resp;
	WMI_LOGP("%s : called", __func__);
	/* Allocate and populate the response */
	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return -ENOMEM;

	resp->vdev_id = update_ndl_param->vdev_id;
	resp->status = 0;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		WMI_LOGP("null psoc");
		return -EINVAL;
	}

	ocb_rx_ops = target_if_ocb_get_rx_ops(psoc);
	if (ocb_rx_ops->ocb_dcc_ndl_update) {
		status = ocb_rx_ops->ocb_dcc_ndl_update(psoc, resp);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGP("dcc_ndl_update failed.");
			status = -EINVAL;
		} else {
			status = 0;
		}
	} else {
		WMI_LOGP("No dcc_ndl_update callback");
		status = -EINVAL;
	}

	qdf_mem_free(resp);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_ocb_set_config_cmd_tlv() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: 0 on success
 */
static QDF_STATUS fake_ocb_set_config_cmd_tlv(wmi_unified_t wmi_handle,
			struct ocb_config *config)
{
	u32 i;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_ocb_rx_ops *ocb_rx_ops;
	ol_scn_t scn = (ol_scn_t) wmi_handle->scn_handle;

	WMI_LOGP("%s : called", __func__);
	WMI_LOGI("%s: vdev_id=%d, channel_count=%d, schedule_size=%d, flag=%x",
		 __func__, config->vdev_id, config->channel_count,
		 config->schedule_size, config->flags);

	for (i = 0; i < config->channel_count; i++) {
		WMI_LOGI("%s: channel info for channel %d"
			" chan_freq=%d, bandwidth=%d, " QDF_MAC_ADDRESS_STR
			" max_pwr=%d, min_pwr=%d, reg_pwr=%d, antenna_max=%d, "
			"flags=%d", __func__, i, config->channels[i].chan_freq,
			config->channels[i].bandwidth,
			QDF_MAC_ADDR_ARRAY(
				config->channels[i].mac_address.bytes),
			config->channels[i].max_pwr,
			config->channels[i].min_pwr,
			config->channels[i].reg_pwr,
			config->channels[i].antenna_max,
			config->channels[i].flags);
	}

	for (i = 0; i < config->schedule_size; i++) {
		WMI_LOGI("%s: schedule info for channel %d: "
			"chan_fre=%d, total_duration=%d, guard_intreval=%d",
			__func__, i, config->schedule[i].chan_freq,
			config->schedule[i].total_duration,
			config->schedule[i].guard_interval);
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		WMI_LOGP("null psoc");
		return -EINVAL;
	}

	ocb_rx_ops = target_if_ocb_get_rx_ops(psoc);
	if (ocb_rx_ops->ocb_set_config_status) {
		status = ocb_rx_ops->ocb_set_config_status(psoc, 0);
		if (status != QDF_STATUS_SUCCESS) {
			WMI_LOGP("ocb_set_config_status failed.");
			return -EINVAL;
		}
	} else {
		WMI_LOGP("No ocb_set_config_status callback");
		return -EINVAL;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_ocb_stop_timing_advert_cmd_tlv() - stop sending the
 *  timing advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS fake_ocb_stop_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_ocb_start_timing_advert_cmd_tlv() - start sending the
 *  timing advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS
fake_ocb_start_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
		struct ocb_timing_advert_param *timing_advert)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_peer_create_cmd_tlv() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_peer_create_cmd_tlv(wmi_unified_t wmi,
					struct peer_create_params *param)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_peer_delete_cmd_tlv() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_peer_delete_cmd_tlv(wmi_unified_t wmi,
				 uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
				 uint8_t vdev_id)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_vdev_start_cmd_tlv() - send vdev start request to fw
 * @wmi_handle: wmi handle
 * @req: vdev start params
 *
 * Return: QDF status
 */
static QDF_STATUS fake_vdev_start_cmd_tlv(wmi_unified_t wmi_handle,
			  struct vdev_start_params *req)
{
	tp_wma_handle wma = (tp_wma_handle) wmi_handle->scn_handle;
	WMI_LOGP("%s : called", __func__);
	WMI_LOGI("%s: vdev_id %d freq %d chanmode %d ch_info is_dfs %d "
		"beacon interval %d dtim %d center_chan %d center_freq2 %d "
		"max_txpow: 0x%x "
		"Tx SS %d, Rx SS %d, ldpc_rx: %d, cac %d, regd %d, HE ops: %d",
		__func__, (int)req->vdev_id, req->channel.mhz,
		req->channel.phy_mode,
		(int)req->channel.dfs_set, req->beacon_intval, req->dtim_period,
		req->channel.cfreq1, req->channel.cfreq2,
		req->channel.maxregpower,
		req->preferred_tx_streams, req->preferred_rx_streams,
		(int)req->ldpc_rx_enabled, req->cac_duration_ms,
		req->regdomain, req->he_ops);
	ucfg_ocb_config_channel(wma->pdev);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_vdev_down_cmd_tlv() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_vdev_down_cmd_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 *  fake_vdev_set_param_cmd_tlv() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS fake_vdev_set_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * fake_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv() -
 *  faked API to enable/disable mcc scheduler
 * @wmi_handle: wmi handle
 * @mcc_adaptive_scheduler: enable/disable
 *
 * This function enable/disable mcc adaptive scheduler in fw.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS fake_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv(
		wmi_unified_t wmi_handle, uint32_t mcc_adaptive_scheduler,
		uint32_t pdev_id)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

/*
 * fake_process_set_ie_info_cmd_tlv() - Function to send IE info to firmware
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS fake_process_set_ie_info_cmd_tlv(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info)
{
	WMI_LOGP("%s : called", __func__);
	return QDF_STATUS_SUCCESS;
}

void wmi_ocb_ut_attach(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *wmi_ops;

	if (!wmi_handle) {
		WMI_LOGP("%s: null wmi handle", __func__);
		return;
	}

	wmi_ops = wmi_handle->ops;
	wmi_ops->send_vdev_create_cmd = fake_vdev_create_cmd_tlv;
	wmi_ops->send_vdev_delete_cmd = fake_vdev_delete_cmd_tlv;
	wmi_ops->send_vdev_down_cmd = fake_vdev_down_cmd_tlv;
	wmi_ops->send_vdev_start_cmd = fake_vdev_start_cmd_tlv;
	wmi_ops->send_peer_create_cmd = fake_peer_create_cmd_tlv;
	wmi_ops->send_peer_delete_cmd = fake_peer_delete_cmd_tlv;
	wmi_ops->send_vdev_set_param_cmd = fake_vdev_set_param_cmd_tlv;
	wmi_ops->send_ocb_set_utc_time_cmd = fake_ocb_set_utc_time_cmd_tlv;
	wmi_ops->send_ocb_get_tsf_timer_cmd = fake_ocb_get_tsf_timer_cmd_tlv;
	wmi_ops->send_dcc_clear_stats_cmd = fake_dcc_clear_stats_cmd_tlv;
	wmi_ops->send_dcc_get_stats_cmd = fake_dcc_get_stats_cmd_tlv;
	wmi_ops->send_dcc_update_ndl_cmd = fake_dcc_update_ndl_cmd_tlv;
	wmi_ops->send_ocb_set_config_cmd = fake_ocb_set_config_cmd_tlv;
	wmi_ops->send_ocb_stop_timing_advert_cmd =
			fake_ocb_stop_timing_advert_cmd_tlv;
	wmi_ops->send_ocb_start_timing_advert_cmd =
			fake_ocb_start_timing_advert_cmd_tlv;
	wmi_ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd =
			fake_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv;
	wmi_ops->send_process_set_ie_info_cmd =
			fake_process_set_ie_info_cmd_tlv;
}

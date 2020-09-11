/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wlan_ocb_public_structs.h>
#include <wmi_unified_ocb_api.h>

/**
 * send_ocb_set_utc_time_cmd() - send the UTC time to the firmware
 * @wmi_handle: pointer to the wmi handle
 * @utc: pointer to the UTC time struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_set_utc_time_cmd_tlv(wmi_unified_t wmi_handle,
						struct ocb_utc_param *utc)
{
	QDF_STATUS ret;
	wmi_ocb_set_utc_time_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len, i;
	wmi_buf_t buf;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_set_utc_time_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_set_utc_time_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_set_utc_time_cmd_fixed_param));
	cmd->vdev_id = utc->vdev_id;

	for (i = 0; i < SIZE_UTC_TIME; i++)
		WMI_UTC_TIME_SET(cmd, i, utc->utc_time[i]);

	for (i = 0; i < SIZE_UTC_TIME_ERROR; i++)
		WMI_TIME_ERROR_SET(cmd, i, utc->time_error[i]);

	wmi_mtrace(WMI_OCB_SET_UTC_TIME_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_SET_UTC_TIME_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to set OCB UTC time");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_start_timing_advert_cmd_tlv() - start sending the timing advertisement
 *				   frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_start_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
				struct ocb_timing_advert_param *timing_advert)
{
	QDF_STATUS ret;
	wmi_ocb_start_timing_advert_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len, len_template;
	wmi_buf_t buf;

	len = sizeof(*cmd) +
		     WMI_TLV_HDR_SIZE;

	len_template = timing_advert->template_length;
	/* Add padding to the template if needed */
	if (len_template % 4 != 0)
		len_template += 4 - (len_template % 4);
	len += len_template;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_start_timing_advert_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_start_timing_advert_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_start_timing_advert_cmd_fixed_param));
	cmd->vdev_id = timing_advert->vdev_id;
	cmd->repeat_rate = timing_advert->repeat_rate;
	cmd->channel_freq = timing_advert->chan_freq;
	cmd->timestamp_offset = timing_advert->timestamp_offset;
	cmd->time_value_offset = timing_advert->time_value_offset;
	cmd->timing_advert_template_length = timing_advert->template_length;
	buf_ptr += sizeof(*cmd);

	/* Add the timing advert template */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       len_template);
	qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
		     (uint8_t *)timing_advert->template_value,
		     timing_advert->template_length);

	wmi_mtrace(WMI_OCB_START_TIMING_ADVERT_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_START_TIMING_ADVERT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to start OCB timing advert");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_stop_timing_advert_cmd_tlv() - stop sending the timing advertisement frames
 *				  on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_stop_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert)
{
	QDF_STATUS ret;
	wmi_ocb_stop_timing_advert_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len;
	wmi_buf_t buf;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_stop_timing_advert_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_stop_timing_advert_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_stop_timing_advert_cmd_fixed_param));
	cmd->vdev_id = timing_advert->vdev_id;
	cmd->channel_freq = timing_advert->chan_freq;

	wmi_mtrace(WMI_OCB_STOP_TIMING_ADVERT_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_STOP_TIMING_ADVERT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to stop OCB timing advert");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_get_tsf_timer_cmd_tlv() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @request: pointer to the request
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_get_tsf_timer_cmd_tlv(wmi_unified_t wmi_handle,
			  uint8_t vdev_id)
{
	QDF_STATUS ret;
	wmi_ocb_get_tsf_timer_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);

	cmd = (wmi_ocb_get_tsf_timer_cmd_fixed_param *)buf_ptr;
	qdf_mem_zero(cmd, len);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_get_tsf_timer_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_get_tsf_timer_cmd_fixed_param));
	cmd->vdev_id = vdev_id;

	/* Send the WMI command */
	wmi_mtrace(WMI_OCB_GET_TSF_TIMER_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_GET_TSF_TIMER_CMDID);
	/* If there is an error, set the completion event */
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI message: %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_get_stats_cmd_tlv() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_dcc_get_stats_cmd_tlv(wmi_unified_t wmi_handle,
		     struct ocb_dcc_get_stats_param *get_stats_param)
{
	QDF_STATUS ret;
	wmi_dcc_get_stats_cmd_fixed_param *cmd;
	wmi_dcc_channel_stats_request *channel_stats_array;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint32_t i;

	/* Validate the input */
	if (get_stats_param->request_array_len !=
	    get_stats_param->channel_count * sizeof(*channel_stats_array)) {
		wmi_err("Invalid parameter");
		return QDF_STATUS_E_INVAL;
	}

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		get_stats_param->request_array_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_get_stats_cmd_fixed_param *)buf_ptr;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_get_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_get_stats_cmd_fixed_param));
	cmd->vdev_id = get_stats_param->vdev_id;
	cmd->num_channels = get_stats_param->channel_count;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       get_stats_param->request_array_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	channel_stats_array = (wmi_dcc_channel_stats_request *)buf_ptr;
	qdf_mem_copy(channel_stats_array, get_stats_param->request_array,
		     get_stats_param->request_array_len);
	for (i = 0; i < cmd->num_channels; i++)
		WMITLV_SET_HDR(&channel_stats_array[i].tlv_header,
			       WMITLV_TAG_STRUC_wmi_dcc_channel_stats_request,
			       WMITLV_GET_STRUCT_TLVLEN(
					wmi_dcc_channel_stats_request));

	/* Send the WMI command */
	wmi_mtrace(WMI_DCC_GET_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_DCC_GET_STATS_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send WMI message: %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_clear_stats_cmd_tlv() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 * @dcc_stats_bitmap: dcc status bitmap
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_dcc_clear_stats_cmd_tlv(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap)
{
	QDF_STATUS ret;
	wmi_dcc_clear_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_clear_stats_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_clear_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_clear_stats_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->dcc_stats_bitmap = dcc_stats_bitmap;

	/* Send the WMI command */
	wmi_mtrace(WMI_DCC_CLEAR_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_DCC_CLEAR_STATS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send the WMI command");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_update_ndl_cmd_tlv() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: 0 on success
 */
static QDF_STATUS send_dcc_update_ndl_cmd_tlv(wmi_unified_t wmi_handle,
		       struct ocb_dcc_update_ndl_param *update_ndl_param)
{
	QDF_STATUS qdf_status;
	wmi_dcc_update_ndl_cmd_fixed_param *cmd;
	wmi_dcc_ndl_chan *ndl_chan_array;
	wmi_dcc_ndl_active_state_config *ndl_active_state_array;
	uint32_t active_state_count;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint32_t i;

	/* validate the input */
	if (update_ndl_param->dcc_ndl_chan_list_len !=
	    update_ndl_param->channel_count * sizeof(*ndl_chan_array)) {
		wmi_err("Invalid parameter");
		return QDF_STATUS_E_INVAL;
	}
	active_state_count = 0;
	ndl_chan_array = update_ndl_param->dcc_ndl_chan_list;
	for (i = 0; i < update_ndl_param->channel_count; i++)
		active_state_count +=
			WMI_NDL_NUM_ACTIVE_STATE_GET(&ndl_chan_array[i]);
	if (update_ndl_param->dcc_ndl_active_state_list_len !=
	    active_state_count * sizeof(*ndl_active_state_array)) {
		wmi_err("Invalid parameter");
		return QDF_STATUS_E_INVAL;
	}

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + update_ndl_param->dcc_ndl_chan_list_len +
		WMI_TLV_HDR_SIZE +
		update_ndl_param->dcc_ndl_active_state_list_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_update_ndl_cmd_fixed_param *)buf_ptr;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_update_ndl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_update_ndl_cmd_fixed_param));
	cmd->vdev_id = update_ndl_param->vdev_id;
	cmd->num_channel = update_ndl_param->channel_count;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       update_ndl_param->dcc_ndl_chan_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	ndl_chan_array = (wmi_dcc_ndl_chan *)buf_ptr;
	qdf_mem_copy(ndl_chan_array, update_ndl_param->dcc_ndl_chan_list,
		     update_ndl_param->dcc_ndl_chan_list_len);
	for (i = 0; i < cmd->num_channel; i++)
		WMITLV_SET_HDR(&ndl_chan_array[i].tlv_header,
			       WMITLV_TAG_STRUC_wmi_dcc_ndl_chan,
			       WMITLV_GET_STRUCT_TLVLEN(
					wmi_dcc_ndl_chan));
	buf_ptr += update_ndl_param->dcc_ndl_chan_list_len;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       update_ndl_param->dcc_ndl_active_state_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	ndl_active_state_array = (wmi_dcc_ndl_active_state_config *)buf_ptr;
	qdf_mem_copy(ndl_active_state_array,
		     update_ndl_param->dcc_ndl_active_state_list,
		     update_ndl_param->dcc_ndl_active_state_list_len);
	for (i = 0; i < active_state_count; i++) {
		WMITLV_SET_HDR(&ndl_active_state_array[i].tlv_header,
			WMITLV_TAG_STRUC_wmi_dcc_ndl_active_state_config,
			WMITLV_GET_STRUCT_TLVLEN(
			    wmi_dcc_ndl_active_state_config));
	}
	buf_ptr += update_ndl_param->dcc_ndl_active_state_list_len;

	/* Send the WMI command */
	wmi_mtrace(WMI_DCC_UPDATE_NDL_CMDID, cmd->vdev_id, 0);
	qdf_status = wmi_unified_cmd_send(wmi_handle, buf, len,
					  WMI_DCC_UPDATE_NDL_CMDID);
	/* If there is an error, set the completion event */
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		wmi_err("Failed to send WMI message: %d", qdf_status);
		wmi_buf_free(buf);
	}

	return qdf_status;
}

/**
 * send_ocb_set_config_cmd_tlv() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: 0 on success
 */
static QDF_STATUS send_ocb_set_config_cmd_tlv(wmi_unified_t wmi_handle,
					      struct ocb_config *config)
{
	QDF_STATUS ret;
	wmi_ocb_set_config_cmd_fixed_param *cmd;
	wmi_channel *chan;
	wmi_ocb_channel *ocb_chan;
	wmi_qos_parameter *qos_param;
	wmi_dcc_ndl_chan *ndl_chan;
	wmi_dcc_ndl_active_state_config *ndl_active_config;
	wmi_ocb_schedule_element *sched_elem;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int32_t len;
	int32_t i, j, active_state_count;

	/*
	 * Validate the dcc_ndl_chan_list_len and count the number of active
	 * states. Validate dcc_ndl_active_state_list_len.
	 */
	active_state_count = 0;
	if (config->dcc_ndl_chan_list_len) {
		if (!config->dcc_ndl_chan_list ||
			config->dcc_ndl_chan_list_len !=
			config->channel_count * sizeof(wmi_dcc_ndl_chan)) {
			wmi_err("NDL channel is invalid. List len: %d",
				 config->dcc_ndl_chan_list_len);
			return QDF_STATUS_E_INVAL;
		}

		for (i = 0, ndl_chan = config->dcc_ndl_chan_list;
				i < config->channel_count; ++i, ++ndl_chan)
			active_state_count +=
				WMI_NDL_NUM_ACTIVE_STATE_GET(ndl_chan);

		if (active_state_count) {
			if (!config->dcc_ndl_active_state_list ||
				config->dcc_ndl_active_state_list_len !=
				active_state_count *
				sizeof(wmi_dcc_ndl_active_state_config)) {
				wmi_err("NDL active state is invalid");
				return QDF_STATUS_E_INVAL;
			}
		}
	}

	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_channel) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_ocb_channel) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_qos_parameter) * WMI_MAX_NUM_AC +
		WMI_TLV_HDR_SIZE + config->dcc_ndl_chan_list_len +
		WMI_TLV_HDR_SIZE + active_state_count *
			sizeof(wmi_dcc_ndl_active_state_config) +
		WMI_TLV_HDR_SIZE + config->schedule_size *
			sizeof(wmi_ocb_schedule_element);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_set_config_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_set_config_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_set_config_cmd_fixed_param));
	cmd->vdev_id = config->vdev_id;
	cmd->channel_count = config->channel_count;
	cmd->schedule_size = config->schedule_size;
	cmd->flags = config->flags;
	buf_ptr += sizeof(*cmd);

	/* Add the wmi_channel info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->channel_count * sizeof(wmi_channel));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->channel_count; i++) {
		chan = (wmi_channel *)buf_ptr;
		WMITLV_SET_HDR(&chan->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan->mhz = config->channels[i].chan_freq;
		chan->band_center_freq1 = config->channels[i].chan_freq;
		chan->band_center_freq2 = 0;
		chan->info = 0;

		WMI_SET_CHANNEL_MODE(chan, config->channels[i].ch_mode);
		WMI_SET_CHANNEL_MAX_POWER(chan, config->channels[i].max_pwr);
		WMI_SET_CHANNEL_MIN_POWER(chan, config->channels[i].min_pwr);
		WMI_SET_CHANNEL_MAX_TX_POWER(chan, config->channels[i].max_pwr);
		WMI_SET_CHANNEL_REG_POWER(chan, config->channels[i].reg_pwr);
		WMI_SET_CHANNEL_ANTENNA_MAX(chan,
					    config->channels[i].antenna_max);

		if (config->channels[i].bandwidth < 10)
			WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);
		else if (config->channels[i].bandwidth < 20)
			WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
		buf_ptr += sizeof(*chan);
	}

	/* Add the wmi_ocb_channel info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->channel_count * sizeof(wmi_ocb_channel));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->channel_count; i++) {
		ocb_chan = (wmi_ocb_channel *)buf_ptr;
		WMITLV_SET_HDR(&ocb_chan->tlv_header,
			       WMITLV_TAG_STRUC_wmi_ocb_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_channel));
		ocb_chan->bandwidth = config->channels[i].bandwidth;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(
					config->channels[i].mac_address.bytes,
					&ocb_chan->mac_address);
		buf_ptr += sizeof(*ocb_chan);
	}

	/* Add the wmi_qos_parameter info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		config->channel_count * sizeof(wmi_qos_parameter)*WMI_MAX_NUM_AC);
	buf_ptr += WMI_TLV_HDR_SIZE;
	/* WMI_MAX_NUM_AC parameters for each channel */
	for (i = 0; i < config->channel_count; i++) {
		for (j = 0; j < WMI_MAX_NUM_AC; j++) {
			qos_param = (wmi_qos_parameter *)buf_ptr;
			WMITLV_SET_HDR(&qos_param->tlv_header,
				WMITLV_TAG_STRUC_wmi_qos_parameter,
				WMITLV_GET_STRUCT_TLVLEN(wmi_qos_parameter));
			qos_param->aifsn =
				config->channels[i].qos_params[j].aifsn;
			qos_param->cwmin =
				config->channels[i].qos_params[j].cwmin;
			qos_param->cwmax =
				config->channels[i].qos_params[j].cwmax;
			buf_ptr += sizeof(*qos_param);
		}
	}

	/* Add the wmi_dcc_ndl_chan (per channel) */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->dcc_ndl_chan_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (config->dcc_ndl_chan_list_len) {
		ndl_chan = (wmi_dcc_ndl_chan *)buf_ptr;
		qdf_mem_copy(ndl_chan, config->dcc_ndl_chan_list,
			     config->dcc_ndl_chan_list_len);
		for (i = 0; i < config->channel_count; i++)
			WMITLV_SET_HDR(&(ndl_chan[i].tlv_header),
				WMITLV_TAG_STRUC_wmi_dcc_ndl_chan,
				WMITLV_GET_STRUCT_TLVLEN(wmi_dcc_ndl_chan));
		buf_ptr += config->dcc_ndl_chan_list_len;
	}

	/* Add the wmi_dcc_ndl_active_state_config */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, active_state_count *
		       sizeof(wmi_dcc_ndl_active_state_config));
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (active_state_count) {
		ndl_active_config = (wmi_dcc_ndl_active_state_config *)buf_ptr;
		qdf_mem_copy(ndl_active_config,
			config->dcc_ndl_active_state_list,
			active_state_count * sizeof(*ndl_active_config));
		for (i = 0; i < active_state_count; ++i)
			WMITLV_SET_HDR(&(ndl_active_config[i].tlv_header),
			  WMITLV_TAG_STRUC_wmi_dcc_ndl_active_state_config,
			  WMITLV_GET_STRUCT_TLVLEN(
				wmi_dcc_ndl_active_state_config));
		buf_ptr += active_state_count *
			sizeof(*ndl_active_config);
	}

	/* Add the wmi_ocb_schedule_element info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		config->schedule_size * sizeof(wmi_ocb_schedule_element));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->schedule_size; i++) {
		sched_elem = (wmi_ocb_schedule_element *)buf_ptr;
		WMITLV_SET_HDR(&sched_elem->tlv_header,
			WMITLV_TAG_STRUC_wmi_ocb_schedule_element,
			WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_schedule_element));
		sched_elem->channel_freq = config->schedule[i].chan_freq;
		sched_elem->total_duration = config->schedule[i].total_duration;
		sched_elem->guard_interval = config->schedule[i].guard_interval;
		buf_ptr += sizeof(*sched_elem);
	}

	wmi_mtrace(WMI_OCB_SET_CONFIG_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_SET_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to set OCB config");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * extract_ocb_channel_config_resp_tlv() - extract ocb channel config resp
 * @wmi_handle: wmi handle
 * @evt_buf: wmi event buffer
 * @status: status buffer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS extract_ocb_channel_config_resp_tlv(wmi_unified_t wmi_handle,
						      void *evt_buf,
						      uint32_t *status)
{
	WMI_OCB_SET_CONFIG_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_ocb_set_config_resp_event_fixed_param *fix_param;

	param_tlvs = evt_buf;
	fix_param = param_tlvs->fixed_param;

	*status = fix_param->status;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_ocb_tsf_timer_tlv() - extract TSF timer from event buffer
 * @wmi_handle: wmi handle
 * @evt_buf: wmi event buffer
 * @resp: response buffer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS extract_ocb_tsf_timer_tlv(wmi_unified_t wmi_handle,
			void *evt_buf, struct ocb_get_tsf_timer_response *resp)
{
	WMI_OCB_GET_TSF_TIMER_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_ocb_get_tsf_timer_resp_event_fixed_param *fix_param;

	param_tlvs = evt_buf;
	fix_param = param_tlvs->fixed_param;
	resp->vdev_id = fix_param->vdev_id;
	resp->timer_high = fix_param->tsf_timer_high;
	resp->timer_low = fix_param->tsf_timer_low;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_ocb_ndl_resp_tlv() - extract TSF timer from event buffer
 * @wmi_handle: wmi handle
 * @evt_buf: wmi event buffer
 * @resp: response buffer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS extract_ocb_ndl_resp_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp)
{
	WMI_DCC_UPDATE_NDL_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_dcc_update_ndl_resp_event_fixed_param *fix_param;

	param_tlvs = evt_buf;
	fix_param = param_tlvs->fixed_param;
	resp->vdev_id = fix_param->vdev_id;
	resp->status = fix_param->status;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_ocb_dcc_stats_tlv() - extract DCC stats from event buffer
 * @wmi_handle: wmi handle
 * @evt_buf: wmi event buffer
 * @resp: response buffer
 *
 * Since length of stats is variable, buffer for DCC stats will be allocated
 * in this function. The caller must free the buffer.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS extract_ocb_dcc_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct ocb_dcc_get_stats_response **resp)
{
	struct ocb_dcc_get_stats_response *response;
	WMI_DCC_GET_STATS_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_dcc_get_stats_resp_event_fixed_param *fix_param;

	param_tlvs = (WMI_DCC_GET_STATS_RESP_EVENTID_param_tlvs *)evt_buf;
	fix_param = param_tlvs->fixed_param;

	/* Allocate and populate the response */
	if (fix_param->num_channels > ((WMI_SVC_MSG_MAX_SIZE -
	    sizeof(*fix_param)) / sizeof(wmi_dcc_ndl_stats_per_channel)) ||
	    fix_param->num_channels > param_tlvs->num_stats_per_channel_list) {
		wmi_warn("Too many channels:%d actual:%d",
			 fix_param->num_channels,
			 param_tlvs->num_stats_per_channel_list);
		*resp = NULL;
		return QDF_STATUS_E_INVAL;
	}
	response = qdf_mem_malloc(sizeof(*response) + fix_param->num_channels *
				  sizeof(wmi_dcc_ndl_stats_per_channel));
	*resp = response;
	if (!response)
		return  QDF_STATUS_E_NOMEM;

	response->vdev_id = fix_param->vdev_id;
	response->num_channels = fix_param->num_channels;
	response->channel_stats_array_len =
		fix_param->num_channels *
		sizeof(wmi_dcc_ndl_stats_per_channel);
	response->channel_stats_array = ((uint8_t *)response) +
					sizeof(*response);
	qdf_mem_copy(response->channel_stats_array,
		     param_tlvs->stats_per_channel_list,
		     response->channel_stats_array_len);

	return QDF_STATUS_SUCCESS;
}

void wmi_ocb_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_ocb_set_utc_time_cmd = send_ocb_set_utc_time_cmd_tlv;
	ops->send_ocb_get_tsf_timer_cmd = send_ocb_get_tsf_timer_cmd_tlv;
	ops->send_dcc_clear_stats_cmd = send_dcc_clear_stats_cmd_tlv;
	ops->send_dcc_get_stats_cmd = send_dcc_get_stats_cmd_tlv;
	ops->send_dcc_update_ndl_cmd = send_dcc_update_ndl_cmd_tlv;
	ops->send_ocb_set_config_cmd = send_ocb_set_config_cmd_tlv;
	ops->send_ocb_stop_timing_advert_cmd =
			send_ocb_stop_timing_advert_cmd_tlv;
	ops->send_ocb_start_timing_advert_cmd =
			send_ocb_start_timing_advert_cmd_tlv;
	ops->extract_ocb_chan_config_resp =
			extract_ocb_channel_config_resp_tlv;
	ops->extract_ocb_tsf_timer = extract_ocb_tsf_timer_tlv;
	ops->extract_dcc_update_ndl_resp = extract_ocb_ndl_resp_tlv;
	ops->extract_dcc_stats = extract_ocb_dcc_stats_tlv;
}

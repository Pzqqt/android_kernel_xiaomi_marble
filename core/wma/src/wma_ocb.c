/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wma_ocb.c
 *
 * WLAN Host Device Driver 802.11p OCB implementation
 */

#include "wma_ocb.h"
#include "wmi_unified_api.h"
#include "cds_utils.h"
#include <cdp_txrx_ocb.h>

/**
 * wma_ocb_resp() - send the OCB set config response via callback
 * @wma_handle: pointer to the WMA handle
 * @status: status of the set config command
 */
int wma_ocb_set_config_resp(tp_wma_handle wma_handle, uint8_t status)
{
	QDF_STATUS qdf_status;
	struct sir_ocb_set_config_response *resp;
	cds_msg_t msg = {0};
	struct sir_ocb_config *req = wma_handle->ocb_config_req;
	ol_txrx_vdev_handle vdev = (req ?
		wma_handle->interfaces[req->session_id].handle : NULL);
	struct ol_txrx_ocb_set_chan ocb_set_chan;

	/*
	 * If the command was successful, save the channel information in the
	 * vdev.
	 */
	if (status == QDF_STATUS_SUCCESS && vdev && req) {
		ocb_set_chan.ocb_channel_info = ol_txrx_get_ocb_chan_info(vdev);
		if (ocb_set_chan.ocb_channel_info)
			qdf_mem_free(ocb_set_chan.ocb_channel_info);
		ocb_set_chan.ocb_channel_count =
			req->channel_count;
		if (req->channel_count) {
			int i;
			int buf_size = sizeof(*ocb_set_chan.ocb_channel_info) *
			    req->channel_count;
			ocb_set_chan.ocb_channel_info =
				qdf_mem_malloc(buf_size);
			if (!ocb_set_chan.ocb_channel_info)
				return -ENOMEM;
			qdf_mem_zero(ocb_set_chan.ocb_channel_info, buf_size);
			for (i = 0; i < req->channel_count; i++) {
				ocb_set_chan.ocb_channel_info[i].chan_freq =
					req->channels[i].chan_freq;
				if (req->channels[i].flags &
					OCB_CHANNEL_FLAG_DISABLE_RX_STATS_HDR)
					ocb_set_chan.ocb_channel_info[i].
					disable_rx_stats_hdr = 1;
			}
		} else {
			ocb_set_chan.ocb_channel_info = 0;
			ocb_set_chan.ocb_channel_count = 0;
		}
		ol_txrx_set_ocb_chan_info(vdev, ocb_set_chan);
	}

	/* Free the configuration that was saved in wma_ocb_set_config. */
	qdf_mem_free(wma_handle->ocb_config_req);
	wma_handle->ocb_config_req = NULL;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return -ENOMEM;

	resp->status = status;

	msg.type = eWNI_SME_OCB_SET_CONFIG_RSP;
	msg.bodyptr = resp;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE(FL("Fail to post msg to SME"));
		qdf_mem_free(resp);
		return -EINVAL;
	}

	return 0;
}

/**
 * copy_sir_ocb_config() - deep copy of an OCB config struct
 * @src: pointer to the source struct
 *
 * Return: pointer to the copied struct
 */
static struct sir_ocb_config *copy_sir_ocb_config(struct sir_ocb_config *src)
{
	struct sir_ocb_config *dst;
	uint32_t length;
	void *cursor;

	length = sizeof(*src) +
		src->channel_count * sizeof(*src->channels) +
		src->schedule_size * sizeof(*src->schedule) +
		src->dcc_ndl_chan_list_len +
		src->dcc_ndl_active_state_list_len;

	dst = qdf_mem_malloc(length);
	if (!dst)
		return NULL;

	*dst = *src;

	cursor = dst;
	cursor += sizeof(*dst);
	dst->channels = cursor;
	cursor += src->channel_count * sizeof(*dst->channels);
	qdf_mem_copy(dst->channels, src->channels,
		     src->channel_count * sizeof(*dst->channels));
	dst->schedule = cursor;
	cursor += src->schedule_size * sizeof(*dst->schedule);
	qdf_mem_copy(dst->schedule, src->schedule,
		     src->schedule_size * sizeof(*dst->schedule));
	dst->dcc_ndl_chan_list = cursor;
	cursor += src->dcc_ndl_chan_list_len;
	qdf_mem_copy(dst->dcc_ndl_chan_list, src->dcc_ndl_chan_list,
		     src->dcc_ndl_chan_list_len);
	dst->dcc_ndl_active_state_list = cursor;
	cursor += src->dcc_ndl_active_state_list_len;
	qdf_mem_copy(dst->dcc_ndl_active_state_list,
		     src->dcc_ndl_active_state_list,
		     src->dcc_ndl_active_state_list_len);
	return dst;
}

/**
 * wma_ocb_set_config_req() - send the OCB config request
 * @wma_handle: pointer to the WMA handle
 * @config_req: the configuration to be set.
 */
int wma_ocb_set_config_req(tp_wma_handle wma_handle,
			   struct sir_ocb_config *config_req)
{
	struct wma_target_req *msg;
	struct wma_vdev_start_req req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* if vdev is not yet up, send vdev start request and wait for response.
	 * OCB set_config request should be sent on receiving
	 * vdev start response message
	 */
	if (!wma_handle->interfaces[config_req->session_id].vdev_up) {
		qdf_mem_zero(&req, sizeof(req));
		/* Enqueue OCB Set Schedule request message */
		msg = wma_fill_vdev_req(wma_handle, config_req->session_id,
					WMA_OCB_SET_CONFIG_CMD,
					WMA_TARGET_REQ_TYPE_VDEV_START,
					(void *)config_req, 1000);
		if (!msg) {
			WMA_LOGE(FL("Failed to fill vdev req %d"), req.vdev_id);
			status = QDF_STATUS_E_NOMEM;
			return status;
		}
		req.chan = cds_freq_to_chan(config_req->channels[0].chan_freq);
		req.vdev_id = msg->vdev_id;
		if (cds_chan_to_band(req.chan) == CDS_BAND_2GHZ)
			req.dot11_mode = WNI_CFG_DOT11_MODE_11G;
		else
			req.dot11_mode = WNI_CFG_DOT11_MODE_11A;

		if (wma_handle->ocb_config_req)
			qdf_mem_free(wma_handle->ocb_config_req);
		wma_handle->ocb_config_req = copy_sir_ocb_config(config_req);
		req.preferred_rx_streams = 2;
		req.preferred_tx_streams = 2;

		status = wma_vdev_start(wma_handle, &req, false);
		if (status != QDF_STATUS_SUCCESS) {
			wma_remove_vdev_req(wma_handle, req.vdev_id,
					    WMA_TARGET_REQ_TYPE_VDEV_START);
			WMA_LOGE(FL("vdev_start failed, status = %d"), status);
		}
		return 0;
	} else {
		return wma_ocb_set_config(wma_handle, config_req);
	}
}

int wma_ocb_start_resp_ind_cont(tp_wma_handle wma_handle)
{
	QDF_STATUS qdf_status = 0;

	if (!wma_handle->ocb_config_req) {
		WMA_LOGE(FL("The request could not be found"));
		return QDF_STATUS_E_EMPTY;
	}

	qdf_status = wma_ocb_set_config(wma_handle, wma_handle->ocb_config_req);
	return qdf_status;
}

static WLAN_PHY_MODE wma_ocb_freq_to_mode(uint32_t freq)
{
	if (cds_chan_to_band(cds_freq_to_chan(freq)) == CDS_BAND_2GHZ)
		return MODE_11G;
	else
		return MODE_11A;
}

/**
 * wma_send_ocb_set_config() - send the OCB config to the FW
 * @wma_handle: pointer to the WMA handle
 * @config: the OCB configuration
 *
 * Return: 0 on success
 */
int wma_ocb_set_config(tp_wma_handle wma_handle, struct sir_ocb_config *config)
{
	int32_t ret, i;
	uint32_t *ch_mhz;
	struct ocb_config_param tconfig = {0};

	tconfig.session_id = config->session_id;
	tconfig.channel_count = config->channel_count;
	tconfig.schedule_size = config->schedule_size;
	tconfig.flags = config->flags;
	tconfig.channels = (struct ocb_config_channel *)config->channels;
	tconfig.schedule = (struct ocb_config_sched *)config->schedule;
	tconfig.dcc_ndl_chan_list_len = config->dcc_ndl_chan_list_len;
	tconfig.dcc_ndl_chan_list = config->dcc_ndl_chan_list;
	tconfig.dcc_ndl_active_state_list_len = config->dcc_ndl_active_state_list_len;
	tconfig.dcc_ndl_active_state_list = config->dcc_ndl_active_state_list;
	ch_mhz = qdf_mem_malloc(sizeof(uint32_t)*config->channel_count);
	if (ch_mhz == NULL) {
		WMA_LOGE(FL("Memory allocation failed"));
		return -ENOMEM;
	}

	for (i = 0; i < config->channel_count; i++)
		ch_mhz[i] = wma_ocb_freq_to_mode(config->channels[i].chan_freq);

	/*
	 * Save the configuration so that it can be used in
	 * wma_ocb_set_config_event_handler.
	 */
	if (wma_handle->ocb_config_req != config) {
		if (wma_handle->ocb_config_req)
			qdf_mem_free(wma_handle->ocb_config_req);
		wma_handle->ocb_config_req = copy_sir_ocb_config(config);
	}

	ret = wmi_unified_ocb_set_config(wma_handle->wmi_handle, &tconfig,
				     ch_mhz);
	if (ret != EOK) {
		if (wma_handle->ocb_config_req) {
			qdf_mem_free(wma_handle->ocb_config_req);
			wma_handle->ocb_config_req = NULL;
		}
		qdf_mem_free(ch_mhz);
		WMA_LOGE("Failed to set OCB config");
		return -EIO;
	}
	qdf_mem_free(ch_mhz);

	return 0;
}

/**
 * wma_ocb_set_config_event_handler() - Response event for the set config cmd
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * Return: 0 on success
 */
int wma_ocb_set_config_event_handler(void *handle, uint8_t *event_buf,
				     uint32_t len)
{
	WMI_OCB_SET_CONFIG_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_ocb_set_config_resp_event_fixed_param *fix_param;
	param_tlvs = (WMI_OCB_SET_CONFIG_RESP_EVENTID_param_tlvs *)event_buf;
	fix_param = param_tlvs->fixed_param;
	return wma_ocb_set_config_resp(handle, fix_param->status);
};

/**
 * wma_ocb_set_utc_time() - send the UTC time to the firmware
 * @wma_handle: pointer to the WMA handle
 * @utc: pointer to the UTC time struct
 *
 * Return: 0 on succes
 */
int wma_ocb_set_utc_time(tp_wma_handle wma_handle, struct sir_ocb_utc *utc)
{
	int32_t ret;
	struct ocb_utc_param cmd = {0};

	cmd.vdev_id = utc->vdev_id;
	qdf_mem_copy(&cmd.utc_time, &utc->utc_time, WMI_SIZE_UTC_TIME);
	qdf_mem_copy(&cmd.time_error, &utc->time_error, WMI_SIZE_UTC_TIME_ERROR);
	ret = wmi_unified_ocb_set_utc_time_cmd(wma_handle->wmi_handle, &cmd);
	if (ret != EOK) {
		WMA_LOGE(FL("Failed to set OCB UTC time"));
		return -EIO;
	}

	return 0;
}

/**
 * wma_ocb_start_timing_advert() - start sending the timing advertisement
 *				   frames on a channel
 * @wma_handle: pointer to the WMA handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
int wma_ocb_start_timing_advert(tp_wma_handle wma_handle,
	struct sir_ocb_timing_advert *timing_advert)
{
	int32_t ret;
	struct ocb_timing_advert_param cmd = {0};

	cmd.vdev_id = timing_advert->vdev_id;
	cmd.repeat_rate = timing_advert->repeat_rate;
	cmd.chan_freq = timing_advert->chan_freq;
	cmd.timestamp_offset = timing_advert->timestamp_offset;
	cmd.time_value_offset = timing_advert->time_value_offset;
	cmd.template_length = timing_advert->template_length;
	cmd.template_value = (uint8_t *)timing_advert->template_value;

	ret = wmi_unified_ocb_start_timing_advert(wma_handle->wmi_handle,
				   &cmd);
	if (ret != EOK) {
		WMA_LOGE(FL("Failed to start OCB timing advert"));
		return -EIO;
	}

	return 0;
}

/**
 * wma_ocb_stop_timing_advert() - stop sending the timing advertisement frames
 *				  on a channel
 * @wma_handle: pointer to the WMA handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
int wma_ocb_stop_timing_advert(tp_wma_handle wma_handle,
	struct sir_ocb_timing_advert *timing_advert)
{
	int32_t ret;
	struct ocb_timing_advert_param cmd = {0};

	cmd.vdev_id = timing_advert->vdev_id;
	cmd.chan_freq = timing_advert->chan_freq;
	ret = wmi_unified_ocb_stop_timing_advert(wma_handle->wmi_handle,
				   &cmd);
	if (ret != EOK) {
		WMA_LOGE(FL("Failed to stop OCB timing advert"));
		return -EIO;
	}

	return 0;
}

/**
 * wma_ocb_get_tsf_timer() - stop sending the timing advertisement frames on a
 *			     channel
 * @wma_handle: pointer to the WMA handle
 * @request: pointer to the request
 *
 * Return: 0 on succes
 */
int wma_ocb_get_tsf_timer(tp_wma_handle wma_handle,
			  struct sir_ocb_get_tsf_timer *request)
{
	QDF_STATUS ret;

	/* Send the WMI command */
	ret = wmi_unified_ocb_get_tsf_timer(wma_handle->wmi_handle, request->vdev_id);
	/* If there is an error, set the completion event */
	if (ret != EOK) {
		WMA_LOGE(FL("Failed to send WMI message: %d"), ret);
		return -EIO;
	}
	return 0;
}

/**
 * wma_ocb_get_tsf_timer_resp_event_handler() - Event for the get TSF timer cmd
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * Return: 0 on success
 */
int wma_ocb_get_tsf_timer_resp_event_handler(void *handle, uint8_t *event_buf,
					     uint32_t len)
{
	QDF_STATUS qdf_status;
	struct sir_ocb_get_tsf_timer_response *response;
	WMI_OCB_GET_TSF_TIMER_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_ocb_get_tsf_timer_resp_event_fixed_param *fix_param;
	cds_msg_t msg = {0};

	param_tlvs = (WMI_OCB_GET_TSF_TIMER_RESP_EVENTID_param_tlvs *)event_buf;
	fix_param = param_tlvs->fixed_param;

	/* Allocate and populate the response */
	response = qdf_mem_malloc(sizeof(*response));
	if (response == NULL)
		return -ENOMEM;
	response->vdev_id = fix_param->vdev_id;
	response->timer_high = fix_param->tsf_timer_high;
	response->timer_low = fix_param->tsf_timer_low;

	msg.type = eWNI_SME_OCB_GET_TSF_TIMER_RSP;
	msg.bodyptr = response;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE(FL("Failed to post msg to SME"));
		qdf_mem_free(response);
		return -EINVAL;
	}

	return 0;
}

/**
 * wma_dcc_get_stats() - get the DCC channel stats
 * @wma_handle: pointer to the WMA handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: 0 on succes
 */
int wma_dcc_get_stats(tp_wma_handle wma_handle,
		      struct sir_dcc_get_stats *get_stats_param)
{
	int32_t ret;
	struct dcc_get_stats_param cmd = {0};

	cmd.vdev_id = get_stats_param->vdev_id;
	cmd.channel_count = get_stats_param->channel_count;
	cmd.request_array_len = get_stats_param->request_array_len;
	cmd.request_array = get_stats_param->request_array;

	/* Send the WMI command */
	ret = wmi_unified_dcc_get_stats_cmd(wma_handle->wmi_handle, &cmd);

	if (ret != EOK) {
		WMA_LOGE(FL("Failed to send WMI message: %d"), ret);
		return -EIO;
	}

	return 0;
}

/**
 * wma_dcc_get_stats_resp_event_handler() - Response event for the get stats cmd
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * Return: 0 on success
 */
int wma_dcc_get_stats_resp_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	QDF_STATUS qdf_status;
	struct sir_dcc_get_stats_response *response;
	WMI_DCC_GET_STATS_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_dcc_get_stats_resp_event_fixed_param *fix_param;
	cds_msg_t msg = {0};

	param_tlvs = (WMI_DCC_GET_STATS_RESP_EVENTID_param_tlvs *)event_buf;
	fix_param = param_tlvs->fixed_param;

	/* Allocate and populate the response */
	response = qdf_mem_malloc(sizeof(*response) + fix_param->num_channels *
		sizeof(wmi_dcc_ndl_stats_per_channel));
	if (response == NULL)
		return -ENOMEM;

	response->vdev_id = fix_param->vdev_id;
	response->num_channels = fix_param->num_channels;
	response->channel_stats_array_len =
		fix_param->num_channels * sizeof(wmi_dcc_ndl_stats_per_channel);
	response->channel_stats_array = ((void *)response) + sizeof(*response);
	qdf_mem_copy(response->channel_stats_array,
		     param_tlvs->stats_per_channel_list,
		     response->channel_stats_array_len);

	msg.type = eWNI_SME_DCC_GET_STATS_RSP;
	msg.bodyptr = response;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE(FL("Failed to post msg to SME"));
		qdf_mem_free(response);
		return -EINVAL;
	}

	return 0;
}

/**
 * wma_dcc_clear_stats() - command to clear the DCC stats
 * @wma_handle: pointer to the WMA handle
 * @clear_stats_param: parameters to the command
 *
 * Return: 0 on succes
 */
int wma_dcc_clear_stats(tp_wma_handle wma_handle,
			struct sir_dcc_clear_stats *clear_stats_param)
{
	int32_t ret;

	/* Send the WMI command */
	ret = wmi_unified_dcc_clear_stats(wma_handle->wmi_handle,
				   clear_stats_param->vdev_id,
				   clear_stats_param->dcc_stats_bitmap);
	if (ret != EOK) {
		WMA_LOGE(FL("Failed to send the WMI command"));
		return -EIO;
	}

	return 0;
}

/**
 * wma_dcc_update_ndl() - command to update the NDL data
 * @wma_handle: pointer to the WMA handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: 0 on success
 */
int wma_dcc_update_ndl(tp_wma_handle wma_handle,
		       struct sir_dcc_update_ndl *update_ndl_param)
{
	QDF_STATUS qdf_status;
	struct dcc_update_ndl_param *cmd;

	cmd = (struct dcc_update_ndl_param *) update_ndl_param;
	/* Send the WMI command */
	qdf_status = wmi_unified_dcc_update_ndl(wma_handle->wmi_handle,
				   cmd);
	/* If there is an error, set the completion event */
	if (qdf_status) {
		WMA_LOGE(FL("Failed to send WMI message: %d"), qdf_status);
		return -EIO;
	}

	return 0;
}

/**
 * wma_dcc_update_ndl_resp_event_handler() - Response event for the update NDL
 * command
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * Return: 0 on success
 */
int wma_dcc_update_ndl_resp_event_handler(void *handle, uint8_t *event_buf,
					  uint32_t len)
{
	QDF_STATUS qdf_status;
	struct sir_dcc_update_ndl_response *resp;
	WMI_DCC_UPDATE_NDL_RESP_EVENTID_param_tlvs *param_tlvs;
	wmi_dcc_update_ndl_resp_event_fixed_param *fix_param;
	cds_msg_t msg = {0};

	param_tlvs = (WMI_DCC_UPDATE_NDL_RESP_EVENTID_param_tlvs *)event_buf;
	fix_param = param_tlvs->fixed_param;
	/* Allocate and populate the response */
	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp) {
		WMA_LOGE(FL("Error allocating memory for the response."));
		return -ENOMEM;
	}
	resp->vdev_id = fix_param->vdev_id;
	resp->status = fix_param->status;

	msg.type = eWNI_SME_DCC_UPDATE_NDL_RSP;
	msg.bodyptr = resp;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))	{
		WMA_LOGE(FL("Failed to post msg to SME"));
		qdf_mem_free(resp);
		return -EINVAL;
	}

	return 0;
}

/**
 * wma_dcc_stats_event_handler() - Response event for the get stats cmd
 * @handle: the WMA handle
 * @event_buf: buffer with the event parameters
 * @len: length of the buffer
 *
 * Return: 0 on success
 */
int wma_dcc_stats_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	QDF_STATUS qdf_status;
	struct sir_dcc_get_stats_response *response;
	WMI_DCC_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_dcc_stats_event_fixed_param *fix_param;
	cds_msg_t msg = {0};

	param_tlvs = (WMI_DCC_STATS_EVENTID_param_tlvs *)event_buf;
	fix_param = param_tlvs->fixed_param;
	/* Allocate and populate the response */
	response = qdf_mem_malloc(sizeof(*response) +
	    fix_param->num_channels * sizeof(wmi_dcc_ndl_stats_per_channel));
	if (response == NULL)
		return -ENOMEM;
	response->vdev_id = fix_param->vdev_id;
	response->num_channels = fix_param->num_channels;
	response->channel_stats_array_len =
		fix_param->num_channels * sizeof(wmi_dcc_ndl_stats_per_channel);
	response->channel_stats_array = ((void *)response) + sizeof(*response);
	qdf_mem_copy(response->channel_stats_array,
		     param_tlvs->stats_per_channel_list,
		     response->channel_stats_array_len);

	msg.type = eWNI_SME_DCC_STATS_EVENT;
	msg.bodyptr = response;

	qdf_status = cds_mq_post_message(QDF_MODULE_ID_SME, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))	{
		WMA_LOGE(FL("Failed to post msg to SME"));
		qdf_mem_free(response);
		return -EINVAL;
	}

	return 0;
}

/**
 * wma_ocb_register_event_handlers() - register handlers for the OCB WMI
 * events
 * @wma_handle: pointer to the WMA handle
 *
 * Return: 0 on success, non-zero on failure
 */
int wma_ocb_register_event_handlers(tp_wma_handle wma_handle)
{
	int status;

	if (!wma_handle) {
		WMA_LOGE(FL("wma_handle is NULL"));
		return -EINVAL;
	}

	/* Initialize the members in WMA used by wma_ocb */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_OCB_SET_CONFIG_RESP_EVENTID,
			wma_ocb_set_config_event_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status)
		return status;

	status = wmi_unified_register_event_handler(
			wma_handle->wmi_handle,
			WMI_OCB_GET_TSF_TIMER_RESP_EVENTID,
			wma_ocb_get_tsf_timer_resp_event_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status)
		return status;

	status = wmi_unified_register_event_handler(
			wma_handle->wmi_handle,
			WMI_DCC_GET_STATS_RESP_EVENTID,
			wma_dcc_get_stats_resp_event_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status)
		return status;

	status = wmi_unified_register_event_handler(
			wma_handle->wmi_handle,
			WMI_DCC_UPDATE_NDL_RESP_EVENTID,
			wma_dcc_update_ndl_resp_event_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status)
		return status;

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_DCC_STATS_EVENTID,
			wma_dcc_stats_event_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status)
		return status;

	return QDF_STATUS_SUCCESS;
}

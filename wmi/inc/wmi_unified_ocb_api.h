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
 * DOC: Implement API's specific to OCB component.
 */

#ifndef _WMI_UNIFIED_DSRC_API_H_
#define _WMI_UNIFIED_DSRC_API_H_
#include <wlan_ocb_public_structs.h>

/**
 * wmi_unified_ocb_start_timing_advert() - start sending the timing
 *  advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_start_timing_advert(struct wmi_unified *wmi_handle,
			struct ocb_timing_advert_param *timing_advert);

/**
 * wmi_unified_ocb_stop_timing_advert() - stop sending the timing
 *  advertisement frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_stop_timing_advert(struct wmi_unified *wmi_handle,
			struct ocb_timing_advert_param *timing_advert);

/**
 * wmi_unified_ocb_set_config() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_ocb_set_config(struct wmi_unified *wmi_handle,
				      struct ocb_config *config);

/**
 * wmi_unified_ocb_get_tsf_timer() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @req: request for tsf timer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_get_tsf_timer(struct wmi_unified *wmi_handle,
					 struct ocb_get_tsf_timer_param *req);

/**
 * wmi_unified_ocb_set_utc_time_cmd() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_ocb_set_utc_time_cmd(struct wmi_unified *wmi_handle,
					    struct ocb_utc_param *utc);

/**
 * wmi_unified_dcc_get_stats_cmd() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_dcc_get_stats_cmd(struct wmi_unified *wmi_handle,
		     struct ocb_dcc_get_stats_param *get_stats_param);

/**
 * wmi_unified_dcc_clear_stats() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @clear_stats_param: parameters to the command
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_dcc_clear_stats(struct wmi_unified *wmi_handle,
			struct ocb_dcc_clear_stats_param *clear_stats_param);

/**
 * wmi_unified_dcc_update_ndl() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures
 */
QDF_STATUS wmi_unified_dcc_update_ndl(struct wmi_unified *wmi_handle,
		       struct ocb_dcc_update_ndl_param *update_ndl_param);

/**
 * wmi_extract_ocb_set_channel_config_resp() - extract status from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @status: status buffer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
wmi_extract_ocb_set_channel_config_resp(struct wmi_unified *wmi_handle,
					void *evt_buf,
					uint32_t *status);

/**
 * wmi_extract_ocb_tsf_timer() - extract tsf timer from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: tsf timer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_ocb_tsf_timer(struct wmi_unified *wmi_handle,
				     void *evt_buf,
				     struct ocb_get_tsf_timer_response *resp);

/**
 * wmi_extract_dcc_update_ndl_resp() - extract NDL update from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: ndl update status
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_dcc_update_ndl_resp(struct wmi_unified *wmi_handle,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp);

/**
 * wmi_extract_dcc_stats() - extract DCC stats from wmi event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: DCC stats
 *
 * Since length of the response is variable, response buffer will be allocated.
 * The caller must free the response buffer.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS wmi_extract_dcc_stats(struct wmi_unified *wmi_handle,
				 void *evt_buf,
				 struct ocb_dcc_get_stats_response **response);

#endif /* _WMI_UNIFIED_DSRC_API_H_ */

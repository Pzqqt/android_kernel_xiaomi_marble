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
 * DOC: Implement API's specific to DSRC component.
 */

#include <wmi_unified_priv.h>
#include <wlan_ocb_public_structs.h>
#include <wmi_unified_ocb_api.h>

QDF_STATUS wmi_unified_ocb_start_timing_advert(struct wmi_unified *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	if (wmi_hdl->ops->send_ocb_start_timing_advert_cmd)
		return wmi_hdl->ops->send_ocb_start_timing_advert_cmd(wmi_hdl,
							timing_advert);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_stop_timing_advert(struct wmi_unified *wmi_hdl,
	struct ocb_timing_advert_param *timing_advert)
{
	if (wmi_hdl->ops->send_ocb_stop_timing_advert_cmd)
		return wmi_hdl->ops->send_ocb_stop_timing_advert_cmd(wmi_hdl,
							timing_advert);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_set_utc_time_cmd(struct wmi_unified *wmi_hdl,
					    struct ocb_utc_param *utc)
{
	if (wmi_hdl->ops->send_ocb_set_utc_time_cmd)
		return wmi_hdl->ops->send_ocb_set_utc_time_cmd(wmi_hdl, utc);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_get_tsf_timer(struct wmi_unified *wmi_hdl,
					 struct ocb_get_tsf_timer_param *req)
{
	if (wmi_hdl->ops->send_ocb_get_tsf_timer_cmd)
		return wmi_hdl->ops->send_ocb_get_tsf_timer_cmd(wmi_hdl,
								req->vdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_get_stats_cmd(struct wmi_unified *wmi_hdl,
			struct ocb_dcc_get_stats_param *get_stats_param)
{
	if (wmi_hdl->ops->send_dcc_get_stats_cmd)
		return wmi_hdl->ops->send_dcc_get_stats_cmd(wmi_hdl,
							    get_stats_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_clear_stats(struct wmi_unified *wmi_hdl,
		struct ocb_dcc_clear_stats_param *clear_stats_param)
{
	if (wmi_hdl->ops->send_dcc_clear_stats_cmd)
		return wmi_hdl->ops->send_dcc_clear_stats_cmd(wmi_hdl,
				clear_stats_param->vdev_id,
				clear_stats_param->dcc_stats_bitmap);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_dcc_update_ndl(struct wmi_unified *wmi_hdl,
			struct ocb_dcc_update_ndl_param *update_ndl_param)
{
	if (wmi_hdl->ops->send_dcc_update_ndl_cmd)
		return wmi_hdl->ops->send_dcc_update_ndl_cmd(wmi_hdl,
					update_ndl_param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_ocb_set_config(struct wmi_unified *wmi_hdl,
				      struct ocb_config *config)
{
	if (wmi_hdl->ops->send_ocb_set_config_cmd)
		return wmi_hdl->ops->send_ocb_set_config_cmd(wmi_hdl,
							     config);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_ocb_set_channel_config_resp(struct wmi_unified *wmi_hdl,
					void *evt_buf,
					uint32_t *status)
{
	if (wmi_hdl->ops->extract_ocb_chan_config_resp)
		return wmi_hdl->ops->extract_ocb_chan_config_resp(wmi_hdl,
								  evt_buf,
								  status);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_ocb_tsf_timer(struct wmi_unified *wmi_hdl,
				     void *evt_buf,
				     struct ocb_get_tsf_timer_response *resp)
{
	if (wmi_hdl->ops->extract_ocb_tsf_timer)
		return wmi_hdl->ops->extract_ocb_tsf_timer(wmi_hdl,
							   evt_buf,
							   resp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dcc_update_ndl_resp(struct wmi_unified *wmi_hdl,
		void *evt_buf, struct ocb_dcc_update_ndl_response *resp)
{
	if (wmi_hdl->ops->extract_dcc_update_ndl_resp)
		return wmi_hdl->ops->extract_dcc_update_ndl_resp(wmi_hdl,
								 evt_buf,
								 resp);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_dcc_stats(struct wmi_unified *wmi_hdl,
				 void *evt_buf,
				 struct ocb_dcc_get_stats_response **resp)
{
	if (wmi_hdl->ops->extract_dcc_stats)
		return wmi_hdl->ops->extract_dcc_stats(wmi_hdl,
						       evt_buf,
						       resp);

	return QDF_STATUS_E_FAILURE;
}


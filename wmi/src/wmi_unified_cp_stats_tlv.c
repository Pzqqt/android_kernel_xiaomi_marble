/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "osdep.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "target_if_cp_stats.h"
#include <wlan_cp_stats_public_structs.h>

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
#ifdef WLAN_SUPPORT_TWT
static uint32_t
get_stats_req_twt_dialog_id(struct infra_cp_stats_cmd_info *req)
{
	return req->dialog_id;
}

static enum WMI_HOST_GET_STATS_TWT_STATUS
wmi_get_converted_twt_get_stats_status(WMI_GET_STATS_TWT_STATUS_T tgt_status)
{
	switch (tgt_status) {
	case WMI_GET_STATS_TWT_STATUS_OK:
		return WMI_HOST_GET_STATS_TWT_STATUS_OK;
	case WMI_GET_STATS_TWT_STATUS_DIALOG_ID_NOT_EXIST:
		return WMI_HOST_GET_STATS_TWT_STATUS_DIALOG_ID_NOT_EXIST;
	case WMI_GET_STATS_TWT_STATUS_INVALID_PARAM:
		return WMI_HOST_GET_STATS_TWT_STATUS_INVALID_PARAM;
	default:
		return WMI_HOST_GET_STATS_TWT_STATUS_UNKNOWN_ERROR;
	}
}

static inline
void wmi_extract_ctrl_path_twt_stats_tlv(void *tag_buf,
					 struct twt_infra_cp_stats_event *param)
{
	wmi_ctrl_path_twt_stats_struct *wmi_stats_buf =
			(wmi_ctrl_path_twt_stats_struct *)tag_buf;

	param->dialog_id = wmi_stats_buf->dialog_id;
	param->status = wmi_get_converted_twt_get_stats_status(wmi_stats_buf->status);
	param->num_sp_cycles = wmi_stats_buf->num_sp_cycles;
	param->avg_sp_dur_us = wmi_stats_buf->avg_sp_dur_us;
	param->min_sp_dur_us = wmi_stats_buf->min_sp_dur_us;
	param->max_sp_dur_us = wmi_stats_buf->max_sp_dur_us;
	param->tx_mpdu_per_sp = wmi_stats_buf->tx_mpdu_per_sp;
	param->rx_mpdu_per_sp = wmi_stats_buf->rx_mpdu_per_sp;
	param->tx_bytes_per_sp = wmi_stats_buf->tx_bytes_per_sp;
	param->rx_bytes_per_sp = wmi_stats_buf->rx_bytes_per_sp;

	wmi_debug("dialog_id = %u status = %u", wmi_stats_buf->dialog_id,
		  wmi_stats_buf->status);
	wmi_debug("num_sp_cycles = %u avg_sp_dur_us = 0x%x, \
		  min_sp_dur_us = 0x%x, max_sp_dur_us = 0x%x",
		  wmi_stats_buf->num_sp_cycles, wmi_stats_buf->avg_sp_dur_us,
		  wmi_stats_buf->min_sp_dur_us, wmi_stats_buf->max_sp_dur_us);
	wmi_debug("tx_mpdu_per_sp 0x%x, rx_mpdu_per_sp = 0x%x, \
		  tx_bytes_per_sp = 0x%x, rx_bytes_per_sp = 0x%x",
		  wmi_stats_buf->tx_mpdu_per_sp, wmi_stats_buf->rx_mpdu_per_sp,
		  wmi_stats_buf->tx_bytes_per_sp,
		  wmi_stats_buf->rx_bytes_per_sp);
}

static void wmi_twt_extract_stats_struct(void *tag_buf,
					 struct infra_cp_stats_event *params)
{
	struct twt_infra_cp_stats_event *twt_params;

	twt_params = params->twt_infra_cp_stats +
		     params->num_twt_infra_cp_stats;

	wmi_debug("TWT stats struct found - num_twt_cp_stats %d",
		  params->num_twt_infra_cp_stats);

	params->num_twt_infra_cp_stats++;
	wmi_extract_ctrl_path_twt_stats_tlv(tag_buf, twt_params);
}
#else
static inline
uint32_t get_stats_req_twt_dialog_id(struct infra_cp_stats_cmd_info *req)
{
	return 0;
}

static void wmi_twt_extract_stats_struct(void *tag_buf,
					 struct infra_cp_stats_event *params)
{
}
#endif /* WLAN_SUPPORT_TWT */

/*
 * wmi_stats_extract_tag_struct: function to extract tag structs
 * @tag_type: tag type that is to be printed
 * @tag_buf: pointer to the tag structure
 * @params: buffer to hold parameters extracted from response event
 *
 * Return: None
 */
static void wmi_stats_extract_tag_struct(uint32_t tag_type, void *tag_buf,
					 struct infra_cp_stats_event *params)
{
	wmi_debug("tag_type %d", tag_type);

	switch (tag_type) {
	case WMITLV_TAG_STRUC_wmi_ctrl_path_pdev_stats_struct:
		break;

	case WMITLV_TAG_STRUC_wmi_ctrl_path_mem_stats_struct:
		break;

	case WMITLV_TAG_STRUC_wmi_ctrl_path_twt_stats_struct:
		wmi_twt_extract_stats_struct(tag_buf, params);
		break;

	default:
		break;
	}
}

/*
 * wmi_stats_handler: parse the wmi event and fill the stats values
 * @buff: Buffer containing wmi event
 * @len: length of event buffer
 * @params: buffer to hold parameters extracted from response event
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
QDF_STATUS wmi_stats_handler(void *buff, int32_t len,
			     struct infra_cp_stats_event *params)
{
	WMI_CTRL_PATH_STATS_EVENTID_param_tlvs *param_buf;
	wmi_ctrl_path_stats_event_fixed_param *ev;
	uint8_t *buf_ptr = (uint8_t *)buff;
	uint32_t curr_tlv_tag;
	uint32_t curr_tlv_len;
	uint8_t *tag_start_ptr;

	param_buf = (WMI_CTRL_PATH_STATS_EVENTID_param_tlvs *)buff;
	if (!param_buf) {
		wmi_err_rl("param_buf is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ev = (wmi_ctrl_path_stats_event_fixed_param *)param_buf->fixed_param;

	curr_tlv_tag = WMITLV_GET_TLVTAG(ev->tlv_header);
	curr_tlv_len = WMITLV_GET_TLVLEN(ev->tlv_header);
	buf_ptr = (uint8_t *)param_buf->fixed_param;
	wmi_debug("Fixed param more %d req_id %d status %d", ev->more,
		  ev->request_id, ev->status);
	params->request_id = ev->request_id;
	params->status = ev->status;

	/* buffer should point to next TLV in event */
	buf_ptr += (curr_tlv_len + WMI_TLV_HDR_SIZE);
	len -= (curr_tlv_len + WMI_TLV_HDR_SIZE);

	curr_tlv_tag = WMITLV_GET_TLVTAG(WMITLV_GET_HDR(buf_ptr));
	curr_tlv_len = WMITLV_GET_TLVLEN(WMITLV_GET_HDR(buf_ptr));

	wmi_debug("curr_tlv_len %d curr_tlv_tag %d rem_len %d", len,
		  curr_tlv_len, curr_tlv_tag);

	while ((len >= curr_tlv_len) &&
	       (curr_tlv_tag >= WMITLV_TAG_FIRST_ARRAY_ENUM)) {
		if (curr_tlv_tag == WMITLV_TAG_ARRAY_STRUC) {
			/* Move to next WMITLV_TAG_ARRAY_STRUC */
			buf_ptr += WMI_TLV_HDR_SIZE;
			len -= WMI_TLV_HDR_SIZE;
			if (len <= 0)
				break;
		}
		curr_tlv_tag = WMITLV_GET_TLVTAG(WMITLV_GET_HDR(buf_ptr));
		curr_tlv_len = WMITLV_GET_TLVLEN(WMITLV_GET_HDR(buf_ptr));

		wmi_debug("curr_tlv_len %d curr_tlv_tag %d rem_len %d",
			  len, curr_tlv_len, curr_tlv_tag);
		if (curr_tlv_len) {
			/* point to the tag inside WMITLV_TAG_ARRAY_STRUC */
			tag_start_ptr = buf_ptr + WMI_TLV_HDR_SIZE;
			curr_tlv_tag = WMITLV_GET_TLVTAG(
						WMITLV_GET_HDR(tag_start_ptr));
			wmi_stats_extract_tag_struct(curr_tlv_tag,
						     (void *)tag_start_ptr,
						     params);
		}
		/* Move to next tag */
		buf_ptr += curr_tlv_len + WMI_TLV_HDR_SIZE;
		len -= (curr_tlv_len + WMI_TLV_HDR_SIZE);

		if (len <= 0)
			break;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_infra_cp_stats_tlv - api to extract stats information from
 * event buffer
 * @wmi_handle:  wmi handle
 * @evt_buf:     event buffer
 * @evt_buf_len: length of the event buffer
 * @params:      buffer to populate more flag
 *
 * Return: QDF_STATUS_SUCCESS on success, else other qdf error values
 */
QDF_STATUS
extract_infra_cp_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			   uint32_t evt_buf_len,
			   struct infra_cp_stats_event *params)
{
	wmi_stats_handler(evt_buf, evt_buf_len, params);
	return QDF_STATUS_SUCCESS;
}

/**
 * prepare_infra_cp_stats_buf() - Allocate and prepate wmi cmd request buffer
 * @wmi_handle: wmi handle
 * @stats_req: Request parameters to be filled in wmi cmd request buffer
 * @req_buf_len: length of the output wmi cmd buffer allocated
 *
 * Return: Valid wmi buffer pointer on success and NULL pointer for failure
 */
static wmi_buf_t
prepare_infra_cp_stats_buf(wmi_unified_t wmi_handle,
			   struct infra_cp_stats_cmd_info *stats_req,
			   uint32_t *req_buf_len)
{
	wmi_request_ctrl_path_stats_cmd_fixed_param *cmd_fixed_param;
	uint32_t index;
	wmi_buf_t req_buf;
	uint8_t *buf_ptr;
	uint32_t *pdev_id_array;
	uint32_t *vdev_id_array;
	uint8_t *mac_addr_array;
	uint32_t *dialog_id_array;
	uint32_t num_pdev_ids = stats_req->num_pdev_ids;
	uint32_t num_vdev_ids = stats_req->num_vdev_ids;
	uint32_t num_mac_addr_list = stats_req->num_mac_addr_list;
	uint32_t num_dialog_ids = INFRA_CP_STATS_MAX_REQ_TWT_DIALOG_ID;

	/* Calculate total buffer length */
	*req_buf_len = (sizeof(wmi_request_ctrl_path_stats_cmd_fixed_param) +
		       WMI_TLV_HDR_SIZE + (sizeof(A_UINT32) * (num_pdev_ids)) +
		       WMI_TLV_HDR_SIZE + sizeof(A_UINT32) * (num_vdev_ids) +
		       WMI_TLV_HDR_SIZE +
		       sizeof(wmi_mac_addr) * (num_mac_addr_list) +
		       WMI_TLV_HDR_SIZE +
		       (sizeof(A_UINT32) * (num_dialog_ids)));
	req_buf = wmi_buf_alloc(wmi_handle, *req_buf_len);
	if (!req_buf)
		return NULL;

	cmd_fixed_param = (wmi_request_ctrl_path_stats_cmd_fixed_param *)
				wmi_buf_data(req_buf);

	/*Set TLV header*/
	WMITLV_SET_HDR(&cmd_fixed_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_request_ctrl_path_stats_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_request_ctrl_path_stats_cmd_fixed_param));

	index = get_infra_cp_stats_id(stats_req->stats_id);
	cmd_fixed_param->stats_id_mask = (1 << index);

	cmd_fixed_param->request_id = stats_req->action;
	cmd_fixed_param->action = get_infra_cp_stats_action(stats_req->action);

	buf_ptr = (uint8_t *)cmd_fixed_param;
	/* Setting tlv header for pdev id arrays*/
	buf_ptr = buf_ptr + sizeof(*cmd_fixed_param);
	pdev_id_array = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(buf_ptr,  WMITLV_TAG_ARRAY_UINT32,
		       sizeof(A_UINT32) * num_pdev_ids);

	/* Setting tlv header for vdev id arrays*/
	buf_ptr = buf_ptr + WMI_TLV_HDR_SIZE +
		  (sizeof(A_UINT32) * num_pdev_ids);
	vdev_id_array = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       sizeof(A_UINT32) * num_vdev_ids);

	/* Setting tlv header for mac addr arrays*/
	buf_ptr = buf_ptr + WMI_TLV_HDR_SIZE +
		  (sizeof(A_UINT32) * num_vdev_ids);
	mac_addr_array = buf_ptr + WMI_TLV_HDR_SIZE;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       sizeof(wmi_mac_addr) * num_mac_addr_list);

	/* Setting tlv header for dialog id arrays*/
	buf_ptr = buf_ptr + WMI_TLV_HDR_SIZE +
		  sizeof(wmi_mac_addr) * num_mac_addr_list;
	dialog_id_array = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       sizeof(A_UINT32) * num_dialog_ids);

	for (index = 0; index < num_pdev_ids; index++)
		pdev_id_array[index] = stats_req->pdev_id[index];

	for (index = 0; index < num_vdev_ids; index++)
		vdev_id_array[index] = stats_req->vdev_id[index];

	for (index = 0; index < num_mac_addr_list; index++) {
		qdf_mem_copy(mac_addr_array, stats_req->peer_mac_addr[index],
			     QDF_MAC_ADDR_SIZE);
		mac_addr_array += QDF_MAC_ADDR_SIZE;
	}

	dialog_id_array[0] = get_stats_req_twt_dialog_id(stats_req);

	wmi_debug("stats_id_mask 0x%x action 0x%x dialog_id %d",
		  cmd_fixed_param->stats_id_mask, cmd_fixed_param->action,
		  dialog_id_array[0]);
	wmi_debug("num_pdev_ids %d num_vdev_ids %d num_dialog_ids %d \
		   num_mac_addr %d", num_pdev_ids, num_vdev_ids,
		   num_dialog_ids, num_mac_addr_list);

	return req_buf;
}

/**
 * send_infra_cp_stats_request_cmd_tlv() - Prepare and send infra_cp_stats
 * wmi cmd to firmware
 * @wmi_handle: wmi handle
 * @param: Pointer to request structure
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error codes
 * on failure
 */
static QDF_STATUS
send_infra_cp_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				    struct infra_cp_stats_cmd_info *param)
{
	uint32_t len;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = prepare_infra_cp_stats_buf(wmi_handle, param, &len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	wmi_debug("buf_len %d", len);

	wmi_mtrace(WMI_REQUEST_CTRL_PATH_STATS_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_REQUEST_CTRL_PATH_STATS_CMDID);

	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
send_infra_cp_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				    struct infra_cp_stats_cmd_info *param)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef QCA_WIFI_EMULATION
static QDF_STATUS
send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
			   uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			   struct stats_request_params *param)
{
	return QDF_STATUS_SUCCESS;
}
#else
/**
 * send_stats_request_cmd_tlv() - WMI request stats function
 * @param wmi_handle: handle to WMI.
 * @param macaddr: MAC address
 * @param param: pointer to hold stats request parameter
 *
 * Return: 0  on success and -ve on failure.
 */
static QDF_STATUS
send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
			   uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			   struct stats_request_params *param)
{
	int32_t ret;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = param->stats_id;
	cmd->vdev_id = param->vdev_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							param->pdev_id);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	wmi_debug("STATS REQ STATS_ID:%d VDEV_ID:%d PDEV_ID:%d-->",
		 cmd->stats_id, cmd->vdev_id, cmd->pdev_id);

	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send_pm_chk(wmi_handle, buf, len,
					  WMI_REQUEST_STATS_CMDID);

	if (ret) {
		wmi_err("Failed to send stats request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return qdf_status_from_os_return(ret);
}
#endif

#ifdef WLAN_FEATURE_BIG_DATA_STATS
/**
 * send_big_data_stats_request_cmd_tlv () - send big data stats cmd
 * @wmi_handle: wmi handle
 * @param : pointer to command request param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_big_data_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				    struct stats_request_params *param)
{
	int32_t ret = 0;
	wmi_vdev_get_big_data_p2_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_vdev_get_big_data_p2_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_vdev_get_big_data_p2_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(
		&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_get_big_data_p2_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_vdev_get_big_data_p2_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	wmi_debug("STATS VDEV_ID:%d -->", cmd->vdev_id);

	wmi_mtrace(WMI_VDEV_GET_BIG_DATA_P2_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_GET_BIG_DATA_P2_CMDID);

	if (ret) {
		wmi_err("Failed to send big data stats request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return qdf_status_from_os_return(ret);
}
#endif

/**
 * extract_all_stats_counts_tlv() - extract all stats count from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_all_stats_counts_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     wmi_host_stats_event *stats_param)
{
	wmi_stats_event_fixed_param *ev;
	wmi_per_chain_rssi_stats *rssi_event;
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	uint64_t min_data_len;
	uint32_t i;

	qdf_mem_zero(stats_param, sizeof(*stats_param));
	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	rssi_event = param_buf->chain_stats;
	if (!ev) {
		wmi_err("event fixed param NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (param_buf->num_data > WMI_SVC_MSG_MAX_SIZE - sizeof(*ev)) {
		wmi_err("num_data : %u is invalid", param_buf->num_data);
		return QDF_STATUS_E_FAULT;
	}

	for (i = 1; i <= WMI_REQUEST_VDEV_EXTD_STAT; i = i << 1) {
		switch (ev->stats_id & i) {
		case WMI_REQUEST_PEER_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_PEER_STAT;
			break;

		case WMI_REQUEST_AP_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_AP_STAT;
			break;

		case WMI_REQUEST_PDEV_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_PDEV_STAT;
			break;

		case WMI_REQUEST_VDEV_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_VDEV_STAT;
			break;

		case WMI_REQUEST_BCNFLT_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_BCNFLT_STAT;
			break;

		case WMI_REQUEST_VDEV_RATE_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_VDEV_RATE_STAT;
			break;

		case WMI_REQUEST_BCN_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_BCN_STAT;
			break;
		case WMI_REQUEST_PEER_EXTD_STAT:
			stats_param->stats_id |= WMI_REQUEST_PEER_EXTD_STAT;
			break;

		case WMI_REQUEST_PEER_EXTD2_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_PEER_ADV_STATS;
			break;

		case WMI_REQUEST_PMF_BCN_PROTECT_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_PMF_BCN_PROTECT_STAT;
			break;

		case WMI_REQUEST_VDEV_EXTD_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_VDEV_PRB_FILS_STAT;
			break;
		}
	}

	/* ev->num_*_stats may cause uint32_t overflow, so use uint64_t
	 * to save total length calculated
	 */
	min_data_len =
		(((uint64_t)ev->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
		(((uint64_t)ev->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
		(((uint64_t)ev->num_peer_stats) * sizeof(wmi_peer_stats)) +
		(((uint64_t)ev->num_bcnflt_stats) *
		 sizeof(wmi_bcnfilter_stats_t)) +
		(((uint64_t)ev->num_chan_stats) * sizeof(wmi_chan_stats)) +
		(((uint64_t)ev->num_mib_stats) * sizeof(wmi_mib_stats)) +
		(((uint64_t)ev->num_bcn_stats) * sizeof(wmi_bcn_stats)) +
		(((uint64_t)ev->num_peer_extd_stats) *
		 sizeof(wmi_peer_extd_stats)) +
		(((uint64_t)ev->num_mib_extd_stats) *
		 sizeof(wmi_mib_extd_stats));
	if (param_buf->num_data != min_data_len) {
		wmi_err("data len: %u isn't same as calculated: %llu",
			 param_buf->num_data, min_data_len);
		return QDF_STATUS_E_FAULT;
	}

	stats_param->last_event = ev->last_event;
	stats_param->num_pdev_stats = ev->num_pdev_stats;
	stats_param->num_pdev_ext_stats = 0;
	stats_param->num_vdev_stats = ev->num_vdev_stats;
	stats_param->num_peer_stats = ev->num_peer_stats;
	stats_param->num_peer_extd_stats = ev->num_peer_extd_stats;
	stats_param->num_bcnflt_stats = ev->num_bcnflt_stats;
	stats_param->num_chan_stats = ev->num_chan_stats;
	stats_param->num_mib_stats = ev->num_mib_stats;
	stats_param->num_mib_extd_stats = ev->num_mib_extd_stats;
	stats_param->num_bcn_stats = ev->num_bcn_stats;
	stats_param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev->pdev_id);

	/* if chain_stats is not populated */
	if (!param_buf->chain_stats || !param_buf->num_chain_stats)
		return QDF_STATUS_SUCCESS;

	if (WMITLV_TAG_STRUC_wmi_per_chain_rssi_stats !=
	    WMITLV_GET_TLVTAG(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (WMITLV_GET_STRUCT_TLVLEN(wmi_per_chain_rssi_stats) !=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (rssi_event->num_per_chain_rssi_stats >=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header)) {
		wmi_err("num_per_chain_rssi_stats:%u is out of bounds",
			 rssi_event->num_per_chain_rssi_stats);
		return QDF_STATUS_E_INVAL;
	}
	stats_param->num_rssi_stats = rssi_event->num_per_chain_rssi_stats;

	if (param_buf->vdev_extd_stats)
		stats_param->num_vdev_extd_stats =
			param_buf->num_vdev_extd_stats;

	/* if peer_adv_stats is not populated */
	if (param_buf->num_peer_extd2_stats)
		stats_param->num_peer_adv_stats =
			param_buf->num_peer_extd2_stats;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tx_stats() - extract pdev tx stats from event
 */
static void extract_pdev_tx_stats(wmi_host_dbg_tx_stats *tx,
				  struct wlan_dbg_tx_stats *tx_stats)
{
	/* Tx Stats */
	tx->comp_queued = tx_stats->comp_queued;
	tx->comp_delivered = tx_stats->comp_delivered;
	tx->msdu_enqued = tx_stats->msdu_enqued;
	tx->mpdu_enqued = tx_stats->mpdu_enqued;
	tx->wmm_drop = tx_stats->wmm_drop;
	tx->local_enqued = tx_stats->local_enqued;
	tx->local_freed = tx_stats->local_freed;
	tx->hw_queued = tx_stats->hw_queued;
	tx->hw_reaped = tx_stats->hw_reaped;
	tx->underrun = tx_stats->underrun;
	tx->tx_abort = tx_stats->tx_abort;
	tx->mpdus_requed = tx_stats->mpdus_requed;
	tx->data_rc = tx_stats->data_rc;
	tx->self_triggers = tx_stats->self_triggers;
	tx->sw_retry_failure = tx_stats->sw_retry_failure;
	tx->illgl_rate_phy_err = tx_stats->illgl_rate_phy_err;
	tx->pdev_cont_xretry = tx_stats->pdev_cont_xretry;
	tx->pdev_tx_timeout = tx_stats->pdev_tx_timeout;
	tx->pdev_resets = tx_stats->pdev_resets;
	tx->stateless_tid_alloc_failure = tx_stats->stateless_tid_alloc_failure;
	tx->phy_underrun = tx_stats->phy_underrun;
	tx->txop_ovf = tx_stats->txop_ovf;

	return;
}


/**
 * extract_pdev_rx_stats() - extract pdev rx stats from event
 */
static void extract_pdev_rx_stats(wmi_host_dbg_rx_stats *rx,
				  struct wlan_dbg_rx_stats *rx_stats)
{
	/* Rx Stats */
	rx->mid_ppdu_route_change = rx_stats->mid_ppdu_route_change;
	rx->status_rcvd = rx_stats->status_rcvd;
	rx->r0_frags = rx_stats->r0_frags;
	rx->r1_frags = rx_stats->r1_frags;
	rx->r2_frags = rx_stats->r2_frags;
	/* Only TLV */
	rx->r3_frags = 0;
	rx->htt_msdus = rx_stats->htt_msdus;
	rx->htt_mpdus = rx_stats->htt_mpdus;
	rx->loc_msdus = rx_stats->loc_msdus;
	rx->loc_mpdus = rx_stats->loc_mpdus;
	rx->oversize_amsdu = rx_stats->oversize_amsdu;
	rx->phy_errs = rx_stats->phy_errs;
	rx->phy_err_drop = rx_stats->phy_err_drop;
	rx->mpdu_errs = rx_stats->mpdu_errs;

	return;
}

/**
 * extract_pdev_stats_tlv() - extract pdev stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_pdev_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf, uint32_t index,
		       wmi_host_pdev_stats *pdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	pdev_stats->pdev_id =
	     wmi_handle->ops->convert_pdev_id_target_to_host(wmi_handle,
							     ev_param->pdev_id);

	data = param_buf->data;

	if (index < ev_param->num_pdev_stats) {
		wmi_pdev_stats *ev = (wmi_pdev_stats *) ((data) +
				(index * sizeof(wmi_pdev_stats)));

		pdev_stats->chan_nf = ev->chan_nf;
		pdev_stats->tx_frame_count = ev->tx_frame_count;
		pdev_stats->rx_frame_count = ev->rx_frame_count;
		pdev_stats->rx_clear_count = ev->rx_clear_count;
		pdev_stats->cycle_count = ev->cycle_count;
		pdev_stats->phy_err_count = ev->phy_err_count;
		pdev_stats->chan_tx_pwr = ev->chan_tx_pwr;

		extract_pdev_tx_stats(&(pdev_stats->pdev_stats.tx),
			&(ev->pdev_stats.tx));
		extract_pdev_rx_stats(&(pdev_stats->pdev_stats.rx),
			&(ev->pdev_stats.rx));
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stats_tlv() - extract vdev stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_vdev_stats) {
		wmi_vdev_stats *ev = (wmi_vdev_stats *) ((data) +
				((ev_param->num_pdev_stats) *
				sizeof(wmi_pdev_stats)) +
				(index * sizeof(wmi_vdev_stats)));

		vdev_stats->vdev_id = ev->vdev_id;
		vdev_stats->vdev_snr.bcn_snr = ev->vdev_snr.bcn_snr;
		vdev_stats->vdev_snr.dat_snr = ev->vdev_snr.dat_snr;

		OS_MEMCPY(vdev_stats->tx_frm_cnt, ev->tx_frm_cnt,
			sizeof(ev->tx_frm_cnt));
		vdev_stats->rx_frm_cnt = ev->rx_frm_cnt;
		OS_MEMCPY(vdev_stats->multiple_retry_cnt,
				ev->multiple_retry_cnt,
				sizeof(ev->multiple_retry_cnt));
		OS_MEMCPY(vdev_stats->fail_cnt, ev->fail_cnt,
				sizeof(ev->fail_cnt));
		vdev_stats->rts_fail_cnt = ev->rts_fail_cnt;
		vdev_stats->rts_succ_cnt = ev->rts_succ_cnt;
		vdev_stats->rx_err_cnt = ev->rx_err_cnt;
		vdev_stats->rx_discard_cnt = ev->rx_discard_cnt;
		vdev_stats->ack_fail_cnt = ev->ack_fail_cnt;
		OS_MEMCPY(vdev_stats->tx_rate_history, ev->tx_rate_history,
			sizeof(ev->tx_rate_history));
		OS_MEMCPY(vdev_stats->bcn_rssi_history, ev->bcn_rssi_history,
			sizeof(ev->bcn_rssi_history));

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_stats_tlv() - extract peer stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_peer_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf, uint32_t index,
		       wmi_host_peer_stats *peer_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_peer_stats) {
		wmi_peer_stats *ev = (wmi_peer_stats *) ((data) +
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			(index * sizeof(wmi_peer_stats)));

		OS_MEMSET(peer_stats, 0, sizeof(wmi_host_peer_stats));

		OS_MEMCPY(&(peer_stats->peer_macaddr),
			&(ev->peer_macaddr), sizeof(wmi_mac_addr));

		peer_stats->peer_rssi = ev->peer_rssi;
		peer_stats->peer_tx_rate = ev->peer_tx_rate;
		peer_stats->peer_rx_rate = ev->peer_rx_rate;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_extd_stats_tlv() - extract extended peer stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_peer_extd_stats_tlv(wmi_unified_t wmi_handle,
			    void *evt_buf, uint32_t index,
			    wmi_host_peer_extd_stats *peer_extd_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	ev_param = (wmi_stats_event_fixed_param *)param_buf->fixed_param;
	data = (uint8_t *)param_buf->data;
	if (!data)
		return QDF_STATUS_E_FAILURE;

	if (index < ev_param->num_peer_extd_stats) {
		wmi_peer_extd_stats *ev = (wmi_peer_extd_stats *) (data +
			(ev_param->num_pdev_stats * sizeof(wmi_pdev_stats)) +
			(ev_param->num_vdev_stats * sizeof(wmi_vdev_stats)) +
			(ev_param->num_peer_stats * sizeof(wmi_peer_stats)) +
			(ev_param->num_bcnflt_stats *
			sizeof(wmi_bcnfilter_stats_t)) +
			(ev_param->num_chan_stats * sizeof(wmi_chan_stats)) +
			(ev_param->num_mib_stats * sizeof(wmi_mib_stats)) +
			(ev_param->num_bcn_stats * sizeof(wmi_bcn_stats)) +
			(index * sizeof(wmi_peer_extd_stats)));

		qdf_mem_zero(peer_extd_stats, sizeof(wmi_host_peer_extd_stats));
		qdf_mem_copy(&peer_extd_stats->peer_macaddr, &ev->peer_macaddr,
			     sizeof(wmi_mac_addr));

		peer_extd_stats->rx_mc_bc_cnt = ev->rx_mc_bc_cnt;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * extract_pmf_bcn_protect_stats_tlv() - extract pmf bcn stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @pmf_bcn_stats: Pointer to hold pmf bcn protect stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */

static QDF_STATUS
extract_pmf_bcn_protect_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				  wmi_host_pmf_bcn_protect_stats *pmf_bcn_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_FAILURE;

	ev_param = (wmi_stats_event_fixed_param *)param_buf->fixed_param;

	if ((ev_param->stats_id & WMI_REQUEST_PMF_BCN_PROTECT_STAT) &&
	    param_buf->pmf_bcn_protect_stats) {
		pmf_bcn_stats->igtk_mic_fail_cnt =
			param_buf->pmf_bcn_protect_stats->igtk_mic_fail_cnt;
		pmf_bcn_stats->igtk_replay_cnt =
			param_buf->pmf_bcn_protect_stats->igtk_replay_cnt;
		pmf_bcn_stats->bcn_mic_fail_cnt =
			param_buf->pmf_bcn_protect_stats->bcn_mic_fail_cnt;
		pmf_bcn_stats->bcn_replay_cnt =
			param_buf->pmf_bcn_protect_stats->bcn_replay_cnt;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
static void wmi_infra_cp_stats_ops_attach_tlv(struct wmi_ops *ops)
{
	ops->send_infra_cp_stats_request_cmd =
					send_infra_cp_stats_request_cmd_tlv;
}
#else
static void wmi_infra_cp_stats_ops_attach_tlv(struct wmi_ops *ops)
{
}
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */

void wmi_cp_stats_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_stats_request_cmd = send_stats_request_cmd_tlv;
#ifdef WLAN_FEATURE_BIG_DATA_STATS
	ops->send_big_data_stats_request_cmd =
				send_big_data_stats_request_cmd_tlv;
#endif
	ops->extract_all_stats_count = extract_all_stats_counts_tlv;
	ops->extract_pdev_stats = extract_pdev_stats_tlv;
	ops->extract_vdev_stats = extract_vdev_stats_tlv;
	ops->extract_peer_stats = extract_peer_stats_tlv;
	ops->extract_peer_extd_stats = extract_peer_extd_stats_tlv;
	wmi_infra_cp_stats_ops_attach_tlv(ops);
	ops->extract_pmf_bcn_protect_stats = extract_pmf_bcn_protect_stats_tlv,

	wmi_mc_cp_stats_attach_tlv(wmi_handle);
}

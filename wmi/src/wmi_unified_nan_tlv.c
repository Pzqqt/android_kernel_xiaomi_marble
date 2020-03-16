
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
#include <nan_public_structs.h>
#include <wmi_unified_nan_api.h>
#include <wlan_nan_msg_common_v2.h>

static QDF_STATUS
extract_nan_event_rsp_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			  struct nan_event_params *evt_params,
			  uint8_t **msg_buf)
{
	WMI_NAN_EVENTID_param_tlvs *event;
	wmi_nan_event_hdr *nan_rsp_event_hdr;
	nan_msg_header_t *nan_msg_hdr;
	wmi_nan_event_info *nan_evt_info;

	/*
	 * This is how received evt looks like
	 *
	 * <-------------------- evt_buf ----------------------------------->
	 *
	 * <--wmi_nan_event_hdr--><---WMI_TLV_HDR_SIZE---><----- data -------->
	 *
	 * +-----------+---------+-----------------------+--------------------+-
	 * | tlv_header| data_len| WMITLV_TAG_ARRAY_BYTE | nan_rsp_event_data |
	 * +-----------+---------+-----------------------+--------------------+-
	 *
	 * (Only for NAN Enable Resp)
	 * <--wmi_nan_event_info-->
	 * +-----------+-----------+
	 * | tlv_header| event_info|
	 * +-----------+-----------+
	 *
	 */

	event = (WMI_NAN_EVENTID_param_tlvs *)evt_buf;
	nan_rsp_event_hdr = event->fixed_param;

	/* Actual data may include some padding, so data_len <= num_data */
	if (nan_rsp_event_hdr->data_len > event->num_data) {
		WMI_LOGE("%s: Provided NAN event length(%d) exceeding actual length(%d)!",
			 __func__, nan_rsp_event_hdr->data_len,
			 event->num_data);
		return QDF_STATUS_E_INVAL;
	}
	evt_params->buf_len = nan_rsp_event_hdr->data_len;
	*msg_buf = event->data;

	if (nan_rsp_event_hdr->data_len < sizeof(nan_msg_header_t) ||
	    nan_rsp_event_hdr->data_len > (WMI_SVC_MSG_MAX_SIZE -
							    WMI_TLV_HDR_SIZE)) {
		WMI_LOGE("%s: Invalid NAN event data length(%d)!",  __func__,
			 nan_rsp_event_hdr->data_len);
		return QDF_STATUS_E_INVAL;
	}
	nan_msg_hdr = (nan_msg_header_t *)event->data;

	if (!wmi_service_enabled(wmi_handle, wmi_service_nan_dbs_support)) {
		evt_params->evt_type = nan_event_id_generic_rsp;
		return QDF_STATUS_SUCCESS;
	}

	switch (nan_msg_hdr->msg_id) {
	case NAN_MSG_ID_ENABLE_RSP:
		nan_evt_info = event->event_info;
		if (!nan_evt_info) {
			WMI_LOGE(FL("Fail: NAN enable rsp event info Null"));
			return QDF_STATUS_E_INVAL;
		}
		evt_params->evt_type = nan_event_id_enable_rsp;
		evt_params->mac_id = nan_evt_info->mac_id;
		evt_params->is_nan_enable_success = (nan_evt_info->status == 0);
		evt_params->vdev_id = nan_evt_info->vdev_id;
		break;
	case NAN_MSG_ID_DISABLE_IND:
		evt_params->evt_type = nan_event_id_disable_ind;
		break;
	case NAN_MSG_ID_ERROR_RSP:
		evt_params->evt_type = nan_event_id_error_rsp;
		break;
	default:
		evt_params->evt_type = nan_event_id_generic_rsp;
		break;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_nan_disable_req_cmd_tlv() - to send nan disable request to target
 * @wmi_handle: wmi handle
 * @nan_msg: request data which will be non-null
 *
 * Return: CDF status
 */
static QDF_STATUS send_nan_disable_req_cmd_tlv(wmi_unified_t wmi_handle,
					       struct nan_disable_req *nan_msg)
{
	QDF_STATUS ret;
	wmi_nan_cmd_param *cmd;
	wmi_nan_host_config_param *cfg;
	wmi_buf_t buf;
	/* Initialize with minimum length required, which is Scenario 2*/
	uint16_t len = sizeof(*cmd) + sizeof(*cfg) + 2 * WMI_TLV_HDR_SIZE;
	uint16_t nan_data_len, nan_data_len_aligned = 0;
	uint8_t *buf_ptr;

	/*
	 *  Scenario 1: NAN Disable with NAN msg data from upper layers
	 *
	 *    <-----nan cmd param-----><-- WMI_TLV_HDR_SIZE --><--- data ---->
	 *    +------------+----------+-----------------------+--------------+
	 *    | tlv_header | data_len | WMITLV_TAG_ARRAY_BYTE | nan_msg_data |
	 *    +------------+----------+-----------------------+--------------+
	 *
	 *    <-- WMI_TLV_HDR_SIZE --><------nan host config params----->
	 *   -+-----------------------+---------------------------------+
	 *    | WMITLV_TAG_ARRAY_STRUC| tlv_header | 2g/5g disable flags|
	 *   -+-----------------------+---------------------------------+
	 *
	 * Scenario 2: NAN Disable without any NAN msg data from upper layers
	 *
	 *    <------nan cmd param------><--WMI_TLV_HDR_SIZE--><--WMI_TLV_HDR_SI
	 *    +------------+------------+----------------------+----------------
	 *    | tlv_header | data_len=0 | WMITLV_TAG_ARRAY_BYTE| WMITLV_TAG_ARRA
	 *    +------------+------------+----------------------+----------------
	 *
	 *    ZE----><------nan host config params----->
	 *    -------+---------------------------------+
	 *    Y_STRUC| tlv_header | 2g/5g disable flags|
	 *    -------+---------------------------------+
	 */

	if (!nan_msg) {
		WMI_LOGE("%s:nan req is not valid", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	nan_data_len = nan_msg->params.request_data_len;

	if (nan_data_len) {
		nan_data_len_aligned = roundup(nan_data_len, sizeof(uint32_t));
		if (nan_data_len_aligned < nan_data_len) {
			WMI_LOGE("%s: Int overflow while rounding up data_len",
				 __func__);
			return QDF_STATUS_E_FAILURE;
		}

		if (nan_data_len_aligned > WMI_SVC_MSG_MAX_SIZE
							- WMI_TLV_HDR_SIZE) {
			WMI_LOGE("%s: nan_data_len exceeding wmi_max_msg_size",
				 __func__);
			return QDF_STATUS_E_FAILURE;
		}

		len += nan_data_len_aligned;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_nan_cmd_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nan_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_nan_cmd_param));

	cmd->data_len = nan_data_len;
	WMI_LOGD("%s: nan data len value is %u", __func__, nan_data_len);
	buf_ptr += sizeof(wmi_nan_cmd_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, nan_data_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;

	if (nan_data_len) {
		qdf_mem_copy(buf_ptr, nan_msg->params.request_data,
			     cmd->data_len);
		buf_ptr += nan_data_len_aligned;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_nan_host_config_param));
	buf_ptr += WMI_TLV_HDR_SIZE;

	cfg = (wmi_nan_host_config_param *)buf_ptr;
	WMITLV_SET_HDR(&cfg->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nan_host_config_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_nan_host_config_param));
	cfg->nan_2g_disc_disable = nan_msg->disable_2g_discovery;
	cfg->nan_5g_disc_disable = nan_msg->disable_5g_discovery;

	wmi_mtrace(WMI_NAN_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NAN_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s Failed to send set param command ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_nan_req_cmd_tlv() - to send nan request to target
 * @wmi_handle: wmi handle
 * @nan_msg: request data which will be non-null
 *
 * Return: CDF status
 */
static QDF_STATUS send_nan_req_cmd_tlv(wmi_unified_t wmi_handle,
				       struct nan_msg_params *nan_msg)
{
	QDF_STATUS ret;
	wmi_nan_cmd_param *cmd;
	wmi_buf_t buf;
	wmi_nan_host_config_param *cfg;
	uint16_t len = sizeof(*cmd) + sizeof(*cfg) + 2 * WMI_TLV_HDR_SIZE;
	uint16_t nan_data_len, nan_data_len_aligned;
	uint8_t *buf_ptr;

	/*
	 *    <----- cmd ------------><-- WMI_TLV_HDR_SIZE --><--- data ---->
	 *    +------------+----------+-----------------------+--------------+
	 *    | tlv_header | data_len | WMITLV_TAG_ARRAY_BYTE | nan_msg_data |
	 *    +------------+----------+-----------------------+--------------+
	 *
	 *    <-- WMI_TLV_HDR_SIZE --><------nan host config params-------->
	 *    +-----------------------+------------------------------------+
	 *    | WMITLV_TAG_ARRAY_STRUC| tlv_header | disable flags | flags |
	 *    +-----------------------+------------------------------------+
	 */
	if (!nan_msg) {
		WMI_LOGE("%s:nan req is not valid", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	nan_data_len = nan_msg->request_data_len;
	nan_data_len_aligned = roundup(nan_msg->request_data_len,
				       sizeof(uint32_t));
	if (nan_data_len_aligned < nan_msg->request_data_len) {
		WMI_LOGE("%s: integer overflow while rounding up data_len",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (nan_data_len_aligned > WMI_SVC_MSG_MAX_SIZE - WMI_TLV_HDR_SIZE) {
		WMI_LOGE("%s: wmi_max_msg_size overflow for given datalen",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}

	len += nan_data_len_aligned;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_nan_cmd_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nan_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_nan_cmd_param));
	cmd->data_len = nan_msg->request_data_len;
	buf_ptr += sizeof(wmi_nan_cmd_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, nan_data_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, nan_msg->request_data, cmd->data_len);
	buf_ptr += nan_data_len_aligned;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_nan_host_config_param));
	buf_ptr += WMI_TLV_HDR_SIZE;

	cfg = (wmi_nan_host_config_param *)buf_ptr;
	WMITLV_SET_HDR(&cfg->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nan_host_config_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_nan_host_config_param));

	WMI_NAN_SET_RANGING_INITIATOR_ROLE(cfg->flags, !!(nan_msg->rtt_cap &
					   WMI_FW_NAN_RTT_INITR));
	WMI_NAN_SET_RANGING_RESPONDER_ROLE(cfg->flags, !!(nan_msg->rtt_cap &
					   WMI_FW_NAN_RTT_RESPR));

	wmi_mtrace(WMI_NAN_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_NAN_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s Failed to send NAN req command ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_terminate_all_ndps_cmd_tlv() - send NDP Terminate for all NDP's
 * associated with the given vdev id
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
static QDF_STATUS send_terminate_all_ndps_cmd_tlv(wmi_unified_t wmi_handle,
						  uint32_t vdev_id)
{
	wmi_ndp_cmd_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	QDF_STATUS status;

	WMI_LOGD(FL("Enter"));

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_ndp_cmd_param *)wmi_buf_data(wmi_buf);

	WMITLV_SET_HDR(&cmd->tlv_header, WMITLV_TAG_STRUC_wmi_ndp_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_ndp_cmd_param));

	cmd->vdev_id = vdev_id;
	cmd->ndp_disable = 1;

	wmi_mtrace(WMI_NDP_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, wmi_buf, len, WMI_NDP_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send NDP Terminate cmd: %d", status);
		wmi_buf_free(wmi_buf);
	}

	return status;
}

static QDF_STATUS nan_ndp_initiator_req_tlv(wmi_unified_t wmi_handle,
				struct nan_datapath_initiator_req *ndp_req)
{
	uint16_t len;
	wmi_buf_t buf;
	uint8_t *tlv_ptr;
	QDF_STATUS status;
	wmi_channel *ch_tlv;
	wmi_ndp_initiator_req_fixed_param *cmd;
	uint32_t passphrase_len, service_name_len;
	uint32_t ndp_cfg_len, ndp_app_info_len, pmk_len;
	wmi_ndp_transport_ip_param *tcp_ip_param;

	/*
	 * WMI command expects 4 byte alligned len:
	 * round up ndp_cfg_len and ndp_app_info_len to 4 bytes
	 */
	ndp_cfg_len = qdf_roundup(ndp_req->ndp_config.ndp_cfg_len, 4);
	ndp_app_info_len = qdf_roundup(ndp_req->ndp_info.ndp_app_info_len, 4);
	pmk_len = qdf_roundup(ndp_req->pmk.pmk_len, 4);
	passphrase_len = qdf_roundup(ndp_req->passphrase.passphrase_len, 4);
	service_name_len =
		   qdf_roundup(ndp_req->service_name.service_name_len, 4);
	/* allocated memory for fixed params as well as variable size data */
	len = sizeof(*cmd) + sizeof(*ch_tlv) + (5 * WMI_TLV_HDR_SIZE)
		+ ndp_cfg_len + ndp_app_info_len + pmk_len
		+ passphrase_len + service_name_len;

	if (ndp_req->is_ipv6_addr_present)
		len += sizeof(*tcp_ip_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_ndp_initiator_req_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ndp_initiator_req_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_ndp_initiator_req_fixed_param));
	cmd->vdev_id = wlan_vdev_get_id(ndp_req->vdev);
	cmd->transaction_id = ndp_req->transaction_id;
	cmd->service_instance_id = ndp_req->service_instance_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ndp_req->peer_discovery_mac_addr.bytes,
				   &cmd->peer_discovery_mac_addr);

	cmd->ndp_cfg_len = ndp_req->ndp_config.ndp_cfg_len;
	cmd->ndp_app_info_len = ndp_req->ndp_info.ndp_app_info_len;
	cmd->ndp_channel_cfg = ndp_req->channel_cfg;
	cmd->nan_pmk_len = ndp_req->pmk.pmk_len;
	cmd->nan_csid = ndp_req->ncs_sk_type;
	cmd->nan_passphrase_len = ndp_req->passphrase.passphrase_len;
	cmd->nan_servicename_len = ndp_req->service_name.service_name_len;

	ch_tlv = (wmi_channel *)&cmd[1];
	WMITLV_SET_HDR(ch_tlv, WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	ch_tlv->mhz = ndp_req->channel;
	tlv_ptr = (uint8_t *)&ch_tlv[1];

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_cfg_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     ndp_req->ndp_config.ndp_cfg, cmd->ndp_cfg_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_cfg_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_app_info_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     ndp_req->ndp_info.ndp_app_info, cmd->ndp_app_info_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_app_info_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, pmk_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE], ndp_req->pmk.pmk,
		     cmd->nan_pmk_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + pmk_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, passphrase_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE], ndp_req->passphrase.passphrase,
		     cmd->nan_passphrase_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + passphrase_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, service_name_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     ndp_req->service_name.service_name,
		     cmd->nan_servicename_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + service_name_len;

	if (ndp_req->is_ipv6_addr_present) {
		tcp_ip_param = (wmi_ndp_transport_ip_param *)tlv_ptr;
		WMITLV_SET_HDR(tcp_ip_param,
			       WMITLV_TAG_STRUC_wmi_ndp_transport_ip_param,
			       WMITLV_GET_STRUCT_TLVLEN(
						wmi_ndp_transport_ip_param));
		tcp_ip_param->ipv6_addr_present = true;
		qdf_mem_copy(tcp_ip_param->ipv6_intf_addr,
			     ndp_req->ipv6_addr, WMI_NDP_IPV6_INTF_ADDR_LEN);
	}
	WMI_LOGD("IPv6 addr present: %d, addr: %pI6",
		 ndp_req->is_ipv6_addr_present, ndp_req->ipv6_addr);

	WMI_LOGD("vdev_id = %d, transaction_id: %d, service_instance_id: %d, ch: %d, ch_cfg: %d, csid: %d peer mac addr: mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
		 cmd->vdev_id, cmd->transaction_id, cmd->service_instance_id,
		 ch_tlv->mhz, cmd->ndp_channel_cfg, cmd->nan_csid,
		 cmd->peer_discovery_mac_addr.mac_addr31to0,
		 cmd->peer_discovery_mac_addr.mac_addr47to32);

	WMI_LOGD("ndp_config len: %d ndp_app_info len: %d pmk len: %d pass phrase len: %d service name len: %d",
		 cmd->ndp_cfg_len, cmd->ndp_app_info_len, cmd->nan_pmk_len,
		 cmd->nan_passphrase_len, cmd->nan_servicename_len);

	wmi_mtrace(WMI_NDP_INITIATOR_REQ_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_NDP_INITIATOR_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_NDP_INITIATOR_REQ_CMDID failed, ret: %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS nan_ndp_responder_req_tlv(wmi_unified_t wmi_handle,
					struct nan_datapath_responder_req *req)
{
	uint16_t len;
	wmi_buf_t buf;
	uint8_t *tlv_ptr;
	QDF_STATUS status;
	wmi_ndp_responder_req_fixed_param *cmd;
	wmi_ndp_transport_ip_param *tcp_ip_param;
	uint32_t passphrase_len, service_name_len;
	uint32_t vdev_id = 0, ndp_cfg_len, ndp_app_info_len, pmk_len;

	vdev_id = wlan_vdev_get_id(req->vdev);
	WMI_LOGD("vdev_id: %d, transaction_id: %d, ndp_rsp %d, ndp_instance_id: %d, ndp_app_info_len: %d",
		 vdev_id, req->transaction_id,
		 req->ndp_rsp,
		 req->ndp_instance_id,
		 req->ndp_info.ndp_app_info_len);

	/*
	 * WMI command expects 4 byte alligned len:
	 * round up ndp_cfg_len and ndp_app_info_len to 4 bytes
	 */
	ndp_cfg_len = qdf_roundup(req->ndp_config.ndp_cfg_len, 4);
	ndp_app_info_len = qdf_roundup(req->ndp_info.ndp_app_info_len, 4);
	pmk_len = qdf_roundup(req->pmk.pmk_len, 4);
	passphrase_len = qdf_roundup(req->passphrase.passphrase_len, 4);
	service_name_len =
		qdf_roundup(req->service_name.service_name_len, 4);

	/* allocated memory for fixed params as well as variable size data */
	len = sizeof(*cmd) + 5*WMI_TLV_HDR_SIZE + ndp_cfg_len + ndp_app_info_len
		+ pmk_len + passphrase_len + service_name_len;

	if (req->is_ipv6_addr_present || req->is_port_present ||
	    req->is_protocol_present)
		len += sizeof(*tcp_ip_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_ndp_responder_req_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ndp_responder_req_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_ndp_responder_req_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->transaction_id = req->transaction_id;
	cmd->ndp_instance_id = req->ndp_instance_id;
	cmd->rsp_code = req->ndp_rsp;
	cmd->ndp_cfg_len = req->ndp_config.ndp_cfg_len;
	cmd->ndp_app_info_len = req->ndp_info.ndp_app_info_len;
	cmd->nan_pmk_len = req->pmk.pmk_len;
	cmd->nan_csid = req->ncs_sk_type;
	cmd->nan_passphrase_len = req->passphrase.passphrase_len;
	cmd->nan_servicename_len = req->service_name.service_name_len;

	tlv_ptr = (uint8_t *)&cmd[1];
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_cfg_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     req->ndp_config.ndp_cfg, cmd->ndp_cfg_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_cfg_len;
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_app_info_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     req->ndp_info.ndp_app_info,
		     req->ndp_info.ndp_app_info_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_app_info_len;
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, pmk_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE], req->pmk.pmk,
		     cmd->nan_pmk_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + pmk_len;
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, passphrase_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     req->passphrase.passphrase,
		     cmd->nan_passphrase_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + passphrase_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, service_name_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     req->service_name.service_name,
		     cmd->nan_servicename_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + service_name_len;

	if (req->is_ipv6_addr_present || req->is_port_present ||
	    req->is_protocol_present) {
		tcp_ip_param = (wmi_ndp_transport_ip_param *)tlv_ptr;
		WMITLV_SET_HDR(tcp_ip_param,
			       WMITLV_TAG_STRUC_wmi_ndp_transport_ip_param,
			       WMITLV_GET_STRUCT_TLVLEN(
						wmi_ndp_transport_ip_param));
		tcp_ip_param->ipv6_addr_present = req->is_ipv6_addr_present;
		qdf_mem_copy(tcp_ip_param->ipv6_intf_addr,
			     req->ipv6_addr, WMI_NDP_IPV6_INTF_ADDR_LEN);

		tcp_ip_param->trans_port_present = req->is_port_present;
		tcp_ip_param->transport_port = req->port;

		tcp_ip_param->trans_proto_present = req->is_protocol_present;
		tcp_ip_param->transport_protocol = req->protocol;
	}

	WMI_LOGD("ndp_config len: %d ndp_app_info len: %d pmk len: %d pass phrase len: %d service name len: %d",
		 req->ndp_config.ndp_cfg_len, req->ndp_info.ndp_app_info_len,
		 cmd->nan_pmk_len, cmd->nan_passphrase_len,
		 cmd->nan_servicename_len);

	wmi_mtrace(WMI_NDP_RESPONDER_REQ_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_NDP_RESPONDER_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_NDP_RESPONDER_REQ_CMDID failed, ret: %d", status);
		wmi_buf_free(buf);
	}
	return status;
}

static QDF_STATUS nan_ndp_end_req_tlv(wmi_unified_t wmi_handle,
				      struct nan_datapath_end_req *req)
{
	uint16_t len;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint32_t ndp_end_req_len, i;
	wmi_ndp_end_req *ndp_end_req_lst;
	wmi_ndp_end_req_fixed_param *cmd;

	/* len of tlv following fixed param  */
	ndp_end_req_len = sizeof(wmi_ndp_end_req) * req->num_ndp_instances;
	/* above comes out to 4 byte alligned already, no need of padding */
	len = sizeof(*cmd) + ndp_end_req_len + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_ndp_end_req_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ndp_end_req_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_ndp_end_req_fixed_param));

	cmd->transaction_id = req->transaction_id;

	/* set tlv pointer to end of fixed param */
	WMITLV_SET_HDR((uint8_t *)&cmd[1], WMITLV_TAG_ARRAY_STRUC,
			ndp_end_req_len);

	ndp_end_req_lst = (wmi_ndp_end_req *)((uint8_t *)&cmd[1] +
						WMI_TLV_HDR_SIZE);
	for (i = 0; i < req->num_ndp_instances; i++) {
		WMITLV_SET_HDR(&ndp_end_req_lst[i],
			       WMITLV_TAG_ARRAY_FIXED_STRUC,
			       (sizeof(*ndp_end_req_lst) - WMI_TLV_HDR_SIZE));

		ndp_end_req_lst[i].ndp_instance_id = req->ndp_ids[i];
	}

	wmi_mtrace(WMI_NDP_END_REQ_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_NDP_END_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_NDP_END_REQ_CMDID failed, ret: %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

static QDF_STATUS
extract_ndp_host_event_tlv(wmi_unified_t wmi_handle, uint8_t *data,
			   struct nan_datapath_host_event *evt)
{
	WMI_NDP_EVENTID_param_tlvs *event;
	wmi_ndp_event_param *fixed_params;

	event = (WMI_NDP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	evt->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!evt->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	evt->ndp_termination_in_progress =
		       fixed_params->ndp_termination_in_progress ? true : false;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_initiator_rsp_tlv(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_initiator_rsp *rsp)
{
	WMI_NDP_INITIATOR_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_initiator_rsp_event_fixed_param  *fixed_params;

	event = (WMI_NDP_INITIATOR_RSP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	rsp->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!rsp->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	rsp->transaction_id = fixed_params->transaction_id;
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->status = fixed_params->rsp_status;
	rsp->reason = fixed_params->reason_code;

	return QDF_STATUS_SUCCESS;
}

#define MAX_NAN_MSG_LEN                 200

static QDF_STATUS extract_nan_msg_tlv(uint8_t *data,
				      struct nan_dump_msg *msg)
{
	WMI_NAN_DMESG_EVENTID_param_tlvs *event;
	wmi_nan_dmesg_event_fixed_param *fixed_params;

	event = (WMI_NAN_DMESG_EVENTID_param_tlvs *)data;
	fixed_params = (wmi_nan_dmesg_event_fixed_param *)event->fixed_param;
	if (!fixed_params->msg_len ||
	    fixed_params->msg_len > MAX_NAN_MSG_LEN ||
	    fixed_params->msg_len > event->num_msg)
		return QDF_STATUS_E_FAILURE;

	msg->data_len = fixed_params->msg_len;
	msg->msg = event->msg;

	msg->msg[fixed_params->msg_len - 1] = (uint8_t)'\0';

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_ind_tlv(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_indication_event *rsp)
{
	WMI_NDP_INDICATION_EVENTID_param_tlvs *event;
	wmi_ndp_indication_event_fixed_param *fixed_params;
	size_t total_array_len;

	event = (WMI_NDP_INDICATION_EVENTID_param_tlvs *)data;
	fixed_params =
		(wmi_ndp_indication_event_fixed_param *)event->fixed_param;

	if (fixed_params->ndp_cfg_len > event->num_ndp_cfg) {
		WMI_LOGE("FW message ndp cfg length %d larger than TLV hdr %d",
			 fixed_params->ndp_cfg_len, event->num_ndp_cfg);
		return QDF_STATUS_E_INVAL;
	}

	if (fixed_params->ndp_app_info_len > event->num_ndp_app_info) {
		WMI_LOGE("FW message ndp app info length %d more than TLV hdr %d",
			 fixed_params->ndp_app_info_len,
			 event->num_ndp_app_info);
		return QDF_STATUS_E_INVAL;
	}

	if (fixed_params->ndp_cfg_len >
		(WMI_SVC_MSG_MAX_SIZE - sizeof(*fixed_params))) {
		WMI_LOGE("%s: excess wmi buffer: ndp_cfg_len %d",
			 __func__, fixed_params->ndp_cfg_len);
		return QDF_STATUS_E_INVAL;
	}

	total_array_len = fixed_params->ndp_cfg_len +
					sizeof(*fixed_params);

	if (fixed_params->ndp_app_info_len >
		(WMI_SVC_MSG_MAX_SIZE - total_array_len)) {
		WMI_LOGE("%s: excess wmi buffer: ndp_cfg_len %d",
			 __func__, fixed_params->ndp_app_info_len);
		return QDF_STATUS_E_INVAL;
	}
	total_array_len += fixed_params->ndp_app_info_len;

	if (fixed_params->nan_scid_len >
		(WMI_SVC_MSG_MAX_SIZE - total_array_len)) {
		WMI_LOGE("%s: excess wmi buffer: ndp_cfg_len %d",
			 __func__, fixed_params->nan_scid_len);
		return QDF_STATUS_E_INVAL;
	}

	rsp->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!rsp->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}
	rsp->service_instance_id = fixed_params->service_instance_id;
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->role = fixed_params->self_ndp_role;
	rsp->policy = fixed_params->accept_policy;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				rsp->peer_mac_addr.bytes);
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_discovery_mac_addr,
				rsp->peer_discovery_mac_addr.bytes);

	WMI_LOGD("WMI_NDP_INDICATION_EVENTID(0x%X) received. vdev %d service_instance %d, ndp_instance %d, role %d, policy %d csid: %d, scid_len: %d, peer_addr: %pM, peer_disc_addr: %pM ndp_cfg - %d bytes ndp_app_info - %d bytes",
		 WMI_NDP_INDICATION_EVENTID, fixed_params->vdev_id,
		 fixed_params->service_instance_id,
		 fixed_params->ndp_instance_id, fixed_params->self_ndp_role,
		 fixed_params->accept_policy, fixed_params->nan_csid,
		 fixed_params->nan_scid_len, rsp->peer_mac_addr.bytes,
		 rsp->peer_discovery_mac_addr.bytes, fixed_params->ndp_cfg_len,
		 fixed_params->ndp_app_info_len);

	rsp->ncs_sk_type = fixed_params->nan_csid;
	if (event->ndp_cfg) {
		rsp->ndp_config.ndp_cfg_len = fixed_params->ndp_cfg_len;
		if (rsp->ndp_config.ndp_cfg_len > NDP_QOS_INFO_LEN)
			rsp->ndp_config.ndp_cfg_len = NDP_QOS_INFO_LEN;
		qdf_mem_copy(rsp->ndp_config.ndp_cfg, event->ndp_cfg,
			     rsp->ndp_config.ndp_cfg_len);
	}

	if (event->ndp_app_info) {
		rsp->ndp_info.ndp_app_info_len = fixed_params->ndp_app_info_len;
		if (rsp->ndp_info.ndp_app_info_len > NDP_APP_INFO_LEN)
			rsp->ndp_info.ndp_app_info_len = NDP_APP_INFO_LEN;
		qdf_mem_copy(rsp->ndp_info.ndp_app_info, event->ndp_app_info,
			     rsp->ndp_info.ndp_app_info_len);
	}

	if (event->ndp_scid) {
		rsp->scid.scid_len = fixed_params->nan_scid_len;
		if (rsp->scid.scid_len > NDP_SCID_BUF_LEN)
			rsp->scid.scid_len = NDP_SCID_BUF_LEN;
		qdf_mem_copy(rsp->scid.scid, event->ndp_scid,
			     rsp->scid.scid_len);
	}

	if (event->ndp_transport_ip_param &&
	    event->num_ndp_transport_ip_param) {
		if (event->ndp_transport_ip_param->ipv6_addr_present) {
			rsp->is_ipv6_addr_present = true;
			qdf_mem_copy(rsp->ipv6_addr,
				event->ndp_transport_ip_param->ipv6_intf_addr,
				WMI_NDP_IPV6_INTF_ADDR_LEN);
		}
	}
	WMI_LOGD(FL("IPv6 addr present: %d, addr: %pI6"),
		    rsp->is_ipv6_addr_present, rsp->ipv6_addr);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_confirm_tlv(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_confirm_event *rsp)
{
	uint8_t i;
	WMI_HOST_WLAN_PHY_MODE ch_mode;
	WMI_NDP_CONFIRM_EVENTID_param_tlvs *event;
	wmi_ndp_confirm_event_fixed_param *fixed_params;
	size_t total_array_len;
	bool ndi_dbs = wmi_service_enabled(wmi_handle,
					   wmi_service_ndi_dbs_support);

	event = (WMI_NDP_CONFIRM_EVENTID_param_tlvs *) data;
	fixed_params = (wmi_ndp_confirm_event_fixed_param *)event->fixed_param;
	WMI_LOGD("WMI_NDP_CONFIRM_EVENTID(0x%X) received. vdev %d, ndp_instance %d, rsp_code %d, reason_code: %d, num_active_ndps_on_peer: %d num_ch: %d",
		 WMI_NDP_CONFIRM_EVENTID, fixed_params->vdev_id,
		 fixed_params->ndp_instance_id, fixed_params->rsp_code,
		 fixed_params->reason_code,
		 fixed_params->num_active_ndps_on_peer,
		 fixed_params->num_ndp_channels);

	if (fixed_params->ndp_cfg_len > event->num_ndp_cfg) {
		WMI_LOGE("FW message ndp cfg length %d larger than TLV hdr %d",
			 fixed_params->ndp_cfg_len, event->num_ndp_cfg);
		return QDF_STATUS_E_INVAL;
	}

	if (fixed_params->ndp_app_info_len > event->num_ndp_app_info) {
		WMI_LOGE("FW message ndp app info length %d more than TLV hdr %d",
			 fixed_params->ndp_app_info_len,
			 event->num_ndp_app_info);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("ndp_cfg - %d bytes, ndp_app_info - %d bytes",
		 fixed_params->ndp_cfg_len, fixed_params->ndp_app_info_len);

	if (fixed_params->ndp_cfg_len >
			(WMI_SVC_MSG_MAX_SIZE - sizeof(*fixed_params))) {
		WMI_LOGE("%s: excess wmi buffer: ndp_cfg_len %d",
			 __func__, fixed_params->ndp_cfg_len);
		return QDF_STATUS_E_INVAL;
	}

	total_array_len = fixed_params->ndp_cfg_len +
				sizeof(*fixed_params);

	if (fixed_params->ndp_app_info_len >
		(WMI_SVC_MSG_MAX_SIZE - total_array_len)) {
		WMI_LOGE("%s: excess wmi buffer: ndp_cfg_len %d",
			 __func__, fixed_params->ndp_app_info_len);
		return QDF_STATUS_E_INVAL;
	}

	if (fixed_params->num_ndp_channels > event->num_ndp_channel_list ||
	    fixed_params->num_ndp_channels > event->num_nss_list) {
		WMI_LOGE(FL("NDP Ch count %d greater than NDP Ch TLV len(%d) or NSS TLV len(%d)"),
			 fixed_params->num_ndp_channels,
			 event->num_ndp_channel_list,
			 event->num_nss_list);
		return QDF_STATUS_E_INVAL;
	}

	if (ndi_dbs &&
	    fixed_params->num_ndp_channels > event->num_ndp_channel_info) {
		WMI_LOGE(FL("NDP Ch count %d greater than NDP Ch info(%d)"),
			 fixed_params->num_ndp_channels,
			 event->num_ndp_channel_info);
		return QDF_STATUS_E_INVAL;
	}

	rsp->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!rsp->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->rsp_code = fixed_params->rsp_code;
	rsp->reason_code = fixed_params->reason_code;
	rsp->num_active_ndps_on_peer = fixed_params->num_active_ndps_on_peer;
	rsp->num_channels = fixed_params->num_ndp_channels;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				   rsp->peer_ndi_mac_addr.bytes);
	rsp->ndp_info.ndp_app_info_len = fixed_params->ndp_app_info_len;
	qdf_mem_copy(rsp->ndp_info.ndp_app_info, event->ndp_app_info,
		     rsp->ndp_info.ndp_app_info_len);

	if (rsp->num_channels > NAN_CH_INFO_MAX_CHANNELS) {
		WMI_LOGE(FL("too many channels"));
		rsp->num_channels = NAN_CH_INFO_MAX_CHANNELS;
	}

	for (i = 0; i < rsp->num_channels; i++) {
		rsp->ch[i].freq = event->ndp_channel_list[i].mhz;
		rsp->ch[i].nss = event->nss_list[i];
		ch_mode = WMI_GET_CHANNEL_MODE(&event->ndp_channel_list[i]);
		rsp->ch[i].ch_width = wmi_get_ch_width_from_phy_mode(wmi_handle,
								     ch_mode);
		if (ndi_dbs) {
			rsp->ch[i].mac_id = event->ndp_channel_info[i].mac_id;
			WMI_LOGD("Freq: %d, ch_mode: %d, nss: %d mac_id: %d",
				 rsp->ch[i].freq, rsp->ch[i].ch_width,
				 rsp->ch[i].nss, rsp->ch[i].mac_id);
		} else {
			WMI_LOGD("Freq: %d, ch_mode: %d, nss: %d",
				 rsp->ch[i].freq, rsp->ch[i].ch_width,
				 rsp->ch[i].nss);
		}
	}

	if (event->ndp_transport_ip_param &&
	    event->num_ndp_transport_ip_param) {
		if (event->ndp_transport_ip_param->ipv6_addr_present) {
			rsp->is_ipv6_addr_present = true;
			qdf_mem_copy(rsp->ipv6_addr,
				event->ndp_transport_ip_param->ipv6_intf_addr,
				WMI_NDP_IPV6_INTF_ADDR_LEN);
		}

		if (event->ndp_transport_ip_param->trans_port_present) {
			rsp->is_port_present = true;
			rsp->port =
			    event->ndp_transport_ip_param->transport_port;
		}

		if (event->ndp_transport_ip_param->trans_proto_present) {
			rsp->is_protocol_present = true;
			rsp->protocol =
			    event->ndp_transport_ip_param->transport_protocol;
		}
	}
	WMI_LOGD("IPv6 addr present: %d, addr: %pI6 port: %d present: %d protocol: %d present: %d",
		 rsp->is_ipv6_addr_present, rsp->ipv6_addr, rsp->port,
		 rsp->is_port_present, rsp->protocol, rsp->is_protocol_present);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_responder_rsp_tlv(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_responder_rsp *rsp)
{
	WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_responder_rsp_event_fixed_param  *fixed_params;

	event = (WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	rsp->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!rsp->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}
	rsp->transaction_id = fixed_params->transaction_id;
	rsp->reason = fixed_params->reason_code;
	rsp->status = fixed_params->rsp_status;
	rsp->create_peer = fixed_params->create_peer;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				   rsp->peer_mac_addr.bytes);
	WMI_LOGD("WMI_NDP_RESPONDER_RSP_EVENTID(0x%X) received. vdev_id: %d, peer_mac_addr: %pM,transaction_id: %d, status_code %d, reason_code: %d, create_peer: %d",
		 WMI_NDP_RESPONDER_RSP_EVENTID, fixed_params->vdev_id,
		 rsp->peer_mac_addr.bytes, rsp->transaction_id,
		 rsp->status, rsp->reason, rsp->create_peer);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_end_rsp_tlv(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_end_rsp_event *rsp)
{
	WMI_NDP_END_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_end_rsp_event_fixed_param *fixed_params = NULL;

	event = (WMI_NDP_END_RSP_EVENTID_param_tlvs *) data;
	fixed_params = (wmi_ndp_end_rsp_event_fixed_param *)event->fixed_param;
	WMI_LOGD("WMI_NDP_END_RSP_EVENTID(0x%X) received. transaction_id: %d, rsp_status: %d, reason_code: %d",
		 WMI_NDP_END_RSP_EVENTID, fixed_params->transaction_id,
		 fixed_params->rsp_status, fixed_params->reason_code);

	rsp->vdev = wlan_objmgr_get_vdev_by_opmode_from_psoc(
			wmi_handle->soc->wmi_psoc, QDF_NDI_MODE, WLAN_NAN_ID);
	if (!rsp->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}
	rsp->transaction_id = fixed_params->transaction_id;
	rsp->reason = fixed_params->reason_code;
	rsp->status = fixed_params->rsp_status;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_end_ind_tlv(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_end_indication_event **rsp)
{
	uint32_t i, buf_size;
	wmi_ndp_end_indication *ind;
	struct qdf_mac_addr peer_addr;
	WMI_NDP_END_INDICATION_EVENTID_param_tlvs *event;

	event = (WMI_NDP_END_INDICATION_EVENTID_param_tlvs *) data;
	ind = event->ndp_end_indication_list;

	if (event->num_ndp_end_indication_list == 0) {
		WMI_LOGE("Error: Event ignored, 0 ndp instances");
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("number of ndp instances = %d",
		 event->num_ndp_end_indication_list);

	if (event->num_ndp_end_indication_list > ((UINT_MAX - sizeof(**rsp))/
						sizeof((*rsp)->ndp_map[0]))) {
		WMI_LOGE("num_ndp_end_ind_list %d too large",
			 event->num_ndp_end_indication_list);
		return QDF_STATUS_E_INVAL;
	}

	buf_size = sizeof(**rsp) + event->num_ndp_end_indication_list *
			sizeof((*rsp)->ndp_map[0]);
	*rsp = qdf_mem_malloc(buf_size);
	if (!(*rsp))
		return QDF_STATUS_E_NOMEM;

	(*rsp)->num_ndp_ids = event->num_ndp_end_indication_list;
	for (i = 0; i < (*rsp)->num_ndp_ids; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&ind[i].peer_ndi_mac_addr,
					   peer_addr.bytes);
		WMI_LOGD("ind[%d]: type %d, reason_code %d, instance_id %d num_active %d ",
			 i, ind[i].type, ind[i].reason_code,
			 ind[i].ndp_instance_id,
			 ind[i].num_active_ndps_on_peer);
		/* Add each instance entry to the list */
		(*rsp)->ndp_map[i].ndp_instance_id = ind[i].ndp_instance_id;
		(*rsp)->ndp_map[i].vdev_id = ind[i].vdev_id;
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&ind[i].peer_ndi_mac_addr,
			(*rsp)->ndp_map[i].peer_ndi_mac_addr.bytes);
		(*rsp)->ndp_map[i].num_active_ndp_sessions =
			ind[i].num_active_ndps_on_peer;
		(*rsp)->ndp_map[i].type = ind[i].type;
		(*rsp)->ndp_map[i].reason_code = ind[i].reason_code;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_sch_update_tlv(wmi_unified_t wmi_handle,
		uint8_t *data, struct nan_datapath_sch_update_event *ind)
{
	uint8_t i;
	WMI_HOST_WLAN_PHY_MODE ch_mode;
	WMI_NDL_SCHEDULE_UPDATE_EVENTID_param_tlvs *event;
	wmi_ndl_schedule_update_fixed_param *fixed_params;
	bool ndi_dbs = wmi_service_enabled(wmi_handle,
					   wmi_service_ndi_dbs_support);

	event = (WMI_NDL_SCHEDULE_UPDATE_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	WMI_LOGD(FL("flags: %d, num_ch: %d, num_ndp_instances: %d"),
		 fixed_params->flags, fixed_params->num_channels,
		 fixed_params->num_ndp_instances);

	if (fixed_params->num_channels > event->num_ndl_channel_list ||
	    fixed_params->num_channels > event->num_nss_list) {
		WMI_LOGE(FL("Channel count %d greater than NDP Ch list TLV len(%d) or NSS list TLV len(%d)"),
			 fixed_params->num_channels,
			 event->num_ndl_channel_list,
			 event->num_nss_list);
		return QDF_STATUS_E_INVAL;
	}

	if (ndi_dbs &&
	    fixed_params->num_channels > event->num_ndp_channel_info) {
		WMI_LOGE(FL("Channel count %d greater than NDP Ch info(%d)"),
			 fixed_params->num_channels,
			 event->num_ndp_channel_info);
		return QDF_STATUS_E_INVAL;
	}

	if (fixed_params->num_ndp_instances > event->num_ndp_instance_list) {
		WMI_LOGE(FL("NDP Instance count %d greater than NDP Instancei TLV len %d"),
			 fixed_params->num_ndp_instances,
			 event->num_ndp_instance_list);
		return QDF_STATUS_E_INVAL;
	}

	ind->vdev =
		wlan_objmgr_get_vdev_by_id_from_psoc(wmi_handle->soc->wmi_psoc,
						     fixed_params->vdev_id,
						     WLAN_NAN_ID);
	if (!ind->vdev) {
		WMI_LOGE("vdev is null");
		return QDF_STATUS_E_INVAL;
	}

	ind->flags = fixed_params->flags;
	ind->num_channels = fixed_params->num_channels;
	ind->num_ndp_instances = fixed_params->num_ndp_instances;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_macaddr,
				   ind->peer_addr.bytes);

	if (ind->num_ndp_instances > NDP_NUM_INSTANCE_ID) {
		WMI_LOGE(FL("uint32 overflow"));
		wlan_objmgr_vdev_release_ref(ind->vdev, WLAN_NAN_ID);
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(ind->ndp_instances, event->ndp_instance_list,
		     sizeof(uint32_t) * ind->num_ndp_instances);

	if (ind->num_channels > NAN_CH_INFO_MAX_CHANNELS) {
		WMI_LOGE(FL("too many channels"));
		ind->num_channels = NAN_CH_INFO_MAX_CHANNELS;
	}

	for (i = 0; i < ind->num_channels; i++) {
		ind->ch[i].freq = event->ndl_channel_list[i].mhz;
		ind->ch[i].nss = event->nss_list[i];
		ch_mode = WMI_GET_CHANNEL_MODE(&event->ndl_channel_list[i]);
		ind->ch[i].ch_width = wmi_get_ch_width_from_phy_mode(wmi_handle,
								     ch_mode);
		if (ndi_dbs) {
			ind->ch[i].mac_id = event->ndp_channel_info[i].mac_id;
			WMI_LOGD(FL("Freq: %d, ch_mode: %d, nss: %d mac_id: %d"),
				 ind->ch[i].freq, ind->ch[i].ch_width,
				 ind->ch[i].nss, ind->ch[i].mac_id);
		} else {
			WMI_LOGD(FL("Freq: %d, ch_mode: %d, nss: %d"),
				 ind->ch[i].freq, ind->ch[i].ch_width,
				 ind->ch[i].nss);
		}
	}

	for (i = 0; i < fixed_params->num_ndp_instances; i++)
		WMI_LOGD(FL("instance_id[%d]: %d"),
			 i, event->ndp_instance_list[i]);

	return QDF_STATUS_SUCCESS;
}

void wmi_nan_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_nan_req_cmd = send_nan_req_cmd_tlv;
	ops->send_nan_disable_req_cmd = send_nan_disable_req_cmd_tlv;
	ops->extract_nan_event_rsp = extract_nan_event_rsp_tlv;
	ops->send_terminate_all_ndps_req_cmd = send_terminate_all_ndps_cmd_tlv;
	ops->send_ndp_initiator_req_cmd = nan_ndp_initiator_req_tlv;
	ops->send_ndp_responder_req_cmd = nan_ndp_responder_req_tlv;
	ops->send_ndp_end_req_cmd = nan_ndp_end_req_tlv;
	ops->extract_ndp_initiator_rsp = extract_ndp_initiator_rsp_tlv;
	ops->extract_ndp_ind = extract_ndp_ind_tlv;
	ops->extract_nan_msg = extract_nan_msg_tlv,
	ops->extract_ndp_confirm = extract_ndp_confirm_tlv;
	ops->extract_ndp_responder_rsp = extract_ndp_responder_rsp_tlv;
	ops->extract_ndp_end_rsp = extract_ndp_end_rsp_tlv;
	ops->extract_ndp_end_ind = extract_ndp_end_ind_tlv;
	ops->extract_ndp_sch_update = extract_ndp_sch_update_tlv;
	ops->extract_ndp_host_event = extract_ndp_host_event_tlv;
}

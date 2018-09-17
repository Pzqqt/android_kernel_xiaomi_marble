
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

#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <nan_public_structs.h>
#include <wmi_unified_nan_api.h>

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
	WMI_LOGD(FL("IPv6 addr present: %d, addr: %pI6"),
		 ndp_req->is_ipv6_addr_present, ndp_req->ipv6_addr);

	WMI_LOGD("vdev_id = %d, transaction_id: %d, service_instance_id: %d, ch: %d, ch_cfg: %d, csid: %d",
		 cmd->vdev_id, cmd->transaction_id, cmd->service_instance_id,
		 ch_tlv->mhz, cmd->ndp_channel_cfg, cmd->nan_csid);
	WMI_LOGD("peer mac addr: mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
		 cmd->peer_discovery_mac_addr.mac_addr31to0,
		 cmd->peer_discovery_mac_addr.mac_addr47to32);

	WMI_LOGD("ndp_config len: %d", cmd->ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->ndp_config.ndp_cfg,
			   ndp_req->ndp_config.ndp_cfg_len);

	WMI_LOGD("ndp_app_info len: %d", cmd->ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->ndp_info.ndp_app_info,
			   ndp_req->ndp_info.ndp_app_info_len);

	WMI_LOGD("pmk len: %d", cmd->nan_pmk_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->pmk.pmk, cmd->nan_pmk_len);

	WMI_LOGD("pass phrase len: %d", cmd->nan_passphrase_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->passphrase.passphrase,
			   cmd->nan_passphrase_len);

	WMI_LOGD("service name len: %d", cmd->nan_servicename_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->service_name.service_name,
			   cmd->nan_servicename_len);

	WMI_LOGD("sending WMI_NDP_INITIATOR_REQ_CMDID(0x%X)",
		 WMI_NDP_INITIATOR_REQ_CMDID);

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
	WMI_LOGD(FL("IPv6 addr present: %d, addr: %pI6"),
		 req->is_ipv6_addr_present, req->ipv6_addr);
	WMI_LOGD(FL("port: %d present: %d"), req->is_port_present, req->port);
	WMI_LOGD(FL("protocol: %d present: %d"),
		 req->is_protocol_present, req->protocol);

	WMI_LOGD("vdev_id = %d, transaction_id: %d, csid: %d",
		 cmd->vdev_id, cmd->transaction_id, cmd->nan_csid);

	WMI_LOGD("ndp_config len: %d",
		 req->ndp_config.ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   req->ndp_config.ndp_cfg,
			   req->ndp_config.ndp_cfg_len);

	WMI_LOGD("ndp_app_info len: %d",
		 req->ndp_info.ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   req->ndp_info.ndp_app_info,
			   req->ndp_info.ndp_app_info_len);

	WMI_LOGD("pmk len: %d", cmd->nan_pmk_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   req->pmk.pmk, cmd->nan_pmk_len);

	WMI_LOGD("pass phrase len: %d", cmd->nan_passphrase_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   req->passphrase.passphrase,
			   cmd->nan_passphrase_len);

	WMI_LOGD("service name len: %d", cmd->nan_servicename_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   req->service_name.service_name,
			   cmd->nan_servicename_len);

	WMI_LOGD("sending WMI_NDP_RESPONDER_REQ_CMDID(0x%X)",
		 WMI_NDP_RESPONDER_REQ_CMDID);
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

	WMI_LOGD("Sending WMI_NDP_END_REQ_CMDID to FW");
	wmi_mtrace(WMI_NDP_END_REQ_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_NDP_END_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_NDP_END_REQ_CMDID failed, ret: %d", status);
		wmi_buf_free(buf);
	}

	return status;
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

	WMI_LOGD("WMI_NDP_INDICATION_EVENTID(0x%X) received. vdev %d,\n"
		"service_instance %d, ndp_instance %d, role %d, policy %d,\n"
		"csid: %d, scid_len: %d, peer_addr: %pM, peer_disc_addr: %pM",
		 WMI_NDP_INDICATION_EVENTID, fixed_params->vdev_id,
		 fixed_params->service_instance_id,
		 fixed_params->ndp_instance_id, fixed_params->self_ndp_role,
		 fixed_params->accept_policy,
		 fixed_params->nan_csid, fixed_params->nan_scid_len,
		 rsp->peer_mac_addr.bytes,
		 rsp->peer_discovery_mac_addr.bytes);

	WMI_LOGD("ndp_cfg - %d bytes", fixed_params->ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   &event->ndp_cfg, fixed_params->ndp_cfg_len);

	WMI_LOGD("ndp_app_info - %d bytes",
		 fixed_params->ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			&event->ndp_app_info, fixed_params->ndp_app_info_len);

	rsp->ndp_config.ndp_cfg_len = fixed_params->ndp_cfg_len;
	rsp->ndp_info.ndp_app_info_len = fixed_params->ndp_app_info_len;
	rsp->ncs_sk_type = fixed_params->nan_csid;
	rsp->scid.scid_len = fixed_params->nan_scid_len;

	if (rsp->ndp_config.ndp_cfg_len > NDP_QOS_INFO_LEN)
		rsp->ndp_config.ndp_cfg_len = NDP_QOS_INFO_LEN;
	qdf_mem_copy(rsp->ndp_config.ndp_cfg, event->ndp_cfg,
		     rsp->ndp_config.ndp_cfg_len);

	if (rsp->ndp_info.ndp_app_info_len > NDP_APP_INFO_LEN)
		rsp->ndp_info.ndp_app_info_len = NDP_APP_INFO_LEN;
	qdf_mem_copy(rsp->ndp_info.ndp_app_info, event->ndp_app_info,
		     rsp->ndp_info.ndp_app_info_len);

	if (rsp->scid.scid_len > NDP_SCID_BUF_LEN)
		rsp->scid.scid_len = NDP_SCID_BUF_LEN;
	qdf_mem_copy(rsp->scid.scid, event->ndp_scid, rsp->scid.scid_len);

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

	WMI_LOGD("scid hex dump:");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   rsp->scid.scid, rsp->scid.scid_len);

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

	event = (WMI_NDP_CONFIRM_EVENTID_param_tlvs *) data;
	fixed_params = (wmi_ndp_confirm_event_fixed_param *)event->fixed_param;
	WMI_LOGD("WMI_NDP_CONFIRM_EVENTID(0x%X) received. vdev %d, ndp_instance %d, rsp_code %d, reason_code: %d, num_active_ndps_on_peer: %d",
		 WMI_NDP_CONFIRM_EVENTID, fixed_params->vdev_id,
		 fixed_params->ndp_instance_id, fixed_params->rsp_code,
		 fixed_params->reason_code,
		 fixed_params->num_active_ndps_on_peer);
	WMI_LOGE("num_ch: %d", fixed_params->num_ndp_channels);

	if (fixed_params->ndp_cfg_len > event->num_ndp_cfg) {
		WMI_LOGE("FW message ndp cfg length %d larger than TLV hdr %d",
			 fixed_params->ndp_cfg_len, event->num_ndp_cfg);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("ndp_cfg - %d bytes", fixed_params->ndp_cfg_len);

	if (fixed_params->ndp_app_info_len > event->num_ndp_app_info) {
		WMI_LOGE("FW message ndp app info length %d more than TLV hdr %d",
			 fixed_params->ndp_app_info_len,
			 event->num_ndp_app_info);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("ndp_app_info - %d bytes",
		 fixed_params->ndp_app_info_len);

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
		WMI_LOGE(FL("NDP Ch count %d greater than NDP Ch TLV len (%d) or NSS TLV len (%d)"),
			 fixed_params->num_ndp_channels,
			 event->num_ndp_channel_list,
			 event->num_nss_list);
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
		rsp->ch[i].channel = event->ndp_channel_list[i].mhz;
		rsp->ch[i].nss = event->nss_list[i];
		ch_mode = WMI_GET_CHANNEL_MODE(&event->ndp_channel_list[i]);
		rsp->ch[i].ch_width = wmi_get_ch_width_from_phy_mode(wmi_handle,
								     ch_mode);
		WMI_LOGD(FL("ch: %d, ch_mode: %d, nss: %d"),
			 rsp->ch[i].channel,
			 rsp->ch[i].ch_width,
			 rsp->ch[i].nss);
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
	WMI_LOGD(FL("IPv6 addr present: %d, addr: %pI6"),
		 rsp->is_ipv6_addr_present, rsp->ipv6_addr);
	WMI_LOGD(FL("port: %d present: %d"), rsp->port, rsp->is_port_present);
	WMI_LOGD(FL("protocol: %d present: %d"),
		 rsp->protocol, rsp->is_protocol_present);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_ndp_responder_rsp_tlv(wmi_unified_t wmi_handle,
			uint8_t *data, struct nan_datapath_responder_rsp *rsp)
{
	WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_responder_rsp_event_fixed_param  *fixed_params;

	event = (WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	WMI_LOGD("WMI_NDP_RESPONDER_RSP_EVENTID(0x%X) received. vdev_id: %d, peer_mac_addr: %pM,transaction_id: %d, status_code %d, reason_code: %d, create_peer: %d",
		 WMI_NDP_RESPONDER_RSP_EVENTID, fixed_params->vdev_id,
		 rsp->peer_mac_addr.bytes, rsp->transaction_id,
		 rsp->status, rsp->reason, rsp->create_peer);

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
	if (!(*rsp)) {
		WMI_LOGE("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

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

	event = (WMI_NDL_SCHEDULE_UPDATE_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	WMI_LOGD(FL("flags: %d, num_ch: %d, num_ndp_instances: %d"),
		 fixed_params->flags, fixed_params->num_channels,
		 fixed_params->num_ndp_instances);

	if (fixed_params->num_channels > event->num_ndl_channel_list ||
	    fixed_params->num_channels > event->num_nss_list) {
		WMI_LOGE(FL("Channel count %d greater than NDP Ch list TLV len (%d) or NSS list TLV len (%d)"),
			 fixed_params->num_channels,
			 event->num_ndl_channel_list,
			 event->num_nss_list);
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
		ind->ch[i].channel = event->ndl_channel_list[i].mhz;
		ind->ch[i].nss = event->nss_list[i];
		ch_mode = WMI_GET_CHANNEL_MODE(&event->ndl_channel_list[i]);
		ind->ch[i].ch_width = wmi_get_ch_width_from_phy_mode(wmi_handle,
								     ch_mode);
		WMI_LOGD(FL("ch: %d, ch_mode: %d, nss: %d"),
			 ind->ch[i].channel,
			 ind->ch[i].ch_width,
			 ind->ch[i].nss);
	}

	for (i = 0; i < fixed_params->num_ndp_instances; i++)
		WMI_LOGD(FL("instance_id[%d]: %d"),
			 i, event->ndp_instance_list[i]);

	return QDF_STATUS_SUCCESS;
}

void wmi_nan_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_ndp_initiator_req_cmd = nan_ndp_initiator_req_tlv;
	ops->send_ndp_responder_req_cmd = nan_ndp_responder_req_tlv;
	ops->send_ndp_end_req_cmd = nan_ndp_end_req_tlv;
	ops->extract_ndp_initiator_rsp = extract_ndp_initiator_rsp_tlv;
	ops->extract_ndp_ind = extract_ndp_ind_tlv;
	ops->extract_ndp_confirm = extract_ndp_confirm_tlv;
	ops->extract_ndp_responder_rsp = extract_ndp_responder_rsp_tlv;
	ops->extract_ndp_end_rsp = extract_ndp_end_rsp_tlv;
	ops->extract_ndp_end_ind = extract_ndp_end_ind_tlv;
	ops->extract_ndp_sch_update = extract_ndp_sch_update_tlv;
}

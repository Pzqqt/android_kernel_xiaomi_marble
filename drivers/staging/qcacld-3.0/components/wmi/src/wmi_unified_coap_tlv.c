/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: Implement API's specific to CoAP component.
 */

#include <wmi_unified_priv.h>
#include "wmi.h"
#include "ol_defines.h"

/*
 * send_coap_add_pattern_cmd_tlv() - Send wmi cmd for adding CoAP pattern
 * @wmi_handle: wmi handle
 * @param: parameter for CoAP add pattern
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_coap_add_pattern_cmd_tlv(wmi_unified_t wmi_handle,
			      struct coap_offload_reply_param *param)
{
	WMI_WOW_COAP_ADD_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint8_t *buf_ptr;
	uint32_t len, coapmsg_len_align, verify_len_align;

	wmi_debug("vdev id %d pattern id %d timeout %d src ip 0x%x:%d coap msg len %d",
		  param->vdev_id, param->pattern_id, param->cache_timeout,
		  param->src_ip_v4, param->src_udp_port,
		  param->coapmsg_len);

	wmi_debug("filter: dest ip 0x%x:%d is bc %d verify offset %d len %d",
		  param->dest_ip_v4, param->dest_udp_port,
		  param->dest_ip_v4_is_bc, param->verify_offset,
		  param->verify_len);

	if (!param->verify || !param->verify_len ||
	    !param->coapmsg || !param->coapmsg_len) {
		wmi_err("invalid param");
		return QDF_STATUS_E_INVAL;
	}

	coapmsg_len_align = qdf_align(param->coapmsg_len, 4);
	verify_len_align = qdf_align(param->verify_len, 4);
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + coapmsg_len_align +
		WMI_TLV_HDR_SIZE + verify_len_align;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(buf);
	cmd = (WMI_WOW_COAP_ADD_PATTERN_CMD_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_WOW_COAP_ADD_PATTERN_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			WMI_WOW_COAP_ADD_PATTERN_CMD_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->pattern_id = param->pattern_id;
	cmd->timeout = param->cache_timeout;
	WMI_COAP_IPV6_SET(cmd->pattern_type, 0);
	WMI_COAP_ADDR_TYPE_SET(cmd->pattern_type,
			       param->dest_ip_v4_is_bc ? 1 : 0);
	qdf_mem_copy(cmd->match_udp_ip.ipv4_addr, &param->dest_ip_v4,
		     sizeof(param->dest_ip_v4));
	cmd->match_udp_port = param->dest_udp_port;
	qdf_mem_copy(cmd->udp_local_ip.ipv4_addr, &param->src_ip_v4,
		     sizeof(param->src_ip_v4));
	cmd->udp_local_port = param->src_udp_port;
	cmd->verify_offset = param->verify_offset;
	cmd->verify_len = param->verify_len;
	cmd->coapmsg_len = param->coapmsg_len;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, verify_len_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->verify, param->verify_len);

	buf_ptr += verify_len_align;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, coapmsg_len_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->coapmsg, param->coapmsg_len);
	buf_ptr += coapmsg_len_align;

	wmi_mtrace(WMI_WOW_COAP_ADD_PATTERN_CMDID,
		   cmd->vdev_id, cmd->pattern_id);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_WOW_COAP_ADD_PATTERN_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send wow coap add pattern command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/*
 * send_coap_del_pattern_cmd_tlv() - Send wmi cmd for deleting CoAP pattern
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_coap_del_pattern_cmd_tlv(wmi_unified_t wmi_handle,
			      uint8_t vdev_id, uint32_t pattern_id)
{
	WMI_WOW_COAP_DEL_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint32_t len = sizeof(*cmd);

	wmi_debug("vdev id %d pattern id %d", vdev_id, pattern_id);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (WMI_WOW_COAP_DEL_PATTERN_CMD_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_WMI_WOW_COAP_DEL_PATTERN_CMD_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(
			WMI_WOW_COAP_DEL_PATTERN_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern_id;
	wmi_mtrace(WMI_WOW_COAP_DEL_PATTERN_CMDID,
		   cmd->vdev_id, cmd->pattern_id);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_WOW_COAP_DEL_PATTERN_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send wow coap del pattern command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/*
 * send_coap_add_pattern_cmd_tlv() - Send wmi cmd for adding CoAP keepalive
 * pattern
 * @wmi_handle: wmi handle
 * @param: parameter for CoAP add pattern
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_coap_add_keepalive_pattern_cmd_tlv(wmi_unified_t wmi_handle,
			struct coap_offload_periodic_tx_param *param)
{
	WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint8_t *buf_ptr;
	uint32_t len, coapmsg_len_align;

	wmi_debug("vdev id %d pattern id %d ip src 0x%x:%d dest 0x%x:%d bc %d timeout %d",
		  param->vdev_id, param->pattern_id, param->src_ip_v4,
		  param->src_udp_port, param->dest_ip_v4,
		  param->dest_udp_port, param->dest_ip_v4_is_bc,
		  param->timeout);

	if (!param->coapmsg || !param->coapmsg_len) {
		wmi_err("invalid CoAP message");
		return QDF_STATUS_E_INVAL;
	}

	coapmsg_len_align = qdf_align(param->coapmsg_len, 4);
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + coapmsg_len_align;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(buf);
	cmd = (WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMD_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMD_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMD_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->pattern_id = param->pattern_id;

	/* only support IPv4 in current stage */
	WMI_COAP_IPV6_SET(cmd->pattern_type, 0);
	WMI_COAP_ADDR_TYPE_SET(cmd->pattern_type,
			       param->dest_ip_v4_is_bc ? 1 : 0);
	qdf_mem_copy(cmd->udp_remote_ip.ipv4_addr, &param->dest_ip_v4,
		     sizeof(param->dest_ip_v4));
	cmd->udp_remote_port = param->dest_udp_port;
	qdf_mem_copy(cmd->udp_local_ip.ipv4_addr, &param->src_ip_v4,
		     sizeof(param->src_ip_v4));
	cmd->udp_local_port = param->src_udp_port;
	cmd->timeout = param->timeout;
	cmd->coapmsg_len = param->coapmsg_len;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, coapmsg_len_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->coapmsg, param->coapmsg_len);
	buf_ptr += coapmsg_len_align;

	wmi_mtrace(WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMDID,
		   cmd->vdev_id, cmd->pattern_id);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_WOW_COAP_ADD_KEEPALIVE_PATTERN_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send wow coap add keepalive pattern command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/*
 * send_coap_del_pattern_cmd_tlv() - Send wmi cmd for deleting CoAP
 * keepalive pattern
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_coap_del_keepalive_pattern_cmd_tlv(wmi_unified_t wmi_handle,
					uint8_t vdev_id, uint32_t pattern_id)
{
	WMI_WOW_COAP_DEL_KEEPALIVE_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(*cmd);

	wmi_debug("vdev id %d pattern id %d", vdev_id, pattern_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = wmi_buf_data(buf);
	cmd = (WMI_WOW_COAP_DEL_KEEPALIVE_PATTERN_CMD_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_WOW_COAP_DEL_KEEPALIVE_PATTERN_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			WMI_WOW_COAP_DEL_PATTERN_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern_id;
	wmi_mtrace(WMI_WOW_COAP_DEL_KEEPALIVE_PATTERN_CMDID,
		   cmd->vdev_id, cmd->pattern_id);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_WOW_COAP_DEL_KEEPALIVE_PATTERN_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send wow coap del keepalive pattern command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/*
 * send_coap_cache_get_cmd_tlv() - Send wmi cmd for getting cached CoAP
 * messages
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
send_coap_cache_get_cmd_tlv(wmi_unified_t wmi_handle,
			    uint8_t vdev_id, uint32_t pattern_id)
{
	WMI_WOW_COAP_GET_BUF_INFO_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	uint32_t len = sizeof(*cmd);

	wmi_debug("vdev id %d pattern id %d", vdev_id, pattern_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (WMI_WOW_COAP_GET_BUF_INFO_CMD_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_WOW_COAP_GET_BUF_INFO_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			WMI_WOW_COAP_GET_BUF_INFO_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern_id;
	wmi_mtrace(WMI_WOW_COAP_GET_BUF_INFO_CMDID,
		   cmd->vdev_id, cmd->pattern_id);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_WOW_COAP_GET_BUF_INFO_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send wow coap get buf info command %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * coap_extract_buf_info_tlv() - Extract CoAP buf info event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer to the event buffer
 * @info: pointer to CoAP buf info
 *
 * The caller needs to free any possible nodes in info->info_list
 * regardless of failure or success.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
coap_extract_buf_info_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			  struct coap_buf_info *info)
{
	WMI_WOW_COAP_BUF_INFO_EVENT_fixed_param *buf_info_ev;
	WMI_WOW_COAP_BUF_INFO_EVENTID_param_tlvs *param_buf = evt_buf;
	wmi_coap_tuple *tuple;
	uint8_t *payload;
	uint32_t num_tuple, num_payload;
	struct coap_buf_node *buf_node;
	int i, j;

	buf_info_ev = param_buf->fixed_param;
	if (!buf_info_ev) {
		wmi_debug("received null event data from target");
		return QDF_STATUS_E_INVAL;
	}

	if (buf_info_ev->vdev_id > WLAN_MAX_VDEVS) {
		wmi_debug("received invalid vdev_id %d",
			  buf_info_ev->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	info->vdev_id = buf_info_ev->vdev_id;
	info->req_id = buf_info_ev->pattern_id;
	info->more_info = buf_info_ev->more_tuples;

	num_tuple = param_buf->num_coap_tuple;
	num_payload = param_buf->num_payloads;
	for (i = 0, j = 0; i < num_tuple && j < num_payload; i++) {
		tuple = &param_buf->coap_tuple[i];
		if (!tuple->payload_len) {
			wmi_err("idx %d: invalid payload len 0", i);
			continue;
		}

		payload = &param_buf->payloads[j];
		j += qdf_align(tuple->payload_len, 4);
		if (j > num_payload) {
			wmi_err("idx %d: payload len overflow, pos %d - total %d",
				i, j, num_payload);
			return QDF_STATUS_E_INVAL;
		}

		buf_node = qdf_mem_malloc(sizeof(*buf_node));
		if (!buf_node)
			return QDF_STATUS_E_NOMEM;

		buf_node->payload = qdf_mem_malloc(tuple->payload_len);
		if (!buf_node->payload) {
			qdf_mem_free(buf_node);
			return QDF_STATUS_E_NOMEM;
		}

		buf_node->tsf = tuple->tsf;
		qdf_mem_copy(&buf_node->src_ip, tuple->src_ip.ipv4_addr,
			     sizeof(buf_node->src_ip));
		buf_node->len = tuple->payload_len;
		qdf_mem_copy(buf_node->payload, payload, buf_node->len);
		qdf_list_insert_back(&info->info_list, &buf_node->node);

		wmi_debug("idx %d: src ip 0x%x tsf 0x%llx payload len %d",
			  i, buf_node->src_ip, buf_node->tsf, buf_node->len);
	}

	wmi_debug("vdev_id %d req_id %d num_tuple %d payload len %d more info %d",
		  info->vdev_id, info->req_id, num_tuple,
		  num_payload, info->more_info);
	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_coap_attach_tlv() - attach CoAP tlv handlers
 * @wmi_handle: wmi handle
 *
 * Return: void
 */
void wmi_coap_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_coap_add_pattern_cmd = send_coap_add_pattern_cmd_tlv;
	ops->send_coap_del_pattern_cmd = send_coap_del_pattern_cmd_tlv;
	ops->send_coap_add_keepalive_pattern_cmd =
		send_coap_add_keepalive_pattern_cmd_tlv;
	ops->send_coap_del_keepalive_pattern_cmd =
		send_coap_del_keepalive_pattern_cmd_tlv;
	ops->send_coap_cache_get_cmd = send_coap_cache_get_cmd_tlv;
	ops->extract_coap_buf_info = coap_extract_buf_info_tlv;
}

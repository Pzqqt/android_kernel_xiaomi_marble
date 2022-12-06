/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <osdep.h>
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_api.h"
#ifdef WLAN_MLO_MULTI_CHIP
#include "wmi_unified_11be_setup_api.h"
#endif
#include "wmi_unified_11be_tlv.h"

size_t vdev_create_mlo_params_size(struct vdev_create_params *param)
{
	if (qdf_is_macaddr_zero((struct qdf_mac_addr *)param->mlo_mac))
		return WMI_TLV_HDR_SIZE;

	return sizeof(wmi_vdev_create_mlo_params) + WMI_TLV_HDR_SIZE;
}

uint8_t *vdev_create_add_mlo_params(uint8_t *buf_ptr,
				    struct vdev_create_params *param)
{
	wmi_vdev_create_mlo_params *mlo_params;

	if (qdf_is_macaddr_zero((struct qdf_mac_addr *)param->mlo_mac)) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		return buf_ptr + WMI_TLV_HDR_SIZE;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_vdev_create_mlo_params));
	buf_ptr += sizeof(uint32_t);

	mlo_params = (wmi_vdev_create_mlo_params *)buf_ptr;
	WMITLV_SET_HDR(&mlo_params->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_mlo_params,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_create_mlo_params));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->mlo_mac, &mlo_params->mld_macaddr);

	wmi_debug("MLD Addr = "QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(param->mlo_mac));
	return buf_ptr + sizeof(wmi_vdev_create_mlo_params);
}

size_t vdev_start_mlo_params_size(struct vdev_start_params *req)
{
	size_t vdev_start_mlo_size;

	vdev_start_mlo_size = sizeof(wmi_vdev_start_mlo_params) +
			      WMI_TLV_HDR_SIZE +
			      (req->mlo_partner.num_links *
			      sizeof(wmi_partner_link_params)) +
			      WMI_TLV_HDR_SIZE;

	return vdev_start_mlo_size;
}

#ifdef WLAN_MCAST_MLO
static void vdev_start_add_mlo_mcast_params(uint32_t *mlo_flags,
					    struct vdev_start_params *req)
{
	WMI_MLO_FLAGS_SET_MCAST_VDEV(*mlo_flags,
				     req->mlo_flags.mlo_mcast_vdev);
}
#else
#define vdev_start_add_mlo_mcast_params(mlo_flags, req)
#endif

uint8_t *vdev_start_add_mlo_params(uint8_t *buf_ptr,
				   struct vdev_start_params *req)
{
	wmi_vdev_start_mlo_params *mlo_params;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_vdev_start_mlo_params));
	buf_ptr += sizeof(uint32_t);

	mlo_params = (wmi_vdev_start_mlo_params *)buf_ptr;
	WMITLV_SET_HDR(&mlo_params->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_mlo_params,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_start_mlo_params));

	mlo_params->mlo_flags.mlo_flags = 0;
	WMI_MLO_FLAGS_SET_ENABLED(mlo_params->mlo_flags.mlo_flags,
				  req->mlo_flags.mlo_enabled);
	WMI_MLO_FLAGS_SET_ASSOC_LINK(mlo_params->mlo_flags.mlo_flags,
				     req->mlo_flags.mlo_assoc_link);

	vdev_start_add_mlo_mcast_params(&mlo_params->mlo_flags.mlo_flags,
					req);

	return buf_ptr + sizeof(wmi_vdev_start_mlo_params);
}

uint8_t *vdev_start_add_ml_partner_links(uint8_t *buf_ptr,
					 struct vdev_start_params *req)
{
	wmi_partner_link_params *ml_partner_link;
	struct mlo_vdev_start_partner_links *req_partner;
	uint8_t i;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		      (req->mlo_partner.num_links *
		      sizeof(wmi_partner_link_params)));
	buf_ptr += sizeof(uint32_t);

	req_partner = &req->mlo_partner;
	ml_partner_link = (wmi_partner_link_params *)buf_ptr;
	for (i = 0; i < req->mlo_partner.num_links; i++) {
		WMITLV_SET_HDR(&ml_partner_link->tlv_header,
			       WMITLV_TAG_STRUC_wmi_partner_link_params,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_partner_link_params));
		ml_partner_link->vdev_id = req_partner->partner_info[i].vdev_id;
		ml_partner_link->hw_link_id =
				req_partner->partner_info[i].hw_mld_link_id;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(req_partner->partner_info[i].mac_addr,
					   &ml_partner_link->vdev_macaddr);
		ml_partner_link++;
	}

	return buf_ptr +
		(req->mlo_partner.num_links *
		 sizeof(wmi_partner_link_params));
}

uint8_t *bcn_tmpl_add_ml_partner_links(uint8_t *buf_ptr,
				       struct beacon_tmpl_params *param)
{
	wmi_bcn_tmpl_ml_params *ml_partner_link;
	struct mlo_bcn_templ_partner_links *ml_bcn_tmpl;
	uint8_t i;

	if (param->mlo_partner.num_links > WLAN_UMAC_MLO_MAX_VDEVS) {
		wmi_err("mlo_partner.num_link(%d) are greater than supported partner links(%d)",
			param->mlo_partner.num_links, WLAN_UMAC_MLO_MAX_VDEVS);
		return buf_ptr;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (param->mlo_partner.num_links *
			sizeof(wmi_bcn_tmpl_ml_params)));
	buf_ptr += sizeof(uint32_t);

	ml_bcn_tmpl = &param->mlo_partner;
	ml_partner_link = (wmi_bcn_tmpl_ml_params *)buf_ptr;
	for (i = 0; i < ml_bcn_tmpl->num_links; i++) {
		WMITLV_SET_HDR(&ml_partner_link->tlv_header,
			       WMITLV_TAG_STRUC_wmi_bcn_tmpl_ml_params,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_tmpl_ml_params)
			       );
		ml_partner_link->vdev_id = ml_bcn_tmpl->partner_info[i].vdev_id;
		ml_partner_link->hw_link_id =
			ml_bcn_tmpl->partner_info[i].hw_link_id;
		ml_partner_link->beacon_interval =
			ml_bcn_tmpl->partner_info[i].beacon_interval;
		ml_partner_link->csa_switch_count_offset =
			ml_bcn_tmpl->partner_info[i].csa_switch_count_offset;
		ml_partner_link->ext_csa_switch_count_offset =
			ml_bcn_tmpl->partner_info[i].ext_csa_switch_count_offset;
		ml_partner_link++;
	}

	return buf_ptr +
		(param->mlo_partner.num_links *
		 sizeof(wmi_bcn_tmpl_ml_params));
}

size_t peer_create_mlo_params_size(struct peer_create_params *req)
{
	return sizeof(wmi_peer_create_mlo_params) + WMI_TLV_HDR_SIZE;
}

uint8_t *peer_create_add_mlo_params(uint8_t *buf_ptr,
				    struct peer_create_params *req)
{
	wmi_peer_create_mlo_params *mlo_params;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_create_mlo_params));
	buf_ptr += sizeof(uint32_t);

	mlo_params = (wmi_peer_create_mlo_params *)buf_ptr;
	WMITLV_SET_HDR(&mlo_params->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_create_mlo_params,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_peer_create_mlo_params));

	mlo_params->mlo_flags.mlo_flags = 0;
	WMI_MLO_FLAGS_SET_ENABLED(mlo_params->mlo_flags.mlo_flags,
				  req->mlo_enabled);

	return buf_ptr + sizeof(wmi_peer_create_mlo_params);
}

size_t peer_assoc_mlo_params_size(struct peer_assoc_params *req)
{
	size_t peer_assoc_mlo_size = sizeof(wmi_peer_assoc_mlo_params) +
			WMI_TLV_HDR_SIZE +
			(req->ml_links.num_links *
			sizeof(wmi_peer_assoc_mlo_partner_link_params)) +
			WMI_TLV_HDR_SIZE;

	return peer_assoc_mlo_size;
}

uint8_t *peer_assoc_add_mlo_params(uint8_t *buf_ptr,
				   struct peer_assoc_params *req)
{
	wmi_peer_assoc_mlo_params *mlo_params;

	/* Add WMI peer assoc mlo params */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_assoc_mlo_params));
	buf_ptr += sizeof(uint32_t);

	mlo_params = (wmi_peer_assoc_mlo_params *)buf_ptr;
	WMITLV_SET_HDR(&mlo_params->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_assoc_mlo_params,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_peer_assoc_mlo_params));

	mlo_params->mlo_flags.mlo_flags = 0;
	WMI_MLO_FLAGS_SET_ENABLED(mlo_params->mlo_flags.mlo_flags,
				  req->mlo_params.mlo_enabled);
	WMI_MLO_FLAGS_SET_ASSOC_LINK(mlo_params->mlo_flags.mlo_flags,
				     req->mlo_params.mlo_assoc_link);
	WMI_MLO_FLAGS_SET_PRIMARY_UMAC(mlo_params->mlo_flags.mlo_flags,
				       req->mlo_params.mlo_primary_umac);
	WMI_MLO_FLAGS_SET_LINK_INDEX_VALID(mlo_params->mlo_flags.mlo_flags,
					   req->mlo_params.mlo_logical_link_index_valid);
	WMI_MLO_FLAGS_SET_PEER_ID_VALID(mlo_params->mlo_flags.mlo_flags,
					req->mlo_params.mlo_peer_id_valid);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(req->mlo_params.mld_mac,
				   &mlo_params->mld_macaddr);
	mlo_params->logical_link_index = req->mlo_params.logical_link_index;
	mlo_params->mld_peer_id = req->mlo_params.ml_peer_id;

	return buf_ptr + sizeof(wmi_peer_assoc_mlo_params);
}

uint8_t *peer_assoc_add_ml_partner_links(uint8_t *buf_ptr,
					 struct peer_assoc_params *req)
{
	wmi_peer_assoc_mlo_partner_link_params *ml_partner_link;
	struct ml_partner_info *partner_info;
	uint8_t i;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (req->ml_links.num_links *
		       sizeof(wmi_peer_assoc_mlo_partner_link_params)));
	buf_ptr += sizeof(uint32_t);

	ml_partner_link = (wmi_peer_assoc_mlo_partner_link_params *)buf_ptr;
	partner_info = req->ml_links.partner_info;
	for (i = 0; i < req->ml_links.num_links; i++) {
		WMITLV_SET_HDR(&ml_partner_link->tlv_header,
			       WMITLV_TAG_STRUC_wmi_peer_assoc_mlo_partner_link_params,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_peer_assoc_mlo_partner_link_params));
		ml_partner_link->vdev_id = partner_info[i].vdev_id;
		ml_partner_link->hw_mld_link_id = partner_info[i].hw_mld_link_id;
		ml_partner_link++;
	}

	return buf_ptr +
	       (req->ml_links.num_links *
		sizeof(wmi_peer_assoc_mlo_partner_link_params));
}

/**
 * force_mode_host_to_fw() - translate force mode for MLO link set active
 *  command
 * @host_mode: force mode defined by host
 * @fw_mode: buffer to store force mode defined by FW
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_INVAL otherwise
 */
static inline QDF_STATUS
force_mode_host_to_fw(enum mlo_link_force_mode host_mode,
		      WMI_MLO_LINK_FORCE_MODE *fw_mode)
{
	switch (host_mode) {
	case MLO_LINK_FORCE_MODE_ACTIVE:
		*fw_mode = WMI_MLO_LINK_FORCE_ACTIVE;
		break;
	case MLO_LINK_FORCE_MODE_INACTIVE:
		*fw_mode = WMI_MLO_LINK_FORCE_INACTIVE;
		break;
	case MLO_LINK_FORCE_MODE_ACTIVE_NUM:
		*fw_mode = WMI_MLO_LINK_FORCE_ACTIVE_LINK_NUM;
		break;
	case MLO_LINK_FORCE_MODE_INACTIVE_NUM:
		*fw_mode = WMI_MLO_LINK_FORCE_INACTIVE_LINK_NUM;
		break;
	case MLO_LINK_FORCE_MODE_NO_FORCE:
		*fw_mode = WMI_MLO_LINK_NO_FORCE;
		break;
	default:
		wmi_err("Invalid force mode: %d", host_mode);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * force_reason_host_to_fw() - translate force reason for MLO link set active
 *  command
 * @host_reason: force reason defined by host
 * @fw_reason: buffer to store force reason defined by FW
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_INVAL otherwise
 */
static inline QDF_STATUS
force_reason_host_to_fw(enum mlo_link_force_reason host_reason,
			WMI_MLO_LINK_FORCE_REASON *fw_reason)
{
	switch (host_reason) {
	case MLO_LINK_FORCE_REASON_CONNECT:
		*fw_reason = WMI_MLO_LINK_FORCE_REASON_NEW_CONNECT;
		break;
	case MLO_LINK_FORCE_REASON_DISCONNECT:
		*fw_reason =  WMI_MLO_LINK_FORCE_REASON_NEW_DISCONNECT;
		break;
	default:
		wmi_err("Invalid force reason: %d", host_reason);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_mlo_link_set_active_cmd_tlv() - send mlo link set active command
 * @wmi_handle: wmi handle
 * @param: Pointer to mlo link set active param
 *
 * Return: QDF_STATUS_SUCCESS for success or QDF_STATUS_E_* for error
 */
static QDF_STATUS
send_mlo_link_set_active_cmd_tlv(wmi_unified_t wmi_handle,
				 struct mlo_link_set_active_param *param)
{
	QDF_STATUS status;
	wmi_mlo_link_set_active_cmd_fixed_param *cmd;
	wmi_mlo_set_active_link_number_param *link_num_param;
	uint32_t *vdev_bitmap;
	uint32_t num_link_num_param = 0, num_vdev_bitmap = 0, tlv_len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	int i;
	WMITLV_TAG_ID tag_id;
	WMI_MLO_LINK_FORCE_MODE force_mode;
	WMI_MLO_LINK_FORCE_REASON force_reason;

	if (!param->num_vdev_bitmap && !param->num_link_entry) {
		wmi_err("No entry is provided vdev bit map %d link entry %d",
			param->num_vdev_bitmap,
			param->num_link_entry);
		return QDF_STATUS_E_INVAL;
	}

	status = force_mode_host_to_fw(param->force_mode, &force_mode);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_INVAL;

	status = force_reason_host_to_fw(param->reason, &force_reason);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_INVAL;

	switch (force_mode) {
	case WMI_MLO_LINK_FORCE_ACTIVE_LINK_NUM:
	case WMI_MLO_LINK_FORCE_INACTIVE_LINK_NUM:
		num_link_num_param = param->num_link_entry;
		/* fallthrough */
	case WMI_MLO_LINK_FORCE_ACTIVE:
	case WMI_MLO_LINK_FORCE_INACTIVE:
	case WMI_MLO_LINK_NO_FORCE:
		num_vdev_bitmap = param->num_vdev_bitmap;
		break;
	default:
		wmi_err("Invalid force reason: %d", force_mode);
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd) +
	      WMI_TLV_HDR_SIZE + sizeof(*link_num_param) * num_link_num_param +
	      WMI_TLV_HDR_SIZE + sizeof(*vdev_bitmap) * num_vdev_bitmap;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_mlo_link_set_active_cmd_fixed_param *)buf_ptr;
	tlv_len = WMITLV_GET_STRUCT_TLVLEN
			(wmi_mlo_link_set_active_cmd_fixed_param);

	tag_id = WMITLV_TAG_STRUC_wmi_mlo_link_set_active_cmd_fixed_param;
	WMITLV_SET_HDR(&cmd->tlv_header, tag_id, tlv_len);
	cmd->force_mode = force_mode;
	cmd->reason = force_reason;
	wmi_debug("mode %d reason %d num_link_num_param %d num_vdev_bitmap %d",
		  cmd->force_mode, cmd->reason, num_link_num_param,
		  num_vdev_bitmap);
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(*link_num_param) * num_link_num_param);
	buf_ptr += WMI_TLV_HDR_SIZE;

	if (num_link_num_param) {
		link_num_param =
			(wmi_mlo_set_active_link_number_param *)buf_ptr;
		tlv_len = WMITLV_GET_STRUCT_TLVLEN
				(wmi_mlo_set_active_link_number_param);
		for (i = 0; i < num_link_num_param; i++) {
			WMITLV_SET_HDR(&link_num_param->tlv_header, 0, tlv_len);
			link_num_param->num_of_link =
				param->link_num[i].num_of_link;
			link_num_param->vdev_type =
				param->link_num[i].vdev_type;
			link_num_param->vdev_subtype =
				param->link_num[i].vdev_subtype;
			link_num_param->home_freq =
				param->link_num[i].home_freq;
			wmi_debug("entry[%d]: num_of_link %d vdev type %d subtype %d freq %d",
				  i, link_num_param->num_of_link,
				  link_num_param->vdev_type,
				  link_num_param->vdev_subtype,
				  link_num_param->home_freq);
			link_num_param++;
		}

		buf_ptr += sizeof(*link_num_param) * num_link_num_param;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       sizeof(*vdev_bitmap) * num_vdev_bitmap);
	buf_ptr += WMI_TLV_HDR_SIZE;

	if (num_vdev_bitmap) {
		vdev_bitmap = (A_UINT32 *)(buf_ptr);
		for (i = 0; i < num_vdev_bitmap; i++) {
			vdev_bitmap[i] = param->vdev_bitmap[i];
			wmi_debug("entry[%d]: vdev_id_bitmap 0x%x ",
				  i, vdev_bitmap[i]);
		}

		buf_ptr += sizeof(*vdev_bitmap) * num_vdev_bitmap;
	}

	wmi_mtrace(WMI_MLO_LINK_SET_ACTIVE_CMDID, 0, cmd->force_mode);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_MLO_LINK_SET_ACTIVE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send MLO link set active command to FW: %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * extract_mlo_link_set_active_resp_tlv() - extract mlo link set active resp
 *  from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @resp: Pointer to hold mlo link set active resp
 *
 * Return: QDF_STATUS_SUCCESS for success or QDF_STATUS_E_* for error
 */
static QDF_STATUS
extract_mlo_link_set_active_resp_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				     struct mlo_link_set_active_resp *resp)
{
	wmi_mlo_link_set_active_resp_event_fixed_param *evt;
	WMI_MLO_LINK_SET_ACTIVE_RESP_EVENTID_param_tlvs *param_buf;
	uint32_t entry_num, *bitmap;
	int i;

	param_buf = evt_buf;
	if (!param_buf || !resp) {
		wmi_err("Invalid param");
		return QDF_STATUS_E_INVAL;
	}

	evt = param_buf->fixed_param;
	resp->status = evt->status;
	wmi_debug("status: %u", resp->status);

	bitmap = param_buf->force_active_vdev_bitmap;
	entry_num = qdf_min(param_buf->num_force_active_vdev_bitmap,
			    (uint32_t)MLO_VDEV_BITMAP_SZ);
	resp->active_sz = entry_num;
	for (i = 0; i < entry_num; i++) {
		resp->active[i] = bitmap[i];
		wmi_debug("active[%d]: 0x%x", i, resp->active[i]);
	}

	bitmap = param_buf->force_inactive_vdev_bitmap;
	entry_num = qdf_min(param_buf->num_force_inactive_vdev_bitmap,
			    (uint32_t)MLO_VDEV_BITMAP_SZ);
	resp->inactive_sz = entry_num;
	for (i = 0; i < entry_num; i++) {
		resp->inactive[i] = bitmap[i];
		wmi_debug("inactive[%d]: 0x%x", i, resp->inactive[i]);
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_MLO_MULTI_CHIP
QDF_STATUS mlo_setup_cmd_send_tlv(struct wmi_unified *wmi_handle,
				  struct wmi_mlo_setup_params *param)
{
	QDF_STATUS ret;
	wmi_mlo_setup_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	uint8_t *buf_ptr;
	uint32_t *partner_links;
	uint8_t idx;

	if (param->num_valid_hw_links > MAX_LINK_IN_MLO)
		return QDF_STATUS_E_INVAL;

	len = sizeof(*cmd) +
		(param->num_valid_hw_links * sizeof(uint32_t)) +
		WMI_TLV_HDR_SIZE;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_mlo_setup_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_mlo_setup_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_mlo_setup_cmd_fixed_param));

	cmd->mld_group_id = param->mld_grp_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (sizeof(uint32_t) * param->num_valid_hw_links));
	partner_links = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (idx = 0; idx < param->num_valid_hw_links; idx++)
		partner_links[idx] = param->partner_links[idx];

	wmi_mtrace(WMI_MLO_SETUP_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_MLO_SETUP_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send MLO setup command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

QDF_STATUS mlo_ready_cmd_send_tlv(struct wmi_unified *wmi_handle,
				  struct wmi_mlo_ready_params *param)
{
	QDF_STATUS ret;
	wmi_mlo_ready_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_mlo_ready_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_mlo_ready_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_mlo_ready_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);

	wmi_mtrace(WMI_MLO_READY_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_MLO_READY_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send MLO ready command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

QDF_STATUS mlo_teardown_cmd_send_tlv(struct wmi_unified *wmi_handle,
				     struct wmi_mlo_teardown_params *param)
{
	QDF_STATUS ret;
	wmi_mlo_teardown_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_mlo_teardown_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_mlo_teardown_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_mlo_teardown_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								wmi_handle,
								param->pdev_id);
	switch (param->reason) {
	case WMI_MLO_TEARDOWN_REASON_SSR:
		cmd->reason_code = WMI_MLO_TEARDOWN_SSR_REASON;
		break;
	case WMI_MLO_TEARDOWN_REASON_DOWN:
	default:
		cmd->reason_code = WMI_MLO_TEARDOWN_SSR_REASON + 1;
		break;
	}

	wmi_mtrace(WMI_MLO_TEARDOWN_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_MLO_TEARDOWN_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send MLO Teardown command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

QDF_STATUS
extract_mlo_setup_cmpl_event_tlv(struct wmi_unified *wmi_handle,
				 uint8_t *buf,
				 struct wmi_mlo_setup_complete_params *params)
{
	WMI_MLO_SETUP_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_mlo_setup_complete_event_fixed_param *ev;

	param_buf = (WMI_MLO_SETUP_COMPLETE_EVENTID_param_tlvs *)buf;
	if (!param_buf) {
		wmi_err_rl("Param_buf is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ev = (wmi_mlo_setup_complete_event_fixed_param *)param_buf->fixed_param;

	params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								wmi_handle,
								ev->pdev_id);
	if (!ev->status)
		params->status = WMI_MLO_SETUP_STATUS_SUCCESS;
	else
		params->status = WMI_MLO_SETUP_STATUS_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
extract_mlo_teardown_cmpl_event_tlv(struct wmi_unified *wmi_handle,
				    uint8_t *buf,
				    struct wmi_mlo_teardown_cmpl_params *params)
{
	WMI_MLO_TEARDOWN_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_mlo_teardown_complete_fixed_param *ev;

	param_buf = (WMI_MLO_TEARDOWN_COMPLETE_EVENTID_param_tlvs *)buf;
	if (!param_buf) {
		wmi_err_rl("Param_buf is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ev = (wmi_mlo_teardown_complete_fixed_param *)param_buf->fixed_param;

	params->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								wmi_handle,
								ev->pdev_id);
	if (!ev->status)
		params->status = WMI_MLO_TEARDOWN_STATUS_SUCCESS;
	else
		params->status = WMI_MLO_TEARDOWN_STATUS_FAILURE;

	return QDF_STATUS_SUCCESS;
}

static void wmi_11be_attach_mlo_setup_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->mlo_setup_cmd_send = mlo_setup_cmd_send_tlv;
	ops->mlo_teardown_cmd_send = mlo_teardown_cmd_send_tlv;
	ops->mlo_ready_cmd_send = mlo_ready_cmd_send_tlv;
	ops->extract_mlo_setup_cmpl_event = extract_mlo_setup_cmpl_event_tlv;
	ops->extract_mlo_teardown_cmpl_event =
					extract_mlo_teardown_cmpl_event_tlv;
}

#else /*WLAN_MLO_MULTI_CHIP*/

static void wmi_11be_attach_mlo_setup_tlv(wmi_unified_t wmi_handle)
{}

#endif /*WLAN_MLO_MULTI_CHIP*/

void wmi_11be_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	wmi_11be_attach_mlo_setup_tlv(wmi_handle);
	ops->extract_mlo_link_set_active_resp =
		extract_mlo_link_set_active_resp_tlv;
	ops->send_mlo_link_set_active_cmd =
		send_mlo_link_set_active_cmd_tlv;
}

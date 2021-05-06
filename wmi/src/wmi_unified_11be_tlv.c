/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "wmi_unified_11be_tlv.h"

size_t vdev_create_mlo_params_size(void)
{
	return sizeof(wmi_vdev_create_mlo_params) + WMI_TLV_HDR_SIZE;
}

uint8_t *vdev_create_add_mlo_params(uint8_t *buf_ptr,
				    struct vdev_create_params *param)
{
	wmi_vdev_create_mlo_params *mlo_params;

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

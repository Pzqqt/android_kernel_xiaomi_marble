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

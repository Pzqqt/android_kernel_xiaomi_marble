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

#ifndef _WMI_UNIFIED_11BE_TLV_H_
#define _WMI_UNIFIED_11BE_TLV_H_

#ifdef WLAN_FEATURE_11BE_MLO
/**
 *  vdev_create_mlo_params_size() - Get MLO params size in vdev create
 *
 *  Return: size of MLO params in vdev create
 */
size_t vdev_create_mlo_params_size(void);
/**
 *  vdev_create_add_mlo_params() - Add MLO params in vdev create cmd
 *  @buf_ptr: pointer to vdev create buffer.
 *  @param: ponter to vdev create request param
 *
 *  Return: pointer to new offset of vdev create buffer
 */
uint8_t *vdev_create_add_mlo_params(uint8_t *buf_ptr,
				    struct vdev_create_params *param);
/**
 *  vdev_start_mlo_params_size() - Get MLO params size in vdev start
 *  @req: Vdev start request params
 *
 *  Return: size of MLO params in vdev start
 */
size_t vdev_start_mlo_params_size(struct vdev_start_params *req);
/**
 *  vdev_start_add_mlo_params() - Add MLO params in vdev start cmd
 *  @buf_ptr: pointer to vdev start buffer.
 *  @req: ponter to vdev create request param
 *
 *  Return: pointer to new offset of cmd buffer
 */
uint8_t *vdev_start_add_mlo_params(uint8_t *buf_ptr,
				   struct vdev_start_params *req);
/**
 *  vdev_start_add_ml_partner_links() - Add MLO partner links in vdev start cmd
 *  @buf_ptr: pointer to vdev start cmd buffer.
 *  @req: ponter to vdev start request param
 *
 *  Return: pointer to new offset of cmd buffer
 */
uint8_t *vdev_start_add_ml_partner_links(uint8_t *buf_ptr,
					 struct vdev_start_params *req);
/**
 * bcn_tmpl_add_ml_partner_links - Add MLO partner links in beacon template
 *                                 command
 * @buf_ptr: pointer to beacon cmd buffer.
 * @param: pointer to beacon template params
 *
 * Return: pointer to new offset of cmd buffer
 */
uint8_t *bcn_tmpl_add_ml_partner_links(uint8_t *buf_ptr,
				       struct beacon_tmpl_params *param);
/**
 *  peer_create_add_mlo_params() - Add MLO params in peer create cmd
 *  @buf_ptr: pointer to peer create cmd buffer.
 *  @req: pointer to peer create request param
 *
 *  Return: pointer to new offset of cmd buffer
 */
uint8_t *peer_create_add_mlo_params(uint8_t *buf_ptr,
				    struct peer_create_params *req);
/**
 *  peer_create_mlo_params_size() - Get ML params size in peer create
 *  @req: pointer to peer create request param
 *
 *  Return: size of ML params in peer create cmd
 */
size_t peer_create_mlo_params_size(struct peer_create_params *req);
/**
 *  peer_assoc_mlo_params_size() - Get ML params size in peer assoc
 *  @req: pointer to peer assoc request param
 *
 *  Return: size of ML params in peer assoc cmd
 */
size_t peer_assoc_mlo_params_size(struct peer_assoc_params *req);
/**
 *  peer_assoc_add_mlo_params() - Add MLO params in peer assoc cmd
 *  @buf_ptr: pointer to peer assoc cmd buffer.
 *  @req: pointer to peer assoc request param
 *
 *  Return: pointer to new offset of cmd buffer
 */
uint8_t *peer_assoc_add_mlo_params(uint8_t *buf_ptr,
				   struct peer_assoc_params *req);
/**
 *  peer_assoc_add_ml_partner_links() - Add MLO partner links in peer assoc cmd
 *  @buf_ptr: pointer to peer assoc cmd buffer.
 *  @req: pointer to peer assoc request param
 *
 *  Return: pointer to new offset of cmd buffer
 */
uint8_t *peer_assoc_add_ml_partner_links(uint8_t *buf_ptr,
					 struct peer_assoc_params *req);
/** wmi_11be_tlv_attach_tlv - Attach 11be relaated callbacks
 *  @wmi_handle: WMI handle
 */
void wmi_11be_attach_tlv(wmi_unified_t wmi_handle);
#else
static uint8_t *vdev_create_add_mlo_params(uint8_t *buf_ptr,
					   struct vdev_create_params *param)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static size_t vdev_create_mlo_params_size(void)
{
	return WMI_TLV_HDR_SIZE;
}

static uint8_t *vdev_start_add_mlo_params(uint8_t *buf_ptr,
					  struct vdev_start_params *req)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static size_t vdev_start_mlo_params_size(struct vdev_start_params *req)
{
	return WMI_TLV_HDR_SIZE;
}

static uint8_t *vdev_start_add_ml_partner_links(uint8_t *buf_ptr,
						struct vdev_start_params *req)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static uint8_t *bcn_tmpl_add_ml_partner_links(uint8_t *buf_ptr,
					      struct beacon_tmpl_params *param)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static uint8_t *peer_create_add_mlo_params(uint8_t *buf_ptr,
					  struct peer_create_params *req)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static size_t peer_create_mlo_params_size(struct peer_create_params *req)
{
	return WMI_TLV_HDR_SIZE;
}

static size_t peer_assoc_mlo_params_size(struct peer_assoc_params *req)
{
	size_t peer_assoc_mlo_size =
			WMI_TLV_HDR_SIZE +
			WMI_TLV_HDR_SIZE;

	return peer_assoc_mlo_size;
}

static uint8_t *peer_assoc_add_mlo_params(uint8_t *buf_ptr,
					  struct peer_assoc_params *req)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static uint8_t *peer_assoc_add_ml_partner_links(uint8_t *buf_ptr,
						struct peer_assoc_params *req)
{
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	return buf_ptr + WMI_TLV_HDR_SIZE;
}

static void wmi_11be_attach_tlv(wmi_unified_t wmi_handle)
{ }
#endif /*WLAN_FEATURE_11BE_MLO*/
#endif /*_WMI_UNIFIED_11BE_TLV_H_*/

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
#endif /*WLAN_FEATURE_11BE_MLO*/
#endif /*_WMI_UNIFIED_11BE_TLV_H_*/

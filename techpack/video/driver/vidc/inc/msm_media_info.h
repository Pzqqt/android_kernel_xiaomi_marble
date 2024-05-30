/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __MSM_MEDIA_INFO_H__
#define __MSM_MEDIA_INFO_H__

#include <media/v4l2_vidc_extensions.h>
#include <linux/videodev2.h>

/* Width and Height should be multiple of 16 */
#define INTERLACE_WIDTH_MAX 1920
#define INTERLACE_HEIGHT_MAX 1920
#define INTERLACE_MB_PER_FRAME_MAX ((1920*1088)/256)

#ifndef MSM_MEDIA_ALIGN
#define MSM_MEDIA_ALIGN(__sz, __align) (((__align) & ((__align) - 1)) ?\
	((((__sz) + (__align) - 1) / (__align)) * (__align)) :\
	(((__sz) + (__align) - 1) & (~((__align) - 1))))
#endif

#ifndef MSM_MEDIA_ROUNDUP
#define MSM_MEDIA_ROUNDUP(__sz, __r) (((__sz) + ((__r) - 1)) / (__r))
#endif

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_Y_STRIDE_BYTES(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int alignment, stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_VIDC_NV12C:
		alignment = 128;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		alignment = 256;
		stride = MSM_MEDIA_ALIGN(width, 192);
		stride = MSM_MEDIA_ALIGN(stride * 4/3, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 256;
		stride = MSM_MEDIA_ALIGN(width * 2, alignment);
		break;
	default:
		break;
	}
invalid_input:
	return stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_Y_STRIDE_PIX(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int alignment, stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_VIDC_NV12C:
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 128;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		alignment = 192;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	default:
		break;
	}
invalid_input:
	return stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_UV_STRIDE_BYTES(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int alignment, stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_VIDC_NV12C:
		alignment = 128;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		alignment = 256;
		stride = MSM_MEDIA_ALIGN(width, 192);
		stride = MSM_MEDIA_ALIGN(stride * 4/3, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 256;
		stride = MSM_MEDIA_ALIGN(width * 2, alignment);
		break;
	default:
		break;
	}
invalid_input:
	return stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_UV_STRIDE_PIX(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int alignment, stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_VIDC_NV12C:
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 128;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		alignment = 192;
		stride = MSM_MEDIA_ALIGN(width, alignment);
		break;
	default:
		break;
	}
invalid_input:
	return stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @height
 * Progressive: height
 * Interlaced: (height+1)>>1
 */
static inline unsigned int VIDEO_Y_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	unsigned int alignment, sclines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_VIDC_NV12C:
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 32;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		alignment = 16;
		break;
	default:
		return 0;
	}
	sclines = MSM_MEDIA_ALIGN(height, alignment);
invalid_input:
	return sclines;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @height
 * Progressive: height
 * Interlaced: (height+1)>>1
 */
static inline unsigned int VIDEO_UV_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	unsigned int alignment, sclines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_VIDC_TP10C:
	case V4L2_PIX_FMT_VIDC_P010:
		alignment = 16;
		break;
	case V4L2_PIX_FMT_VIDC_NV12C:
		alignment = 32;
		break;
	default:
		goto invalid_input;
	}

	sclines = MSM_MEDIA_ALIGN((height+1)>>1, alignment);

invalid_input:
	return sclines;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_Y_META_STRIDE(unsigned int v4l2_fmt,
	unsigned int width)
{
	int y_tile_width = 0, y_meta_stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_NV12C:
		y_tile_width = 32;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		y_tile_width = 48;
		break;
	default:
		goto invalid_input;
	}

	y_meta_stride = MSM_MEDIA_ROUNDUP(width, y_tile_width);
	y_meta_stride = MSM_MEDIA_ALIGN(y_meta_stride, 64);

invalid_input:
	return y_meta_stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @height
 * Progressive: height
 * Interlaced: (height+1)>>1
 */
static inline unsigned int VIDEO_Y_META_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	int y_tile_height = 0, y_meta_scanlines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_NV12C:
		y_tile_height = 8;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		y_tile_height = 4;
		break;
	default:
		goto invalid_input;
	}

	y_meta_scanlines = MSM_MEDIA_ROUNDUP(height, y_tile_height);
	y_meta_scanlines = MSM_MEDIA_ALIGN(y_meta_scanlines, 16);

invalid_input:
	return y_meta_scanlines;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @width
 * Progressive: width
 * Interlaced: width
 */
static inline unsigned int VIDEO_UV_META_STRIDE(unsigned int v4l2_fmt,
	unsigned int width)
{
	int uv_tile_width = 0, uv_meta_stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_NV12C:
		uv_tile_width = 16;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		uv_tile_width = 24;
		break;
	default:
		goto invalid_input;
	}

	uv_meta_stride = MSM_MEDIA_ROUNDUP((width+1)>>1, uv_tile_width);
	uv_meta_stride = MSM_MEDIA_ALIGN(uv_meta_stride, 64);

invalid_input:
	return uv_meta_stride;
}

/*
 * Function arguments:
 * @v4l2_fmt
 * @height
 * Progressive: height
 * Interlaced: (height+1)>>1
 */
static inline unsigned int VIDEO_UV_META_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	int uv_tile_height = 0, uv_meta_scanlines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_NV12C:
		uv_tile_height = 8;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		uv_tile_height = 4;
		break;
	default:
		goto invalid_input;
	}

	uv_meta_scanlines = MSM_MEDIA_ROUNDUP((height+1)>>1, uv_tile_height);
	uv_meta_scanlines = MSM_MEDIA_ALIGN(uv_meta_scanlines, 16);

invalid_input:
	return uv_meta_scanlines;
}

static inline unsigned int VIDEO_RGB_STRIDE_BYTES(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int alignment = 0, stride = 0, bpp = 4;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_ARGB32C:
	case V4L2_PIX_FMT_RGBA32:
		alignment = 256;
		break;
	default:
		goto invalid_input;
	}

	stride = MSM_MEDIA_ALIGN(width * bpp, alignment);

invalid_input:
	return stride;
}

static inline unsigned int VIDEO_RGB_STRIDE_PIX(unsigned int v4l2_fmt,
	unsigned int width)
{
	unsigned int bpp = 4;

	return VIDEO_RGB_STRIDE_BYTES(v4l2_fmt, width) / bpp;
}

static inline unsigned int VIDEO_RGB_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	unsigned int alignment = 0, scanlines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_ARGB32C:
		alignment = 16;
		break;
	case V4L2_PIX_FMT_RGBA32:
		alignment = 32;
		break;
	default:
		goto invalid_input;
	}

	scanlines = MSM_MEDIA_ALIGN(height, alignment);

invalid_input:
	return scanlines;
}

static inline unsigned int VIDEO_RGB_META_STRIDE(unsigned int v4l2_fmt,
	unsigned int width)
{
	int rgb_tile_width = 0, rgb_meta_stride = 0;

	if (!width)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_ARGB32C:
	case V4L2_PIX_FMT_RGBA32:
		rgb_tile_width = 16;
		break;
	default:
		goto invalid_input;
	}

	rgb_meta_stride = MSM_MEDIA_ROUNDUP(width, rgb_tile_width);
	rgb_meta_stride = MSM_MEDIA_ALIGN(rgb_meta_stride, 64);

invalid_input:
	return rgb_meta_stride;
}

static inline unsigned int VIDEO_RGB_META_SCANLINES(unsigned int v4l2_fmt,
	unsigned int height)
{
	int rgb_tile_height = 0, rgb_meta_scanlines = 0;

	if (!height)
		goto invalid_input;

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_VIDC_ARGB32C:
	case V4L2_PIX_FMT_RGBA32:
		rgb_tile_height = 4;
		break;
	default:
		goto invalid_input;
	}

	rgb_meta_scanlines = MSM_MEDIA_ROUNDUP(height, rgb_tile_height);
	rgb_meta_scanlines = MSM_MEDIA_ALIGN(rgb_meta_scanlines, 16);

invalid_input:
	return rgb_meta_scanlines;
}

static inline unsigned int VIDEO_RAW_BUFFER_SIZE(unsigned int v4l2_fmt,
	unsigned int pix_width, unsigned int pix_height, unsigned int interlace)
{
	unsigned int size = 0;
	unsigned int y_plane, uv_plane, y_stride,
		uv_stride, y_sclines, uv_sclines;
	unsigned int y_ubwc_plane = 0, uv_ubwc_plane = 0;
	unsigned int y_meta_stride = 0, y_meta_scanlines = 0;
	unsigned int uv_meta_stride = 0, uv_meta_scanlines = 0;
	unsigned int y_meta_plane = 0, uv_meta_plane = 0;
	unsigned int rgb_stride = 0, rgb_scanlines = 0;
	unsigned int rgb_plane = 0, rgb_ubwc_plane = 0, rgb_meta_plane = 0;
	unsigned int rgb_meta_stride = 0, rgb_meta_scanlines = 0;

	if (!pix_width || !pix_height)
		goto invalid_input;

	y_stride = VIDEO_Y_STRIDE_BYTES(v4l2_fmt, pix_width);
	uv_stride = VIDEO_UV_STRIDE_BYTES(v4l2_fmt, pix_width);
	y_sclines = VIDEO_Y_SCANLINES(v4l2_fmt, pix_height);
	uv_sclines = VIDEO_UV_SCANLINES(v4l2_fmt, pix_height);
	rgb_stride = VIDEO_RGB_STRIDE_BYTES(v4l2_fmt, pix_width);
	rgb_scanlines = VIDEO_RGB_SCANLINES(v4l2_fmt, pix_height);

	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV21:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_VIDC_P010:
		y_plane = y_stride * y_sclines;
		uv_plane = uv_stride * uv_sclines;
		size = y_plane + uv_plane;
		break;
	case V4L2_PIX_FMT_VIDC_NV12C:
		y_meta_stride = VIDEO_Y_META_STRIDE(v4l2_fmt, pix_width);
		uv_meta_stride = VIDEO_UV_META_STRIDE(v4l2_fmt, pix_width);
		if (!interlace && v4l2_fmt == V4L2_PIX_FMT_VIDC_NV12C) {
			y_ubwc_plane = MSM_MEDIA_ALIGN(y_stride * y_sclines, 4096);
			uv_ubwc_plane = MSM_MEDIA_ALIGN(uv_stride * uv_sclines, 4096);
			y_meta_scanlines =
				VIDEO_Y_META_SCANLINES(v4l2_fmt, pix_height);
			y_meta_plane = MSM_MEDIA_ALIGN(
				y_meta_stride * y_meta_scanlines, 4096);
			uv_meta_scanlines =
				VIDEO_UV_META_SCANLINES(v4l2_fmt, pix_height);
			uv_meta_plane = MSM_MEDIA_ALIGN(uv_meta_stride *
				uv_meta_scanlines, 4096);
			size = (y_ubwc_plane + uv_ubwc_plane + y_meta_plane +
				uv_meta_plane);
		} else {
			if (pix_width <= INTERLACE_WIDTH_MAX &&
				pix_height <= INTERLACE_HEIGHT_MAX &&
				(pix_height * pix_width) / 256 <= INTERLACE_MB_PER_FRAME_MAX) {
				y_sclines =
					VIDEO_Y_SCANLINES(v4l2_fmt, (pix_height+1)>>1);
				y_ubwc_plane =
					MSM_MEDIA_ALIGN(y_stride * y_sclines, 4096);
				uv_sclines =
					VIDEO_UV_SCANLINES(v4l2_fmt, (pix_height+1)>>1);
				uv_ubwc_plane =
					MSM_MEDIA_ALIGN(uv_stride * uv_sclines, 4096);
				y_meta_scanlines =
				VIDEO_Y_META_SCANLINES(v4l2_fmt, (pix_height+1)>>1);
				y_meta_plane = MSM_MEDIA_ALIGN(
					y_meta_stride * y_meta_scanlines, 4096);
				uv_meta_scanlines =
				VIDEO_UV_META_SCANLINES(v4l2_fmt, (pix_height+1)>>1);
				uv_meta_plane = MSM_MEDIA_ALIGN(uv_meta_stride *
					uv_meta_scanlines, 4096);
				size = (y_ubwc_plane + uv_ubwc_plane + y_meta_plane +
					uv_meta_plane)*2;
			} else {
				y_sclines = VIDEO_Y_SCANLINES(v4l2_fmt, pix_height);
				y_ubwc_plane =
					MSM_MEDIA_ALIGN(y_stride * y_sclines, 4096);
				uv_sclines = VIDEO_UV_SCANLINES(v4l2_fmt, pix_height);
				uv_ubwc_plane =
					MSM_MEDIA_ALIGN(uv_stride * uv_sclines, 4096);
				y_meta_scanlines =
					VIDEO_Y_META_SCANLINES(v4l2_fmt, pix_height);
				y_meta_plane = MSM_MEDIA_ALIGN(
					y_meta_stride * y_meta_scanlines, 4096);
				uv_meta_scanlines =
					VIDEO_UV_META_SCANLINES(v4l2_fmt, pix_height);
				uv_meta_plane = MSM_MEDIA_ALIGN(uv_meta_stride *
					uv_meta_scanlines, 4096);
				size = (y_ubwc_plane + uv_ubwc_plane + y_meta_plane +
					uv_meta_plane);
			}
		}
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		y_ubwc_plane = MSM_MEDIA_ALIGN(y_stride * y_sclines, 4096);
		uv_ubwc_plane = MSM_MEDIA_ALIGN(uv_stride * uv_sclines, 4096);
		y_meta_stride = VIDEO_Y_META_STRIDE(v4l2_fmt, pix_width);
		y_meta_scanlines = VIDEO_Y_META_SCANLINES(v4l2_fmt, pix_height);
		y_meta_plane = MSM_MEDIA_ALIGN(
			y_meta_stride * y_meta_scanlines, 4096);
		uv_meta_stride = VIDEO_UV_META_STRIDE(v4l2_fmt, pix_width);
		uv_meta_scanlines = VIDEO_UV_META_SCANLINES(v4l2_fmt, pix_height);
		uv_meta_plane = MSM_MEDIA_ALIGN(uv_meta_stride *
			uv_meta_scanlines, 4096);

		size = y_ubwc_plane + uv_ubwc_plane + y_meta_plane +
			uv_meta_plane;
		break;
	case V4L2_PIX_FMT_VIDC_ARGB32C:
		rgb_ubwc_plane = MSM_MEDIA_ALIGN(rgb_stride * rgb_scanlines,
			4096);
		rgb_meta_stride = VIDEO_RGB_META_STRIDE(v4l2_fmt, pix_width);
		rgb_meta_scanlines = VIDEO_RGB_META_SCANLINES(v4l2_fmt,
			pix_height);
		rgb_meta_plane = MSM_MEDIA_ALIGN(rgb_meta_stride *
			rgb_meta_scanlines, 4096);
		size = rgb_ubwc_plane + rgb_meta_plane;
		break;
	case V4L2_PIX_FMT_RGBA32:
		rgb_plane = MSM_MEDIA_ALIGN(rgb_stride * rgb_scanlines, 4096);
		size = rgb_plane;
		break;
	default:
		break;
	}

invalid_input:
	size = MSM_MEDIA_ALIGN(size, 4096);
	return size;
}

#endif

/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __HFI_BUFFER_AR50LT__
#define __HFI_BUFFER_AR50LT__

#include <linux/types.h>
#include "hfi_property.h"

typedef u8 HFI_U8;
typedef s8 HFI_S8;
typedef u16 HFI_U16;
typedef s16 HFI_S16;
typedef u32 HFI_U32;
typedef s32 HFI_S32;
typedef u64 HFI_U64;
typedef u32 HFI_BOOL;

#ifndef MIN
#define  MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define  MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define HFI_ALIGNMENT_4096 (4096)

#define BUF_SIZE_ALIGN_16 (16)
#define BUF_SIZE_ALIGN_32 (32)
#define BUF_SIZE_ALIGN_64 (64)
#define BUF_SIZE_ALIGN_128 (128)
#define BUF_SIZE_ALIGN_256 (256)
#define BUF_SIZE_ALIGN_512 (512)
#define BUF_SIZE_ALIGN_4096 (4096)

#define HFI_ALIGN(a, b) (((b) & ((b) - 1)) ? (((a) + (b) - 1) / \
	(b) * (b)) : (((a) + (b) - 1) & (~((b) - 1))))

#define HFI_WORKMODE_1 1
#define HFI_WORKMODE_2 2

#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_HEIGHT (8)
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_WIDTH (32)
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_HEIGHT (8)
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_WIDTH (16)

#define HFI_UBWC_CALC_METADATA_PLANE_STRIDE(metadata_stride, frame_width,\
	metadata_stride_multiple, tile_width_in_pels) \
	(metadata_stride = HFI_ALIGN(((frame_width + (tile_width_in_pels - 1)) /\
	tile_width_in_pels), metadata_stride_multiple))

#define HFI_UBWC_METADATA_PLANE_BUFHEIGHT(metadata_buf_height, frame_height, \
	metadata_height_multiple, tile_height_in_pels) \
	(metadata_buf_height = HFI_ALIGN(((frame_height + \
	(tile_height_in_pels - 1)) / tile_height_in_pels), \
	metadata_height_multiple))

#define HFI_UBWC_UV_METADATA_PLANE_STRIDE(metadata_stride, frame_width, \
	metadata_stride_multiple, tile_width_in_pels) \
	(metadata_stride = HFI_ALIGN(((((frame_width + 1) >> 1) +\
	(tile_width_in_pels - 1)) / tile_width_in_pels), \
	metadata_stride_multiple))

#define HFI_UBWC_UV_METADATA_PLANE_BUFHEIGHT(metadata_buf_height, frame_height,\
	metadata_height_multiple, tile_height_in_pels) \
	(metadata_buf_height = HFI_ALIGN(((((frame_height + 1) >> 1) + \
	(tile_height_in_pels - 1)) / tile_height_in_pels), \
	metadata_height_multiple))

#define HFI_UBWC_METADATA_PLANE_BUFFER_SIZE(buffer_size, _metadata_tride, \
					_metadata_buf_height) \
	(buffer_size = HFI_ALIGN(_metadata_tride * _metadata_buf_height, \
					HFI_ALIGNMENT_4096))

#define BUFFER_ALIGNMENT_512_BYTES 512
#define BUFFER_ALIGNMENT_256_BYTES 256
#define BUFFER_ALIGNMENT_128_BYTES 128
#define BUFFER_ALIGNMENT_64_BYTES 64
#define BUFFER_ALIGNMENT_32_BYTES 32
#define BUFFER_ALIGNMENT_16_BYTES 16
#define BUFFER_ALIGNMENT_8_BYTES 8
#define BUFFER_ALIGNMENT_4_BYTES 4

#define VENUS_DMA_ALIGNMENT BUFFER_ALIGNMENT_256_BYTES

#define MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE 64
#define MAX_FE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE 64
#define MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE 64
#define MAX_FE_NBR_DATA_LUMA_LINE_BUFFER_SIZE 10
#define MAX_FE_NBR_DATA_CB_LINE_BUFFER_SIZE 5
#define MAX_FE_NBR_DATA_CR_LINE_BUFFER_SIZE 5

#define MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE (32 / 8)
#define MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE (16 / 8)
#define MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE (8 / 8)

#define MAX_PE_NBR_DATA_LCU64_LINE_BUFFER_SIZE 176
#define MAX_PE_NBR_DATA_LCU32_LINE_BUFFER_SIZE 176
#define MAX_PE_NBR_DATA_LCU16_LINE_BUFFER_SIZE 176

#define MAX_TILE_COLUMNS 32

#define HFI_NV12_IL_CALC_Y_STRIDE(stride, frame_width, stride_multiple) \
		(stride = HFI_ALIGN(frame_width, stride_multiple))

#define HFI_NV12_IL_CALC_Y_BUFHEIGHT(buf_height, frame_height, \
		min_buf_height_multiple) (buf_height = HFI_ALIGN(frame_height, \
		min_buf_height_multiple))

#define HFI_NV12_IL_CALC_UV_STRIDE(stride, frame_width, stride_multiple) \
		(stride = HFI_ALIGN(frame_width, stride_multiple))

#define HFI_NV12_IL_CALC_UV_BUFHEIGHT(buf_height, frame_height, \
		min_buf_height_multiple) (buf_height = HFI_ALIGN(((frame_height + 1) \
		 >> 1), min_buf_height_multiple))

#define HFI_NV12_IL_CALC_BUF_SIZE(buf_size, y_bufSize, y_stride, y_buf_height, \
		uv_buf_size, uv_stride, uv_buf_height) \
	do { \
		y_bufSize = (y_stride * y_buf_height); \
		uv_buf_size = (uv_stride * uv_buf_height); \
		buf_size = HFI_ALIGN(y_bufSize + uv_buf_size, HFI_ALIGNMENT_4096); \
	} while (0)

#define HFI_NV12_UBWC_IL_CALC_Y_BUF_SIZE(y_bufSize, y_stride, y_buf_height) \
	(y_bufSize = HFI_ALIGN(y_stride * y_buf_height, HFI_ALIGNMENT_4096))

#define HFI_NV12_UBWC_IL_CALC_UV_BUF_SIZE(uv_buf_size, \
	uv_stride, uv_buf_height) \
	(uv_buf_size = HFI_ALIGN(uv_stride * uv_buf_height, HFI_ALIGNMENT_4096))

#define HFI_NV12_UBWC_IL_CALC_BUF_SIZE_V2(buf_size,\
	frame_width, frame_height, y_stride_multiple,\
	y_buffer_height_multiple, uv_stride_multiple, \
	uv_buffer_height_multiple, y_metadata_stride_multiple, \
	y_metadata_buffer_height_multiple, \
	uv_metadata_stride_multiple, uv_metadata_buffer_height_multiple) \
	do { \
		HFI_U32 y_buf_size, uv_buf_size, y_meta_size, uv_meta_size;   \
		HFI_U32 stride, _height; \
		HFI_U32 half_height = (frame_height + 1) >> 1; \
		HFI_NV12_IL_CALC_Y_STRIDE(stride, frame_width,\
					y_stride_multiple); \
		HFI_NV12_IL_CALC_Y_BUFHEIGHT(_height, half_height,\
					y_buffer_height_multiple); \
		HFI_NV12_UBWC_IL_CALC_Y_BUF_SIZE(y_buf_size, stride, _height);\
		HFI_NV12_IL_CALC_UV_STRIDE(stride, frame_width, \
					uv_stride_multiple); \
		HFI_NV12_IL_CALC_UV_BUFHEIGHT(_height, half_height, \
					uv_buffer_height_multiple); \
		HFI_NV12_UBWC_IL_CALC_UV_BUF_SIZE(uv_buf_size, stride, _height);\
		HFI_UBWC_CALC_METADATA_PLANE_STRIDE(stride, frame_width,\
				y_metadata_stride_multiple, \
			HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_WIDTH);\
		HFI_UBWC_METADATA_PLANE_BUFHEIGHT(_height, half_height, \
				y_metadata_buffer_height_multiple,\
			HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_HEIGHT);\
		HFI_UBWC_METADATA_PLANE_BUFFER_SIZE(y_meta_size, stride, \
				_height);    \
		HFI_UBWC_UV_METADATA_PLANE_STRIDE(stride, frame_width,\
				uv_metadata_stride_multiple, \
			HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_WIDTH); \
		HFI_UBWC_UV_METADATA_PLANE_BUFHEIGHT(_height, half_height,\
				uv_metadata_buffer_height_multiple,\
			HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_HEIGHT);\
		HFI_UBWC_METADATA_PLANE_BUFFER_SIZE(uv_meta_size, stride, \
				 _height); \
		buf_size = (y_buf_size + uv_buf_size + y_meta_size + \
			uv_meta_size) << 1;\
	} while (0)

#define SIZE_VPSS_LB(Size, frame_width, frame_height, num_vpp_pipes) \
	do { \
		HFI_U32 vpss_4tap_top_buffer_size, vpss_div2_top_buffer_size,\
		vpss_4tap_left_buffer_size, vpss_div2_left_buffer_size; \
		vpss_4tap_top_buffer_size = vpss_div2_top_buffer_size = \
		vpss_4tap_left_buffer_size = vpss_div2_left_buffer_size = 0;  \
		Size = num_vpp_pipes * 2 * (vpss_4tap_top_buffer_size + \
		vpss_div2_top_buffer_size) + \
		2 * (vpss_4tap_left_buffer_size + vpss_div2_left_buffer_size); \
	} while (0)

#define VPP_CMD_MAX_SIZE (1 << 20)
#define NUM_HW_PIC_BUF 32
#define BIN_BUFFER_THRESHOLD (1280 * 736)
#define H264D_MAX_SLICE 1800
#define SIZE_H264D_BUFTAB_T (256)
#define SIZE_H264D_HW_PIC_T (1 << 11)
#define SIZE_H264D_BSE_CMD_PER_BUF (32 * 4)
#define SIZE_H264D_VPP_CMD_PER_BUF (512)

#define SIZE_H264D_LB_FE_TOP_DATA(frame_width, frame_height) \
	(16 * HFI_ALIGN(frame_width, 16) * 2)

#define SIZE_H264D_LB_FE_TOP_CTRL(frame_width, frame_height) \
	(16 * ((frame_width + 15) >> 4))

#define SIZE_H264D_LB_FE_LEFT_CTRL(frame_width, frame_height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * ((frame_height + 15) >> 4))

#define SIZE_H264D_LB_SE_TOP_CTRL(frame_width, frame_height) \
	(MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * ((frame_width + 15) >> 4))

#define SIZE_H264D_LB_SE_LEFT_CTRL(frame_width, frame_height) \
	(MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * ((frame_height + 15) >> 4))

#define SIZE_H264D_LB_PE_TOP_DATA(frame_width, frame_height) \
	(64 *  ((frame_width + 15) >> 4))

#define SIZE_H264D_LB_VSP_TOP(frame_width, frame_height) \
	((((frame_width + 15) >> 4) << 7))

#define SIZE_H264D_LB_RECON_DMA_METADATA_WR(frame_width, frame_height) \
	(HFI_ALIGN(frame_height, 16) * 32)

#define SIZE_H264D_QP(frame_width, frame_height) \
	(((frame_width + 63) >> 6) * ((frame_height + 63) >> 6) * 128)

#define SIZE_HW_PIC(size_per_buf) \
	(NUM_HW_PIC_BUF * size_per_buf)

#define SIZE_H264D_BSE_CMD_BUF(_size, frame_width, frame_height) \
	do { \
		HFI_U32 _height = HFI_ALIGN(frame_height, \
				BUFFER_ALIGNMENT_32_BYTES);  \
		_size = MIN((((_height + 15) >> 4) * 3 * 4), H264D_MAX_SLICE) *\
					  SIZE_H264D_BSE_CMD_PER_BUF; \
	} while (0)

#define SIZE_H264D_VPP_CMD_BUF(_size, frame_width, frame_height)    \
	do { \
		HFI_U32 _height = HFI_ALIGN(frame_height, \
				BUFFER_ALIGNMENT_32_BYTES); \
		_size = MIN((((_height + 15) >> 4) * 3 * 4), H264D_MAX_SLICE) * \
					SIZE_H264D_VPP_CMD_PER_BUF; \
		if (_size > VPP_CMD_MAX_SIZE) \
			_size = VPP_CMD_MAX_SIZE; \
	} while (0)

#define HFI_BUFFER_COMV_H264D(coMV_size, frame_width, \
			frame_height, _yuv_bufcount_min) \
	do { \
		HFI_U32 frame_width_in_mbs = ((frame_width + 15) >> 4); \
		HFI_U32 frame_height_in_mbs = ((frame_height + 15) >> 4); \
		HFI_U32 col_mv_aligned_width = (frame_width_in_mbs << 6); \
		HFI_U32 col_zero_aligned_width = (frame_width_in_mbs << 2); \
		HFI_U32 col_zero_size = 0, size_colloc = 0; \
		col_mv_aligned_width = HFI_ALIGN(col_mv_aligned_width, \
					BUFFER_ALIGNMENT_16_BYTES); \
		col_zero_aligned_width = HFI_ALIGN(col_zero_aligned_width, \
					BUFFER_ALIGNMENT_16_BYTES); \
		col_zero_size = col_zero_aligned_width * \
					((frame_height_in_mbs + 1) >> 1); \
		col_zero_size = HFI_ALIGN(col_zero_size, \
				BUFFER_ALIGNMENT_64_BYTES);  \
		col_zero_size <<= 1; \
		col_zero_size = HFI_ALIGN(col_zero_size, \
				BUFFER_ALIGNMENT_512_BYTES); \
		size_colloc = col_mv_aligned_width * ((frame_height_in_mbs + \
				1) >> 1); \
		size_colloc = HFI_ALIGN(size_colloc, \
				BUFFER_ALIGNMENT_64_BYTES); \
		size_colloc <<= 1; \
		size_colloc = HFI_ALIGN(size_colloc, \
				BUFFER_ALIGNMENT_512_BYTES); \
		size_colloc += (col_zero_size + SIZE_H264D_BUFTAB_T * 2); \
		coMV_size = size_colloc * (_yuv_bufcount_min); \
		coMV_size += BUFFER_ALIGNMENT_512_BYTES; \
	} while (0)

#define HFI_BUFFER_NON_COMV_H264D(_size, frame_width, frame_height, \
								num_vpp_pipes) \
	do { \
		HFI_U32 _size_bse, _size_vpp; \
		SIZE_H264D_BSE_CMD_BUF(_size_bse, frame_width, frame_height); \
		SIZE_H264D_VPP_CMD_BUF(_size_vpp, frame_width, frame_height); \
		_size = HFI_ALIGN(_size_bse, VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(_size_vpp, VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_HW_PIC(SIZE_H264D_HW_PIC_T), \
			VENUS_DMA_ALIGNMENT); \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
	} while (0)

#define HFI_BUFFER_LINE_H264D(_size, frame_width, frame_height, \
								is_opb, num_vpp_pipes)            \
	do { \
		HFI_U32 vpss_lb_size = 0; \
		num_vpp_pipes = 1; \
		_size = HFI_ALIGN(SIZE_H264D_LB_FE_TOP_DATA(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H264D_LB_FE_TOP_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H264D_LB_FE_LEFT_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
			HFI_ALIGN(SIZE_H264D_LB_SE_TOP_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H264D_LB_SE_LEFT_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) * \
			num_vpp_pipes + \
			HFI_ALIGN(SIZE_H264D_LB_PE_TOP_DATA(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H264D_LB_VSP_TOP(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H264D_LB_RECON_DMA_METADATA_WR \
			(frame_width, frame_height), VENUS_DMA_ALIGNMENT) * 2 + \
			HFI_ALIGN(SIZE_H264D_QP(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT); \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
		if (is_opb) { \
			SIZE_VPSS_LB(vpss_lb_size, frame_width, frame_height, \
						num_vpp_pipes); \
		} \
		_size = HFI_ALIGN((_size + vpss_lb_size), \
			VENUS_DMA_ALIGNMENT); \
	} while (0)

#define H264_CABAC_HDR_RATIO_HD_TOT (3 / 5)  /* 0.6 */
#define H264_CABAC_RES_RATIO_HD_TOT (3 / 2)  /* 1.5 */

#define H264_CABAC_HDR_RATIO_SM_TOT 1  /* 1.0 */
#define H264_CABAC_RES_RATIO_SM_TOT 2  /* 2.0 */

#define SIZE_H264D_HW_BIN_BUFFER(_size, frame_width, frame_height, \
				delay, num_vpp_pipes) \
	do { \
		HFI_U32 size_yuv, size_bin_hdr, size_bin_res; \
		size_yuv = ((frame_width * frame_height) * 3) >> 1; \
		if (size_yuv <= 1920 * 1088 * 3 / 2) { \
			size_bin_hdr = size_yuv * H264_CABAC_HDR_RATIO_SM_TOT; \
			size_bin_res = size_yuv * H264_CABAC_RES_RATIO_SM_TOT; \
		} \
		else { \
			size_bin_hdr = (size_yuv * 3) / 5; \
			size_bin_res = (size_yuv * 3) / 2; \
		} \
		size_bin_hdr = HFI_ALIGN(size_bin_hdr, VENUS_DMA_ALIGNMENT); \
		size_bin_res = HFI_ALIGN(size_bin_res, VENUS_DMA_ALIGNMENT); \
		_size = size_bin_hdr + size_bin_res; \
	} while (0)

#define HFI_BUFFER_BIN_H264D(_size, frame_width, frame_height, \
			delay, num_vpp_pipes) \
	do { \
		HFI_U32 n_aligned_w = HFI_ALIGN(frame_width, \
				BUFFER_ALIGNMENT_16_BYTES);\
		HFI_U32 n_aligned_h = HFI_ALIGN(frame_height, \
				BUFFER_ALIGNMENT_16_BYTES); \
		SIZE_H264D_HW_BIN_BUFFER(_size, n_aligned_w, \
			n_aligned_h, delay, num_vpp_pipes); \
	} while (0)

#define NUM_SLIST_BUF_H264 (256 + 32)
#define SIZE_SLIST_BUF_H264 (512)
#define SIZE_SEI_USERDATA (4096)
#define HFI_BUFFER_PERSIST_H264D(_size) \
	(_size = HFI_ALIGN((SIZE_SLIST_BUF_H264 * NUM_SLIST_BUF_H264), \
	 VENUS_DMA_ALIGNMENT))

#define LCU_MAX_SIZE_PELS 64
#define LCU_MIN_SIZE_PELS 16

#define H265D_MAX_SLICE 600
#define SIZE_H265D_HW_PIC_T SIZE_H264D_HW_PIC_T
#define SIZE_H265D_BSE_CMD_PER_BUF (16 * sizeof(HFI_U32))
#define SIZE_H265D_VPP_CMD_PER_BUF (12*16)

#define SIZE_H265D_LB_FE_TOP_DATA(frame_width, frame_height) \
	(HFI_ALIGN(MAX_FE_NBR_DATA_LUMA_LINE_BUFFER_SIZE * \
	(HFI_ALIGN(frame_width, 64) + 8), VENUS_DMA_ALIGNMENT) * 2)

#define SIZE_H265D_LB_FE_TOP_CTRL(frame_width, frame_height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	(HFI_ALIGN(frame_width, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_FE_LEFT_CTRL(frame_width, frame_height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	(HFI_ALIGN(frame_height, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_SE_TOP_CTRL(frame_width, frame_height) \
	((LCU_MAX_SIZE_PELS / 8 * (128 / 8)) * ((frame_width + 15) >> 4))

#define SIZE_H265D_LB_SE_LEFT_CTRL(frame_width, frame_height)    \
	(MAX(((frame_height + 16 - 1) / 8) * \
		MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE,     \
	MAX(((frame_height + 32 - 1) / 8) * \
		MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((frame_height + 64 - 1) / 8) * \
		MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE)))

#define SIZE_H265D_LB_PE_TOP_DATA(frame_width, frame_height) \
	(MAX_PE_NBR_DATA_LCU64_LINE_BUFFER_SIZE * (HFI_ALIGN(frame_width, \
	LCU_MIN_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_VSP_TOP(frame_width, frame_height) \
	(((frame_width + 63) >> 6) * 128)

#define SIZE_H265D_LB_VSP_LEFT(frame_width, frame_height) \
	(((frame_height + 63) >> 6) * 128)

#define SIZE_H265D_LB_RECON_DMA_METADATA_WR(frame_width, frame_height) \
	SIZE_H264D_LB_RECON_DMA_METADATA_WR(frame_width, frame_height)

#define SIZE_H265D_QP(frame_width, frame_height) \
		SIZE_H264D_QP(frame_width, frame_height)

#define SIZE_H265D_BSE_CMD_BUF(_size, frame_width, frame_height)\
	do { \
		_size = HFI_ALIGN(((HFI_ALIGN(frame_width, \
		LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) * \
		(HFI_ALIGN(frame_height, LCU_MAX_SIZE_PELS) /\
		LCU_MIN_SIZE_PELS)) * NUM_HW_PIC_BUF, VENUS_DMA_ALIGNMENT);  \
		_size = MIN(_size, H265D_MAX_SLICE + 1);          \
		_size = 2 * _size * SIZE_H265D_BSE_CMD_PER_BUF;     \
	} while (0)

#define SIZE_H265D_VPP_CMD_BUF(_size, frame_width, frame_height)  \
	do { \
		_size = HFI_ALIGN(((HFI_ALIGN(frame_width, LCU_MAX_SIZE_PELS) /\
		LCU_MIN_SIZE_PELS) * (HFI_ALIGN(frame_height, \
		LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS)) * \
		NUM_HW_PIC_BUF, VENUS_DMA_ALIGNMENT);  \
		_size = MIN(_size, H265D_MAX_SLICE + 1);    \
		_size = HFI_ALIGN(_size, 4);  \
		_size = 2 * _size * SIZE_H265D_VPP_CMD_PER_BUF; \
		if (_size > VPP_CMD_MAX_SIZE)  { \
			_size = VPP_CMD_MAX_SIZE;   \
		} \
	} while (0)

#define HFI_BUFFER_COMV_H265D(_size, frame_width, frame_height, \
							_yuv_bufcount_min) \
	do { \
		_size = HFI_ALIGN(((((frame_width + 15) >> 4) * \
			((frame_height + 15) >> 4)) << 8), \
			BUFFER_ALIGNMENT_512_BYTES); \
		_size *= _yuv_bufcount_min; \
		_size += BUFFER_ALIGNMENT_512_BYTES; \
	} while (0)

#define HFI_BUFFER_NON_COMV_H265D(_size, frame_width, frame_height, \
				num_vpp_pipes) \
	do { \
		HFI_U32 _size_bse, _size_vpp; \
		SIZE_H265D_BSE_CMD_BUF(_size_bse, frame_width, \
		frame_height); \
		SIZE_H265D_VPP_CMD_BUF(_size_vpp, frame_width, \
		frame_height);  \
		_size = HFI_ALIGN(_size_bse, VENUS_DMA_ALIGNMENT) +     \
			HFI_ALIGN(_size_vpp, VENUS_DMA_ALIGNMENT) +     \
			HFI_ALIGN(2 * sizeof(HFI_U16) *           \
			(HFI_ALIGN(frame_width, LCU_MAX_SIZE_PELS) / \
			LCU_MIN_SIZE_PELS) * (HFI_ALIGN(frame_height, \
			LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS), \
			VENUS_DMA_ALIGNMENT) +                \
			HFI_ALIGN(SIZE_HW_PIC(SIZE_H265D_HW_PIC_T), \
			VENUS_DMA_ALIGNMENT);           \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT);  \
	} while (0)

#define HFI_BUFFER_LINE_H265D(_size, frame_width, frame_height, \
			is_opb, num_vpp_pipes) \
	do { \
		HFI_U32 vpss_lb_size = 0; \
		num_vpp_pipes = 1; \
		_size = HFI_ALIGN(SIZE_H265D_LB_FE_TOP_DATA(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H265D_LB_FE_TOP_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H265D_LB_FE_LEFT_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
			HFI_ALIGN(SIZE_H265D_LB_SE_LEFT_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
			HFI_ALIGN(SIZE_H265D_LB_SE_TOP_CTRL(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H265D_LB_PE_TOP_DATA(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) + \
			HFI_ALIGN(SIZE_H265D_LB_VSP_TOP(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) +        \
			HFI_ALIGN(SIZE_H265D_LB_VSP_LEFT(frame_width, \
			frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes; \
			HFI_ALIGN(SIZE_H265D_LB_RECON_DMA_METADATA_WR\
			(frame_width, frame_height), \
			VENUS_DMA_ALIGNMENT) * 4 +  \
			HFI_ALIGN(SIZE_H265D_QP(frame_width, frame_height),\
			VENUS_DMA_ALIGNMENT);           \
		if (is_opb) {  \
			SIZE_VPSS_LB(vpss_lb_size, frame_width, frame_height,\
						 num_vpp_pipes); \
		} \
		_size = HFI_ALIGN((_size + vpss_lb_size), \
			VENUS_DMA_ALIGNMENT); \
	} while (0)

/* bin buffer ratios for SMall res, these will only be used for verification purpose on EMUs */
#define H265_CABAC_HDR_RATIO_SM_TOT 1  /* 1.0 */
#define H265_CABAC_RES_RATIO_SM_TOT 6  /* 6.0 */
#define H265_CABAC_HDR_RATIO_HD_TOT (41 / 50) /* 0.82 */
#define H265_CABAC_RES_RATIO_HD_TOT (59 / 50) /* 1.18 */

#define SIZE_H265D_HW_BIN_BUFFER(_size, frame_width, frame_height, \
				delay, num_vpp_pipes)    \
	do { \
		HFI_U32 size_yuv, size_bin_hdr, size_bin_res;     \
		size_yuv = ((frame_width * frame_height) * 3) >> 1; \
		if (size_yuv <= ((BIN_BUFFER_THRESHOLD * 3) >> 1)) { \
			size_bin_hdr = size_yuv * H265_CABAC_HDR_RATIO_SM_TOT; \
			size_bin_res = size_yuv * H265_CABAC_RES_RATIO_SM_TOT; \
		}   \
		else { \
			size_bin_hdr = (size_yuv * 41)/50; \
			size_bin_res = (size_yuv * 59)/50; \
		} \
		size_bin_hdr = HFI_ALIGN(size_bin_hdr, VENUS_DMA_ALIGNMENT); \
		size_bin_res = HFI_ALIGN(size_bin_res, VENUS_DMA_ALIGNMENT); \
		_size = size_bin_hdr + size_bin_res; \
	} while (0)

#define HFI_BUFFER_BIN_H265D(_size, frame_width, frame_height, \
				delay, num_vpp_pipes) \
	do {  \
		HFI_U32 n_aligned_w = HFI_ALIGN(frame_width, \
				BUFFER_ALIGNMENT_16_BYTES); \
		HFI_U32 n_aligned_h = HFI_ALIGN(frame_height, \
				BUFFER_ALIGNMENT_16_BYTES); \
		SIZE_H265D_HW_BIN_BUFFER(_size, n_aligned_w, \
			n_aligned_h, delay, num_vpp_pipes); \
	} while (0)

#define SIZE_SLIST_BUF_H265 (1 << 10)
#define NUM_SLIST_BUF_H265 (80 + 20)
#define H265_NUM_TILE_COL 32
#define H265_NUM_TILE_ROW 128
#define H265_NUM_TILE (H265_NUM_TILE_ROW * H265_NUM_TILE_COL + 1)
#define HFI_BUFFER_PERSIST_H265D(_size) \
	(_size = HFI_ALIGN((SIZE_SLIST_BUF_H265 * NUM_SLIST_BUF_H265 + \
	H265_NUM_TILE * sizeof(HFI_U32) + NUM_HW_PIC_BUF * SIZE_SEI_USERDATA),\
	VENUS_DMA_ALIGNMENT))

#define SIZE_VPXD_LB_FE_LEFT_CTRL(frame_width, frame_height)   \
	MAX(((frame_height + 15) >> 4) * \
	MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE, \
	MAX(((frame_height + 31) >> 5) * \
	MAX_FE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((frame_height + 63) >> 6) * MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE))
#define SIZE_VPXD_LB_FE_TOP_CTRL(frame_width, frame_height) \
	(((HFI_ALIGN(frame_width, 64) + 8) * 10 * 2))
#define SIZE_VPXD_LB_SE_TOP_CTRL(frame_width, frame_height) \
	(((frame_width + 15) >> 4) * MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE)
#define SIZE_VPXD_LB_SE_LEFT_CTRL(frame_width, frame_height)  \
	MAX(((frame_height + 15) >> 4) * \
	MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE,\
	MAX(((frame_height + 31) >> 5) * \
	MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((frame_height + 63) >> 6) * MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE))
#define SIZE_VPXD_LB_RECON_DMA_METADATA_WR(frame_width, frame_height) \
	HFI_ALIGN((HFI_ALIGN(frame_height, 8) / (4 / 2)) * 64,\
	BUFFER_ALIGNMENT_32_BYTES)
#define SIZE_MP2D_LB_FE_TOP_DATA(frame_width, frame_height) \
	((HFI_ALIGN(frame_width, 16) + 8) * 10 * 2)
#define SIZE_VP9D_LB_FE_TOP_DATA(frame_width, frame_height) \
	HFI_ALIGN((HFI_ALIGN(HFI_ALIGN(frame_width, 8), 64) + 8) * 10, VENUS_DMA_ALIGNMENT) +\
	HFI_ALIGN((HFI_ALIGN(HFI_ALIGN(frame_width, 8), 64) + 8) * 5, VENUS_DMA_ALIGNMENT) + \
	HFI_ALIGN((HFI_ALIGN(HFI_ALIGN(frame_width, 8), 64) + 8) * 5, VENUS_DMA_ALIGNMENT)
#define SIZE_MP2D_LB_PE_TOP_DATA(frame_width, frame_height) \
	((HFI_ALIGN(frame_width, 16) >> 4) * 64)
#define SIZE_VP9D_LB_PE_TOP_DATA(frame_width, frame_height) \
	((HFI_ALIGN(HFI_ALIGN(frame_width, 8), 64) >> 6) * 176)
#define SIZE_MP2D_LB_VSP_TOP(frame_width, frame_height) \
	(((HFI_ALIGN(frame_width, 16) >> 4) * 64 / 2) + 256)
#define SIZE_VP9D_LB_VSP_TOP(frame_width, frame_height) \
	((((HFI_ALIGN(HFI_ALIGN(frame_width, 8), 64) >> 6) * 64 * 8) + 256))

#define HFI_AR50LT_VP9D_COMV_SIZE \
	((((8192 + 63) >> 6) * ((4320 + 63) >> 6) * 8 * 8 * 2 * 8))

#define SIZE_VP9D_QP(frame_width, frame_height) \
	SIZE_H264D_QP(frame_width, frame_height)

#define HFI_AR50LT_VP9D_LB_SIZE(_size, frame_width, frame_height, num_vpp_pipes)\
		(_size = HFI_ALIGN(SIZE_VPXD_LB_FE_LEFT_CTRL(frame_width, \
		frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
		HFI_ALIGN(SIZE_VPXD_LB_SE_LEFT_CTRL(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
		HFI_ALIGN(SIZE_VP9D_LB_VSP_TOP(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_VPXD_LB_SE_TOP_CTRL(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_VP9D_LB_PE_TOP_DATA(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_VP9D_LB_FE_TOP_DATA(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT)) \

#define HFI_BUFFER_LINE_VP9D(_size, frame_width, frame_height, \
		_yuv_bufcount_min, is_opb, num_vpp_pipes) \
	do   {  \
		HFI_U32 _lb_size = 0;                  \
		HFI_U32 vpss_lb_size = 0;              \
		HFI_AR50LT_VP9D_LB_SIZE(_lb_size, frame_width, frame_height,\
			num_vpp_pipes);             \
		if (is_opb)  {  \
			SIZE_VPSS_LB(vpss_lb_size, frame_width, frame_height, \
			num_vpp_pipes); \
		} \
		_size = _lb_size + vpss_lb_size; \
	} while (0)

#define VPX_DECODER_FRAME_CONCURENCY_LVL (2)
#define VPX_DECODER_FRAME_BIN_HDR_BUDGET_RATIO (1 / 2)
#define VPX_DECODER_FRAME_BIN_RES_BUDGET_RATIO (3 / 2)
#define VPX_DECODER_FRAME_BIN_BUFFER_SIZE (1024 * 1024)// 8000000//(8*1000*10)//1000)
#define BIN_BUFFER_RATIO_SMALL_RES (6/5)

#define HFI_BUFFER_BIN_VP9D(_size, frame_width, frame_height, \
				num_vpp_pipes) \
	do { \
		HFI_U32 _size_yuv = HFI_ALIGN(frame_width, \
		BUFFER_ALIGNMENT_16_BYTES) *\
		HFI_ALIGN(frame_height, BUFFER_ALIGNMENT_16_BYTES) * 3 / 2;  \
		_size_yuv = HFI_ALIGN(_size_yuv, VENUS_DMA_ALIGNMENT); \
		_size = HFI_ALIGN(((((MAX(_size_yuv, VPX_DECODER_FRAME_BIN_BUFFER_SIZE)) * \
		6)/5) / num_vpp_pipes), VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(((MAX(_size_yuv, VPX_DECODER_FRAME_BIN_BUFFER_SIZE) * 4) \
		/ num_vpp_pipes), VENUS_DMA_ALIGNMENT);  \
		_size = _size * num_vpp_pipes; \
	} while (0)

#define VP9_NUM_FRAME_INFO_BUF 32
#define VP9_NUM_PROBABILITY_TABLE_BUF (VP9_NUM_FRAME_INFO_BUF + 4)
#define VP9_PROB_TABLE_SIZE (3840)

#define VP9_UDC_HEADER_BUF_SIZE (3 * 128)
#define MAX_SUPERFRAME_HEADER_LEN (34)
#define CCE_TILE_OFFSET_SIZE HFI_ALIGN(32 * 4 * 4, BUFFER_ALIGNMENT_32_BYTES)

#define HFI_BUFFER_PERSIST_VP9D(_size) \
	(_size = HFI_ALIGN(VP9_NUM_PROBABILITY_TABLE_BUF * VP9_PROB_TABLE_SIZE, \
	VENUS_DMA_ALIGNMENT) + HFI_ALIGN(HFI_AR50LT_VP9D_COMV_SIZE, \
	VENUS_DMA_ALIGNMENT) + HFI_ALIGN(MAX_SUPERFRAME_HEADER_LEN, \
	VENUS_DMA_ALIGNMENT))

#define HFI_BUFFER_LINE_MP2D(_size, frame_width, frame_height, \
_yuv_bufcount_min, is_opb, num_vpp_pipes)           \
	do {               \
		HFI_U32 vpss_lb_size = 0;     \
		_size = HFI_ALIGN(SIZE_VPXD_LB_FE_LEFT_CTRL(frame_width, \
		frame_height), VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
		HFI_ALIGN(SIZE_VPXD_LB_SE_LEFT_CTRL(frame_width, frame_height),\
		VENUS_DMA_ALIGNMENT) * num_vpp_pipes + \
		HFI_ALIGN(SIZE_MP2D_LB_VSP_TOP(frame_width, frame_height),\
		VENUS_DMA_ALIGNMENT) + HFI_ALIGN(SIZE_VPXD_LB_FE_TOP_CTRL\
		(frame_width, frame_height), VENUS_DMA_ALIGNMENT) + \
		2 * HFI_ALIGN(SIZE_VPXD_LB_RECON_DMA_METADATA_WR(frame_width,\
		frame_height), VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_VPXD_LB_SE_TOP_CTRL(frame_width, frame_height),\
		VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_MP2D_LB_PE_TOP_DATA(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT) + \
		HFI_ALIGN(SIZE_MP2D_LB_FE_TOP_DATA(frame_width, frame_height), \
		VENUS_DMA_ALIGNMENT); \
		if (is_opb) { \
			SIZE_VPSS_LB(vpss_lb_size, frame_width, frame_height, \
						num_vpp_pipes); \
		} \
		_size += vpss_lb_size; \
	} while (0)

#define HFI_BUFFER_BIN_MP2D(_size, frame_width, frame_height) 0

#define QMATRIX_SIZE (sizeof(HFI_U32) * 128 + 256)
#define MP2D_QPDUMP_SIZE 115200
#define HFI_BUFFER_PERSIST_MP2D(_size) \
	(_size = QMATRIX_SIZE + MP2D_QPDUMP_SIZE)

#define HFI_BUFFER_BITSTREAM_ENC(size, frame_width, frame_height, \
			rc_type, is_ten_bit) \
	do { \
		HFI_U32 aligned_width, aligned_height, bitstream_size; \
		aligned_width = HFI_ALIGN(frame_width, 32); \
		aligned_height = HFI_ALIGN(frame_height, 32); \
		bitstream_size = aligned_width * aligned_height * 3; \
		if (aligned_width * aligned_height > (4096 * 2176)) { \
			bitstream_size = (bitstream_size >> 3); \
		} \
		else if (bitstream_size > (1280 * 720)) { \
			bitstream_size = (bitstream_size >> 2); \
		} \
		else { \
			bitstream_size = (bitstream_size << 1);\
		} \
		if ((rc_type == HFI_RC_CQ) || (rc_type == HFI_RC_OFF))  { \
			bitstream_size = (bitstream_size << 1);\
		} \
		if (is_ten_bit) { \
			bitstream_size = (bitstream_size) + \
					(bitstream_size >> 2); \
		} \
		size = HFI_ALIGN(bitstream_size, HFI_ALIGNMENT_4096); \
	} while (0)

#define SIZE_ROI_METADATA_ENC(size_roi, frame_width, frame_height, lcu_size)\
	do { \
		HFI_U32 width_in_lcus = 0, height_in_lcus = 0, n_shift = 0;    \
		while (lcu_size && !(lcu_size & 0x1))  { \
			n_shift++;                          \
			lcu_size = lcu_size >> 1;          \
		}                                      \
		width_in_lcus = (frame_width + (lcu_size - 1)) >> n_shift; \
		height_in_lcus = (frame_height + (lcu_size - 1)) >> n_shift;  \
		size_roi = (((width_in_lcus + 7) >> 3) << 3) * \
					height_in_lcus * 2 + 256; \
	} while (0)

#define HFI_BUFFER_INPUT_METADATA_ENC(size, frame_width, frame_height, \
				is_roi_enabled, is_rpu_enabled, lcu_size) \
	do { \
		HFI_U32 roi_size = 0; \
		if (is_roi_enabled) { \
			SIZE_ROI_METADATA_ENC(roi_size, frame_width, \
				frame_height, lcu_size); \
		} \
		size = roi_size + 16384; \
		size = size + (is_rpu_enabled ? SIZE_DOLBY_RPU_METADATA : 0); \
		size = HFI_ALIGN(size, HFI_ALIGNMENT_4096); \
	} while (0)

#define HFI_BUFFER_INPUT_METADATA_H264E(size_metadata, frame_width, \
		frame_height, is_roi_enabled, is_rpu_enabled)    \
		HFI_BUFFER_INPUT_METADATA_ENC(size_metadata, frame_width, \
			frame_height, is_roi_enabled, is_rpu_enabled, 16) \

#define HFI_BUFFER_INPUT_METADATA_H265E(size_metadata, frame_width, \
		frame_height, is_roi_enabled, is_rpu_enabled)    \
		HFI_BUFFER_INPUT_METADATA_ENC(size_metadata, frame_width, \
			frame_height, is_roi_enabled, is_rpu_enabled, 32) \

#define HFI_BUFFER_ARP_ENC(size) \
	(size = 204800)

#define HFI_MAX_COL_FRAME 6
#define HFI_VENUS_VENC_TRE_WB_BUFF_SIZE (65 << 4) // bytes
#define HFI_VENUS_VENC_DB_LINE_BUFF_PER_MB 512
#define HFI_VENUS_VPPSG_MAX_REGISTERS 2048
#define HFI_VENUS_WIDTH_ALIGNMENT 128
#define HFI_VENUS_WIDTH_TEN_BIT_ALIGNMENT 192
#define HFI_VENUS_HEIGHT_ALIGNMENT 32
#define VENUS_METADATA_STRIDE_MULTIPLE 64
#define VENUS_METADATA_HEIGHT_MULTIPLE 16

#ifndef SYSTEM_LAL_TILE10
#define SYSTEM_LAL_TILE10 192
#endif

#define HFI_AR50LT_ENC_RECON_BUF_COUNT(num_recon, n_bframe, ltr_count, \
	_total_hp_layers, _total_hb_layers, hybrid_hp, codec_standard) \
	do { \
		HFI_U32 num_ref = 1; \
		if (n_bframe) \
			num_ref = 2; \
		if (_total_hp_layers > 1) { \
			if (hybrid_hp) \
				num_ref = (_total_hp_layers + 1) >> 1; \
			else if (codec_standard == HFI_CODEC_ENCODE_HEVC) \
				num_ref = (_total_hp_layers + 1) >> 1; \
			else if (codec_standard == HFI_CODEC_ENCODE_AVC && \
				_total_hp_layers < 4) \
				num_ref = (_total_hp_layers - 1);  \
			else \
				num_ref = _total_hp_layers; \
		} \
		if (ltr_count) \
			num_ref = num_ref + ltr_count; \
		if (_total_hb_layers > 1) { \
			if (codec_standard == HFI_CODEC_ENCODE_HEVC) \
				num_ref = (_total_hb_layers); \
			else if (codec_standard == HFI_CODEC_ENCODE_AVC) \
				num_ref = (1 << (_total_hb_layers - 2)) + 1; \
		} \
		num_recon = num_ref + 1; \
	} while (0)

#define SIZE_BIN_BITSTREAM_ENC(_size, rc_type, frame_width, frame_height, \
		work_mode, lcu_size) \
	do { \
		HFI_U32 size_aligned_width = 0, size_aligned_height = 0; \
		HFI_U32 bitstream_size_eval = 0; \
		size_aligned_width = HFI_ALIGN((frame_width), lcu_size); \
		size_aligned_height = HFI_ALIGN((frame_height), lcu_size); \
		if (work_mode == HFI_WORKMODE_2) { \
			if ((rc_type == HFI_RC_CQ) || (rc_type == HFI_RC_OFF)) { \
				bitstream_size_eval = (((size_aligned_width) * \
							(size_aligned_height) * 3) >> 1); \
			} \
			else { \
				bitstream_size_eval = ((size_aligned_width) * \
							(size_aligned_height) * 3); \
				if (rc_type == HFI_RC_LOSSLESS) { \
					bitstream_size_eval = (bitstream_size_eval * 3 >> 2); \
				} \
				else if ((size_aligned_width * size_aligned_height) > \
					(4096 * 2176)) { \
					bitstream_size_eval >>= 3; \
				} \
				else if ((size_aligned_width * size_aligned_height) > \
					(480 * 320)) { \
					bitstream_size_eval >>= 2; \
				} \
				if (lcu_size == 32) { \
					bitstream_size_eval = (bitstream_size_eval * 5 >> 2); \
				} \
			} \
		} \
		else { \
			bitstream_size_eval = size_aligned_width * \
					size_aligned_height * 3; \
		} \
		_size = HFI_ALIGN(bitstream_size_eval, VENUS_DMA_ALIGNMENT); \
	} while (0)

#define SIZE_ENC_SINGLE_PIPE(size, rc_type, bitbin_size, num_vpp_pipes, \
			frame_width, frame_height, lcu_size) \
	do { \
		HFI_U32 size_single_pipe_eval = 0, sao_bin_buffer_size = 0, \
			_padded_bin_sz = 0; \
		HFI_U32 size_aligned_width = 0, size_aligned_height = 0; \
		size_aligned_width = HFI_ALIGN((frame_width), lcu_size); \
		size_aligned_height = HFI_ALIGN((frame_height), lcu_size); \
		if ((size_aligned_width * size_aligned_height) > \
			(3840 * 2160)) { \
			size_single_pipe_eval = (bitbin_size / num_vpp_pipes); \
		} \
		else if (num_vpp_pipes > 2) { \
			size_single_pipe_eval = bitbin_size / 2; \
		} \
		else { \
			size_single_pipe_eval = bitbin_size; \
		} \
		if (rc_type == HFI_RC_LOSSLESS) { \
			size_single_pipe_eval = (size_single_pipe_eval << 1); \
		} \
		sao_bin_buffer_size = (64 * ((((frame_width) + \
				BUFFER_ALIGNMENT_32_BYTES) * ((frame_height) +\
				BUFFER_ALIGNMENT_32_BYTES)) >> 10)) + 384; \
		_padded_bin_sz = HFI_ALIGN(size_single_pipe_eval, \
				VENUS_DMA_ALIGNMENT);\
		size_single_pipe_eval = sao_bin_buffer_size + _padded_bin_sz; \
		size_single_pipe_eval = HFI_ALIGN(size_single_pipe_eval, \
				VENUS_DMA_ALIGNMENT); \
		size = size_single_pipe_eval; \
	} while (0)

#define HFI_BUFFER_BIN_ENC(_size, rc_type, frame_width, frame_height, lcu_size, \
				work_mode, num_vpp_pipes)           \
	do { \
		HFI_U32 bitstream_size = 0, total_bitbin_buffers = 0, \
			size_single_pipe = 0, bitbin_size = 0; \
		num_vpp_pipes = 1;\
		SIZE_BIN_BITSTREAM_ENC(bitstream_size, rc_type, frame_width, \
			frame_height, work_mode, lcu_size);         \
		if (work_mode == HFI_WORKMODE_2) { \
			total_bitbin_buffers = 3; \
			bitbin_size = bitstream_size * 17 / 10; \
			bitbin_size = HFI_ALIGN(bitbin_size, \
				VENUS_DMA_ALIGNMENT); \
		} \
		else if ((lcu_size == 16) || (num_vpp_pipes > 1)) { \
			total_bitbin_buffers = 1; \
			bitbin_size = bitstream_size; \
		} \
		if (total_bitbin_buffers > 0) { \
			SIZE_ENC_SINGLE_PIPE(size_single_pipe, rc_type, bitbin_size, \
				num_vpp_pipes, frame_width, frame_height, lcu_size); \
			bitbin_size = size_single_pipe * num_vpp_pipes; \
			_size = HFI_ALIGN(bitbin_size, VENUS_DMA_ALIGNMENT) * \
					total_bitbin_buffers + 512; \
		} \
		else { \
		/* Avoid 512 Bytes allocation in case of 1Pipe HEVC Direct Mode*/ \
			_size = 0; \
		} \
	} while (0)

#define HFI_BUFFER_BIN_H264E(_size, rc_type, frame_width, frame_height, \
				work_mode, num_vpp_pipes)    \
		HFI_BUFFER_BIN_ENC(_size, rc_type, frame_width, frame_height, 16, \
				work_mode, num_vpp_pipes) \

#define HFI_BUFFER_BIN_H265E(_size, rc_type, frame_width, frame_height, \
				work_mode, num_vpp_pipes)    \
		HFI_BUFFER_BIN_ENC(_size, rc_type, frame_width, frame_height, 32,\
				work_mode, num_vpp_pipes)  \

#define SIZE_ENC_SLICE_INFO_BUF(num_lcu_in_frame) HFI_ALIGN((256 + \
		(num_lcu_in_frame << 4)), VENUS_DMA_ALIGNMENT)
#define SIZE_LINE_BUF_CTRL(frame_width_coded) \
		HFI_ALIGN(frame_width_coded, VENUS_DMA_ALIGNMENT)
#define SIZE_LINE_BUF_CTRL_ID2(frame_width_coded) \
		HFI_ALIGN(frame_width_coded, VENUS_DMA_ALIGNMENT)

#define SIZE_LINEBUFF_DATA(_size, is_ten_bit, frame_width_coded) \
	(_size = is_ten_bit ? (((((10 * (frame_width_coded) +\
		 1024) + (VENUS_DMA_ALIGNMENT - 1)) & \
		 (~(VENUS_DMA_ALIGNMENT - 1))) * 1) + \
		(((((10 * (frame_width_coded) + 1024) >> 1) + \
		(VENUS_DMA_ALIGNMENT - 1)) & (~(VENUS_DMA_ALIGNMENT - 1))) * \
		2)) : (((((8 * (frame_width_coded) + 1024) + \
		(VENUS_DMA_ALIGNMENT - 1)) \
		& (~(VENUS_DMA_ALIGNMENT - 1))) * 1) + \
		(((((8 * (frame_width_coded) +\
		1024) >> 1) + (VENUS_DMA_ALIGNMENT - 1)) & \
		(~(VENUS_DMA_ALIGNMENT - 1))) * 2))) \

#define SIZE_LEFT_LINEBUFF_CTRL(_size, standard, frame_height_coded, \
				num_vpp_pipes_enc) \
	do { \
		_size = (standard == HFI_CODEC_ENCODE_HEVC) ? \
			(((frame_height_coded) + \
			31) / 32 * 3 * 16) : \
			(((frame_height_coded) + 15) / 16 * 4 * 16); \
		if ((num_vpp_pipes_enc) > 1) { \
			_size += BUFFER_ALIGNMENT_512_BYTES; \
			_size = HFI_ALIGN(_size, BUFFER_ALIGNMENT_512_BYTES) *\
				(num_vpp_pipes_enc); \
		} \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
	} while (0)

#define SIZE_LEFT_LINEBUFF_RECON_PIX(_size, is_ten_bit, frame_height_coded, \
				num_vpp_pipes_enc) \
		(_size = (((is_ten_bit + 1) * 2 * (frame_height_coded) + \
		VENUS_DMA_ALIGNMENT) + \
		(VENUS_DMA_ALIGNMENT << (num_vpp_pipes_enc - 1)) - 1) & \
		(~((VENUS_DMA_ALIGNMENT << (num_vpp_pipes_enc - 1)) - 1)) * 1) \

#define SIZE_TOP_LINEBUFF_CTRL_FE(_size, frame_width_coded, standard) \
	do { \
		_size = (standard == HFI_CODEC_ENCODE_HEVC) ? (64 * \
		((frame_width_coded) >> 5)) : (VENUS_DMA_ALIGNMENT + 16 * \
		((frame_width_coded) >> 4)); \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
	} while (0)

#define SIZE_LEFT_LINEBUFF_CTRL_FE(frame_height_coded, num_vpp_pipes_enc) \
	((((VENUS_DMA_ALIGNMENT + 64 * ((frame_height_coded) >> 4)) +         \
	   (VENUS_DMA_ALIGNMENT << (num_vpp_pipes_enc - 1)) - 1) &          \
	  (~((VENUS_DMA_ALIGNMENT << (num_vpp_pipes_enc - 1)) - 1)) * 1) *  \
	 num_vpp_pipes_enc)

#define SIZE_LEFT_LINEBUFF_METADATA_RECON_Y(_size, frame_height_coded, \
		is_ten_bit, num_vpp_pipes_enc) \
	do { \
		_size = ((VENUS_DMA_ALIGNMENT + 64 * ((frame_height_coded) / \
		(8 * (is_ten_bit ? 4 : 8)))));   \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
		_size = (_size * num_vpp_pipes_enc); \
	} while (0)

#define SIZE_LEFT_LINEBUFF_METADATA_RECON_UV(_size, frame_height_coded, \
			is_ten_bit, num_vpp_pipes_enc) \
	do { \
		_size = ((VENUS_DMA_ALIGNMENT + 64 * ((frame_height_coded) / \
				(4 * (is_ten_bit ? 4 : 8)))));    \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT); \
		_size = (_size * num_vpp_pipes_enc); \
	} while (0)

#define SIZE_LINEBUFF_RECON_PIX(_size, is_ten_bit, frame_width_coded) \
	do { \
		_size = ((is_ten_bit ? 3 : 2) * (frame_width_coded));         \
		_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT);             \
	} while (0)

#define SIZE_SLICE_CMD_BUFFER (HFI_ALIGN(20480, VENUS_DMA_ALIGNMENT))
#define SIZE_SPS_PPS_SLICE_HDR (2048 + 4096)

#define SIZE_FRAME_RC_BUF_SIZE(_size, standard, frame_height_coded, \
			num_vpp_pipes_enc) \
	do { \
		_size = (standard == HFI_CODEC_ENCODE_HEVC) ? (256 + 16 * \
			(14 + ((((frame_height_coded) >> 5) + 7) >> 3))) : \
			(256 + 16 * (14 + ((((frame_height_coded) >> 4) + 7) >> 3))); \
		_size *= 3; \
		if (num_vpp_pipes_enc > 1) { \
			_size = HFI_ALIGN(_size, VENUS_DMA_ALIGNMENT) * \
					num_vpp_pipes_enc;\
		} \
		_size = HFI_ALIGN(_size, BUFFER_ALIGNMENT_512_BYTES) * \
				HFI_MAX_COL_FRAME; \
	} while (0)

#define ENC_BITCNT_BUF_SIZE(num_lcu_in_frame) HFI_ALIGN((256 + \
		(4 * (num_lcu_in_frame))), VENUS_DMA_ALIGNMENT)
#define ENC_BITMAP_BUF_SIZE(num_lcu_in_frame) HFI_ALIGN((256 + \
		((num_lcu_in_frame) >> 3)), VENUS_DMA_ALIGNMENT)
#define SIZE_LINE_BUF_SDE(frame_width_coded) HFI_ALIGN((256 + \
		(16 * ((frame_width_coded) >> 4))), VENUS_DMA_ALIGNMENT)

#define SIZE_BSE_SLICE_CMD_BUF ((((8192 << 2) + 7) & (~7)) * 3)

#define SIZE_LAMBDA_LUT (256 * 11)
#define SIZE_OVERRIDE_BUF(num_lcumb) (HFI_ALIGN(((16 * (((num_lcumb) + 7)\
		>> 3))), VENUS_DMA_ALIGNMENT) * 2)
#define SIZE_IR_BUF(num_lcu_in_frame) HFI_ALIGN((((((num_lcu_in_frame) << 1) + 7) &\
	(~7)) * 3), VENUS_DMA_ALIGNMENT)

#define SIZE_VPSS_LINE_BUF(num_vpp_pipes_enc, frame_height_coded, \
			frame_width_coded) \
	(HFI_ALIGN(((((((8192) >> 2) << 5) * (num_vpp_pipes_enc)) + 64) + \
	(((((MAX((frame_width_coded), (frame_height_coded)) + 3) >> 2) << 5) +\
	256) * 16)), VENUS_DMA_ALIGNMENT))

#define SIZE_TOP_LINE_BUF_FIRST_STG_SAO(frame_width_coded) \
	HFI_ALIGN((16 * ((frame_width_coded) >> 5)), VENUS_DMA_ALIGNMENT)

#define HFI_BUFFER_LINE_ENC(_size, frame_width, frame_height, is_ten_bit, \
			num_vpp_pipes_enc, lcu_size, standard) \
	do { \
		HFI_U32 width_in_lcus = 0, height_in_lcus = 0, \
			frame_width_coded = 0, frame_height_coded = 0; \
		HFI_U32 line_buff_data_size = 0, left_line_buff_ctrl_size = 0, \
			left_line_buff_recon_pix_size = 0, \
			top_line_buff_ctrl_fe_size = 0; \
		HFI_U32 left_line_buff_metadata_recon__y__size = 0, \
			left_line_buff_metadata_recon__uv__size = 0, \
			line_buff_recon_pix_size = 0;          \
	    num_vpp_pipes_enc = 1; \
		width_in_lcus = ((frame_width) + (lcu_size)-1) / (lcu_size); \
		height_in_lcus = ((frame_height) + (lcu_size)-1) / (lcu_size); \
		frame_width_coded = width_in_lcus * (lcu_size); \
		frame_height_coded = height_in_lcus * (lcu_size); \
		SIZE_LINEBUFF_DATA(line_buff_data_size, is_ten_bit, \
			frame_width_coded);\
		SIZE_LEFT_LINEBUFF_CTRL(left_line_buff_ctrl_size, standard, \
			frame_height_coded, num_vpp_pipes_enc); \
		SIZE_LEFT_LINEBUFF_RECON_PIX(left_line_buff_recon_pix_size, \
			is_ten_bit, frame_height_coded, num_vpp_pipes_enc); \
		SIZE_TOP_LINEBUFF_CTRL_FE(top_line_buff_ctrl_fe_size, \
			frame_width_coded, standard); \
		SIZE_LEFT_LINEBUFF_METADATA_RECON_Y\
			(left_line_buff_metadata_recon__y__size, \
			frame_height_coded, is_ten_bit, num_vpp_pipes_enc); \
		SIZE_LEFT_LINEBUFF_METADATA_RECON_UV\
			(left_line_buff_metadata_recon__uv__size, \
			frame_height_coded, is_ten_bit, num_vpp_pipes_enc); \
		SIZE_LINEBUFF_RECON_PIX(line_buff_recon_pix_size, is_ten_bit,\
			frame_width_coded); \
		_size = SIZE_LINE_BUF_CTRL(frame_width_coded) + \
			SIZE_LINE_BUF_CTRL_ID2(frame_width_coded) + \
			line_buff_data_size + \
			left_line_buff_ctrl_size + \
			left_line_buff_recon_pix_size + \
			top_line_buff_ctrl_fe_size + \
			left_line_buff_metadata_recon__y__size + \
			left_line_buff_metadata_recon__uv__size + \
			line_buff_recon_pix_size + \
		SIZE_LEFT_LINEBUFF_CTRL_FE(frame_height_coded, \
			num_vpp_pipes_enc) + SIZE_LINE_BUF_SDE(frame_width_coded) + \
		SIZE_VPSS_LINE_BUF(num_vpp_pipes_enc, frame_height_coded, \
			frame_width_coded) + \
		SIZE_TOP_LINE_BUF_FIRST_STG_SAO(frame_width_coded); \
	} while (0)

#define HFI_BUFFER_LINE_H264E(_size, frame_width, frame_height, is_ten_bit, \
		num_vpp_pipes)                    \
	HFI_BUFFER_LINE_ENC(_size, frame_width, frame_height, 0, \
		num_vpp_pipes, 16, HFI_CODEC_ENCODE_AVC) \

#define HFI_BUFFER_LINE_H265E(_size, frame_width, frame_height, is_ten_bit, \
			num_vpp_pipes)                          \
	HFI_BUFFER_LINE_ENC(_size, frame_width, frame_height, \
		is_ten_bit, num_vpp_pipes, 32, HFI_CODEC_ENCODE_HEVC) \

#define HFI_BUFFER_COMV_ENC(_size, frame_width, frame_height, lcu_size, \
			num_recon, standard) \
	do { \
		HFI_U32 size_colloc_mv = 0, size_colloc_rc = 0; \
		HFI_U32 mb_width = ((frame_width) + 15) >> 4; \
		HFI_U32 mb_height = ((frame_height) + 15) >> 4; \
		HFI_U32 width_in_lcus = ((frame_width) + (lcu_size)-1) /\
					(lcu_size); \
		HFI_U32 height_in_lcus = ((frame_height) + (lcu_size)-1) / \
					(lcu_size); \
		HFI_U32 num_lcu_in_frame = width_in_lcus * height_in_lcus; \
		size_colloc_mv = (standard == HFI_CODEC_ENCODE_HEVC) ? \
		(16 * ((num_lcu_in_frame << 2) + BUFFER_ALIGNMENT_32_BYTES)) : \
		(3 * 16 * (width_in_lcus * height_in_lcus +\
		BUFFER_ALIGNMENT_32_BYTES)); \
		size_colloc_mv = HFI_ALIGN(size_colloc_mv, \
		VENUS_DMA_ALIGNMENT) * num_recon; \
		size_colloc_rc = (((mb_width + 3) >> 2) * 16 * mb_height); \
		size_colloc_rc = HFI_ALIGN(size_colloc_rc, \
		VENUS_DMA_ALIGNMENT) * HFI_MAX_COL_FRAME; \
		_size = size_colloc_mv + size_colloc_rc; \
	} while (0)

#define HFI_BUFFER_COMV_H264E(_size, frame_width, frame_height, num_recon) \
		HFI_BUFFER_COMV_ENC(_size, frame_width, frame_height, 16, \
			num_recon, HFI_CODEC_ENCODE_AVC) \

#define HFI_BUFFER_COMV_H265E(_size, frame_width, frame_height, num_recon) \
		HFI_BUFFER_COMV_ENC(_size, frame_width, frame_height, 32,\
			num_recon, HFI_CODEC_ENCODE_HEVC) \

#define HFI_BUFFER_NON_COMV_ENC(_size, frame_width, frame_height, \
			num_vpp_pipes_enc, lcu_size, standard) \
	do { \
		HFI_U32 width_in_lcus = 0, height_in_lcus = 0, \
		frame_width_coded = 0, frame_height_coded = 0, \
		num_lcu_in_frame = 0, num_lcumb = 0; \
		HFI_U32	frame_rc_buf_size = 0; \
	    num_vpp_pipes_enc = 1;\
		width_in_lcus = ((frame_width) + (lcu_size)-1) / (lcu_size); \
		height_in_lcus = ((frame_height) + (lcu_size)-1) / (lcu_size); \
		num_lcu_in_frame = width_in_lcus * height_in_lcus; \
		frame_width_coded = width_in_lcus * (lcu_size); \
		frame_height_coded = height_in_lcus * (lcu_size); \
		num_lcumb = (frame_height_coded / lcu_size) * \
		((frame_width_coded + lcu_size * 8) / lcu_size); \
		SIZE_FRAME_RC_BUF_SIZE(frame_rc_buf_size, standard, \
		frame_height_coded, num_vpp_pipes_enc); \
		_size = SIZE_ENC_SLICE_INFO_BUF(num_lcu_in_frame) + \
			   SIZE_SLICE_CMD_BUFFER + \
			   SIZE_SPS_PPS_SLICE_HDR + \
			   frame_rc_buf_size + \
			   ENC_BITCNT_BUF_SIZE(num_lcu_in_frame) + \
			   ENC_BITMAP_BUF_SIZE(num_lcu_in_frame) + \
			   SIZE_BSE_SLICE_CMD_BUF + \
			   SIZE_LAMBDA_LUT + \
			   SIZE_OVERRIDE_BUF(num_lcumb) + \
			   SIZE_IR_BUF(num_lcu_in_frame); \
	} while (0)

#define HFI_BUFFER_NON_COMV_H264E(_size, frame_width, frame_height, \
				num_vpp_pipes_enc) \
		HFI_BUFFER_NON_COMV_ENC(_size, frame_width, frame_height, \
				num_vpp_pipes_enc, 16, HFI_CODEC_ENCODE_AVC) \

#define HFI_BUFFER_NON_COMV_H265E(_size, frame_width, frame_height, \
				num_vpp_pipes_enc) \
		HFI_BUFFER_NON_COMV_ENC(_size, frame_width, frame_height, \
			num_vpp_pipes_enc, 32, HFI_CODEC_ENCODE_HEVC) \

#define SIZE_ENC_REF_BUFFER(size, frame_width, frame_height) \
	do { \
		HFI_U32 u_buffer_width = 0, u_buffer_height = 0, \
			u_chroma_buffer_height = 0; \
		u_buffer_height = HFI_ALIGN(frame_height, \
			HFI_VENUS_HEIGHT_ALIGNMENT); \
		u_chroma_buffer_height = frame_height >> 1; \
		u_chroma_buffer_height = HFI_ALIGN(u_chroma_buffer_height, \
			HFI_VENUS_HEIGHT_ALIGNMENT); \
		u_buffer_width = HFI_ALIGN(frame_width, \
			HFI_VENUS_WIDTH_ALIGNMENT); \
		size = (u_buffer_height + u_chroma_buffer_height) * \
			u_buffer_width; \
	} while (0)

#define SIZE_ENC_TEN_BIT_REF_BUFFER(size, frame_width, frame_height) \
	do { \
		HFI_U32 ref_buf_height = 0, ref_luma_stride_in_bytes = 0, \
		u_ref_stride = 0, luma_size = 0, ref_chrm_height_in_bytes = 0, \
		chroma_size = 0, ref_buf_size = 0; \
		ref_buf_height = (frame_height + \
		(HFI_VENUS_HEIGHT_ALIGNMENT - 1)) \
		& (~(HFI_VENUS_HEIGHT_ALIGNMENT - 1)); \
		ref_luma_stride_in_bytes = ((frame_width + \
		SYSTEM_LAL_TILE10 - 1) / SYSTEM_LAL_TILE10) * \
		SYSTEM_LAL_TILE10; \
		u_ref_stride = 4 * (ref_luma_stride_in_bytes / 3); \
		u_ref_stride = (u_ref_stride + (BUF_SIZE_ALIGN_128 - 1)) &\
			(~(BUF_SIZE_ALIGN_128 - 1)); \
		luma_size = ref_buf_height * u_ref_stride; \
		ref_chrm_height_in_bytes = (((frame_height + 1) >> 1) + \
			(BUF_SIZE_ALIGN_32 - 1)) & (~(BUF_SIZE_ALIGN_32 - 1)); \
		chroma_size = u_ref_stride * ref_chrm_height_in_bytes; \
		luma_size = (luma_size + (BUF_SIZE_ALIGN_4096 - 1)) & \
			(~(BUF_SIZE_ALIGN_4096 - 1)); \
		chroma_size = (chroma_size + (BUF_SIZE_ALIGN_4096 - 1)) & \
			(~(BUF_SIZE_ALIGN_4096 - 1)); \
		ref_buf_size = luma_size + chroma_size; \
		size = ref_buf_size; \
	} while (0)

#define HFI_BUFFER_DPB_ENC(_size, frame_width, frame_height, is_ten_bit) \
	do { \
		HFI_U32 ten_bit_ref_buf_size = 0, ref_buf_size = 0; \
		if (!is_ten_bit) { \
			SIZE_ENC_REF_BUFFER(ref_buf_size, frame_width, \
				frame_height); \
			_size = ref_buf_size;\
		} \
		else { \
			SIZE_ENC_TEN_BIT_REF_BUFFER(ten_bit_ref_buf_size, \
				frame_width, frame_height); \
			_size = ten_bit_ref_buf_size;\
		} \
	} while (0)

#define HFI_BUFFER_DPB_H264E(_size, frame_width, frame_height) \
		HFI_BUFFER_DPB_ENC(_size, frame_width, frame_height, 0) \

#define HFI_BUFFER_DPB_H265E(_size, frame_width, frame_height, is_ten_bit) \
		HFI_BUFFER_DPB_ENC(_size, frame_width, frame_height, is_ten_bit) \

#define HFI_BUFFER_VPSS_ENC(vpss_size, dswidth, dsheight, ds_enable, blur, is_ten_bit) \
	do { \
		vpss_size = 0; \
		if (ds_enable || blur) { \
			HFI_BUFFER_DPB_ENC(vpss_size, dswidth, dsheight, is_ten_bit); \
		} \
	} while (0)

#define HFI_AR50LT_ENC_MIN_INPUT_BUF_COUNT(numInput, TotalHBLayers) \
	do {  \
		numInput = 3;                                             \
		if (TotalHBLayers >= 2) {   \
			numInput = (1 << (TotalHBLayers - 1)) + 2;        \
		}                                                         \
	} while (0)

#endif /* __HFI_BUFFER_AR50LT__ */


// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/msm_media_info.h>
#include "msm_vidc_buffer_iris2.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"

#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_WIDTH 32
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_HEIGHT 8
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_WIDTH 16
#define HFI_COLOR_FORMAT_YUV420_NV12_UBWC_UV_TILE_HEIGHT 8
#define HFI_COLOR_FORMAT_YUV420_TP10_UBWC_Y_TILE_WIDTH 48
#define HFI_COLOR_FORMAT_YUV420_TP10_UBWC_Y_TILE_HEIGHT 4
#define BUFFER_ALIGNMENT_4096_BYTES 4096
#define VENUS_METADATA_STRIDE_MULTIPLE     64
#define VENUS_METADATA_HEIGHT_MULTIPLE     16
#define HFI_UBWC_CALC_METADATA_PLANE_STRIDE \
	((metadataStride, width, metadataStrideMultiple, tileWidthInPels) \
	metadataStride = ALIGN(((width + (tileWidthInPels - 1)) / \
		tileWidthInPels), metadataStrideMultiple))
#define HFI_UBWC_METADATA_PLANE_BUFHEIGHT \
	((metadataBufHeight, height, metadataHeightMultiple, tileHeightInPels) \
	metadataBufHeight = ALIGN(((height + (tileHeightInPels - 1)) / \
		tileHeightInPels), metadataHeightMultiple))
#define HFI_UBWC_METADATA_PLANE_BUFFER_SIZE \
	((buffersize, MetadataStride, MetadataBufHeight) \
	buffersize = ALIGN(MetadataStride * MetadataBufHeight, \
		BUFFER_ALIGNMENT_4096_BYTES))
#define HFI_UBWC_UV_METADATA_PLANE_STRIDE \
	((metadataStride, width, metadataStrideMultiple, tileWidthInPels) \
		metadataStride = ALIGN(((((width + 1) >> 1) + \
		(tileWidthInPels - 1)) / tileWidthInPels), \
		metadataStrideMultiple))
#define HFI_UBWC_UV_METADATA_PLANE_BUFHEIGHT \
	((metadataBufHeight, height, metadataHeightMultiple, tileHeightInPels) \
	metadataBufHeight = ALIGN(((((height + 1) >> 1) + \
		(tileHeightInPels - 1)) / tileHeightInPels), \
		metadataHeightMultiple))

#define BUFFER_ALIGNMENT_SIZE(x) x

#define VENUS_DMA_ALIGNMENT BUFFER_ALIGNMENT_SIZE(256)

#define MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE   64
#define MAX_FE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE   64
#define MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE   64
#define MAX_FE_NBR_DATA_LUMA_LINE_BUFFER_SIZE   640
#define MAX_FE_NBR_DATA_CB_LINE_BUFFER_SIZE     320
#define MAX_FE_NBR_DATA_CR_LINE_BUFFER_SIZE     320

#define MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE     (128 / 8)
#define MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE     (128 / 8)
#define MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE     (128 / 8)

#define MAX_PE_NBR_DATA_LCU64_LINE_BUFFER_SIZE     (64 * 2 * 3)
#define MAX_PE_NBR_DATA_LCU32_LINE_BUFFER_SIZE     (32 * 2 * 3)
#define MAX_PE_NBR_DATA_LCU16_LINE_BUFFER_SIZE     (16 * 2 * 3)

#define MAX_TILE_COLUMNS 32     /* 8K/256 */

#define VPP_CMD_MAX_SIZE (1 << 20)
#define NUM_HW_PIC_BUF 32
#define BIN_BUFFER_THRESHOLD (1280 * 736)
#define H264D_MAX_SLICE 1800
#define SIZE_H264D_BUFTAB_T  256 // sizeof(h264d_buftab_t) aligned to 256
#define SIZE_H264D_HW_PIC_T (1 << 11) // sizeof(h264d_hw_pic_t) 32 aligned
#define SIZE_H264D_BSE_CMD_PER_BUF (32 * 4)
#define SIZE_H264D_VPP_CMD_PER_BUF 512

// Line Buffer definitions
/* one for luma and 1/2 for each chroma */
#define SIZE_H264D_LB_FE_TOP_DATA(width, height) \
	(MAX_FE_NBR_DATA_LUMA_LINE_BUFFER_SIZE * \
	ALIGN(width, 16) * 3)

#define SIZE_H264D_LB_FE_TOP_CTRL(width, height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	((width + 15) >> 4))

#define SIZE_H264D_LB_FE_LEFT_CTRL(width, height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	((height + 15) >> 4))

#define SIZE_H264D_LB_SE_TOP_CTRL(width, height) \
	(MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	((width + 15) >> 4))

#define SIZE_H264D_LB_SE_LEFT_CTRL(width, height) \
	(MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	((height + 15) >> 4))

#define SIZE_H264D_LB_PE_TOP_DATA(width, height) \
	(MAX_PE_NBR_DATA_LCU64_LINE_BUFFER_SIZE * \
	((width + 15) >> 4))

#define SIZE_H264D_LB_VSP_TOP(width, height) \
	((((width + 15) >> 4) << 7))

#define SIZE_H264D_LB_RECON_DMA_METADATA_WR(width, height) \
	(ALIGN(height, 16) * 32)

#define SIZE_H264D_QP(width, height) \
	(((width + 63) >> 6) * ((height + 63) >> 6) * 128)

#define SIZE_HW_PIC(sizePerBuf) \
	(NUM_HW_PIC_BUF * sizePerBuf)

#define H264_CABAC_HDR_RATIO_HD_TOT 1
#define H264_CABAC_RES_RATIO_HD_TOT 3

/*
 * some content need more bin buffer, but limit buffer
 * size for high resolution
 */
#define NUM_SLIST_BUF_H264            (256 + 32)
#define SIZE_SLIST_BUF_H264           512

#define LCU_MAX_SIZE_PELS 64
#define LCU_MIN_SIZE_PELS 16

#define H265D_MAX_SLICE 3600
#define SIZE_H265D_HW_PIC_T SIZE_H264D_HW_PIC_T
#define SIZE_H265D_BSE_CMD_PER_BUF (16 * sizeof(u32))
#define SIZE_H265D_VPP_CMD_PER_BUF 256

#define SIZE_H265D_LB_FE_TOP_DATA(width, height) \
	(MAX_FE_NBR_DATA_LUMA_LINE_BUFFER_SIZE * \
	(ALIGN(width, 64) + 8) * 2)

#define SIZE_H265D_LB_FE_TOP_CTRL(width, height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	(ALIGN(width, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_FE_LEFT_CTRL(width, height) \
	(MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE * \
	(ALIGN(height, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_SE_TOP_CTRL(width, height) \
	((LCU_MAX_SIZE_PELS / 8 * (128 / 8)) * \
	((width + 15) >> 4))

#define SIZE_H265D_LB_SE_LEFT_CTRL(width, height) \
	(max(((height + 16 - 1) / 8) * MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE,\
	max(((height + 32 - 1) / 8) * MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((height + 64 - 1) / 8) * MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE)))

#define SIZE_H265D_LB_PE_TOP_DATA(width, height) \
	(MAX_PE_NBR_DATA_LCU64_LINE_BUFFER_SIZE * \
	(ALIGN(width, LCU_MIN_SIZE_PELS) / LCU_MIN_SIZE_PELS))

#define SIZE_H265D_LB_VSP_TOP(width, height) \
	(((width + 63) >> 6) * 128)

#define SIZE_H265D_LB_VSP_LEFT(width, height) \
	(((height + 63) >> 6) * 128)

#define SIZE_H265D_LB_RECON_DMA_METADATA_WR(width, height) \
	SIZE_H264D_LB_RECON_DMA_METADATA_WR(width, height)

#define SIZE_H265D_QP(width, height) SIZE_H264D_QP(width, height)

#define H265_CABAC_HDR_RATIO_HD_TOT 2
#define H265_CABAC_RES_RATIO_HD_TOT 2

/*
 * some content need more bin buffer, but limit buffer size
 * for high resolution
 */
#define SIZE_SLIST_BUF_H265 (1 << 10)
#define NUM_SLIST_BUF_H265 (80 + 20)
#define H265_NUM_TILE_COL 32
#define H265_NUM_TILE_ROW 128
#define H265_NUM_TILE (H265_NUM_TILE_ROW * H265_NUM_TILE_COL + 1)

#define SIZE_VPXD_LB_FE_LEFT_CTRL(width, height) \
	max(((height + 15) >> 4) * MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE, \
	max(((height + 31) >> 5) * MAX_FE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((height + 63) >> 6) * MAX_FE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE))
#define SIZE_VPXD_LB_FE_TOP_CTRL(width, height) \
	(((ALIGN(width, 64) + 8) * 10 * 2)) /* + small line */
#define SIZE_VPXD_LB_SE_TOP_CTRL(width, height) \
	(((width + 15) >> 4) * MAX_FE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE)
#define SIZE_VPXD_LB_SE_LEFT_CTRL(width, height) \
	max(((height + 15) >> 4) * MAX_SE_NBR_CTRL_LCU16_LINE_BUFFER_SIZE, \
	max(((height + 31) >> 5) * MAX_SE_NBR_CTRL_LCU32_LINE_BUFFER_SIZE, \
	((height + 63) >> 6) * MAX_SE_NBR_CTRL_LCU64_LINE_BUFFER_SIZE))
#define SIZE_VPXD_LB_RECON_DMA_METADATA_WR(width, height) \
	ALIGN((ALIGN(height, 16) / (4 / 2)) * 64, BUFFER_ALIGNMENT_SIZE(32))
#define SIZE_VP8D_LB_FE_TOP_DATA(width, height) \
	((ALIGN(width, 16) + 8) * 10 * 2)
#define SIZE_VP9D_LB_FE_TOP_DATA(width, height) \
	((ALIGN(ALIGN(width, 16), 64) + 8) * 10 * 2)
#define SIZE_VP8D_LB_PE_TOP_DATA(width, height) \
	((ALIGN(width, 16) >> 4) * 64)
#define SIZE_VP9D_LB_PE_TOP_DATA(width, height) \
	((ALIGN(ALIGN(width, 16), 64) >> 6) * 176)
#define SIZE_VP8D_LB_VSP_TOP(width, height) \
	(((ALIGN(width, 16) >> 4) * 64 / 2) + 256)
#define SIZE_VP9D_LB_VSP_TOP(width, height) \
	(((ALIGN(ALIGN(width, 16), 64) >> 6) * 64 * 8) + 256)


#define HFI_IRIS2_VP9D_COMV_SIZE \
	((((8192 + 63) >> 6) * ((4320 + 63) >> 6) * 8 * 8 * 2 * 8))

#define VPX_DECODER_FRAME_CONCURENCY_LVL 2
#define VPX_DECODER_FRAME_BIN_HDR_BUDGET_RATIO_NUM 1
#define VPX_DECODER_FRAME_BIN_HDR_BUDGET_RATIO_DEN 2
#define VPX_DECODER_FRAME_BIN_RES_BUDGET_RATIO_NUM 3
#define VPX_DECODER_FRAME_BIN_RES_BUDGET_RATIO_DEN 2

#define VP8_NUM_FRAME_INFO_BUF (5 + 1)
#define VP9_NUM_FRAME_INFO_BUF (32)
#define VP8_NUM_PROBABILITY_TABLE_BUF (VP8_NUM_FRAME_INFO_BUF)
#define VP9_NUM_PROBABILITY_TABLE_BUF (VP9_NUM_FRAME_INFO_BUF + 4)
#define VP8_PROB_TABLE_SIZE 3840
#define VP9_PROB_TABLE_SIZE 3840

#define VP9_UDC_HEADER_BUF_SIZE (3 * 128)
#define MAX_SUPERFRAME_HEADER_LEN (34)
#define CCE_TILE_OFFSET_SIZE ALIGN(32 * 4 * 4, BUFFER_ALIGNMENT_SIZE(32))

#define QMATRIX_SIZE (sizeof(u32) * 128 + 256)
#define MP2D_QPDUMP_SIZE 115200

#define HFI_IRIS2_ENC_PERSIST_SIZE 204800

#define HFI_MAX_COL_FRAME 6
#define HFI_VENUS_VENC_TRE_WB_BUFF_SIZE (65 << 4) // bytes
#define HFI_VENUS_VENC_DB_LINE_BUFF_PER_MB      512
#define HFI_VENUS_VPPSG_MAX_REGISTERS  2048
#define HFI_VENUS_WIDTH_ALIGNMENT 128
#define HFI_VENUS_WIDTH_TEN_BIT_ALIGNMENT 192
#define HFI_VENUS_HEIGHT_ALIGNMENT 32

#define SYSTEM_LAL_TILE10 192
#define HDR10_HIST_EXTRADATA_SIZE 4096

static u32 size_vpss_lb(u32 width, u32 height, u32 num_vpp_pipes)
{
	u32 vpss_4tap_top_buffer_size, vpss_div2_top_buffer_size;
	u32 vpss_4tap_left_buffer_size, vpss_div2_left_buffer_size;
	u32 opb_wr_top_line_luma_buf_size, opb_wr_top_line_chroma_buf_size;
	u32 opb_lb_wr_llb_y_buffer_size, opb_lb_wr_llb_uv_buffer_size;
	u32 macrotiling_size;
	u32 size = 0;

	vpss_4tap_top_buffer_size = vpss_div2_top_buffer_size =
		vpss_4tap_left_buffer_size = vpss_div2_left_buffer_size = 0;
	macrotiling_size = 32;
	opb_wr_top_line_luma_buf_size = ALIGN(width, macrotiling_size) /
		macrotiling_size * 256;
	opb_wr_top_line_luma_buf_size = ALIGN(opb_wr_top_line_luma_buf_size,
		VENUS_DMA_ALIGNMENT) + (MAX_TILE_COLUMNS - 1) * 256;
	opb_wr_top_line_luma_buf_size = max(opb_wr_top_line_luma_buf_size,
		(32 * ALIGN(height, 16)));
	opb_wr_top_line_chroma_buf_size = opb_wr_top_line_luma_buf_size;
	opb_lb_wr_llb_uv_buffer_size = opb_lb_wr_llb_y_buffer_size =
		ALIGN((ALIGN(height, 16) / 2) *
			64, BUFFER_ALIGNMENT_SIZE(32));
	size = num_vpp_pipes * 2 * (vpss_4tap_top_buffer_size +
		vpss_div2_top_buffer_size) +
		2 * (vpss_4tap_left_buffer_size +
		vpss_div2_left_buffer_size) +
	opb_wr_top_line_luma_buf_size +
	opb_wr_top_line_chroma_buf_size +
	opb_lb_wr_llb_uv_buffer_size +
	opb_lb_wr_llb_y_buffer_size;

	return size;
}

static u32 hfi_iris2_h264d_comv_size(u32 width, u32 height,
	u32 yuv_buf_min_count)
{
	u32 comv_size = 0;
	u32 frame_width_in_mbs = ((width + 15) >> 4);
	u32 frame_height_in_mbs = ((height + 15) >> 4);
	u32 col_mv_aligned_width = (frame_width_in_mbs << 7);
	u32 col_zero_aligned_width = (frame_width_in_mbs << 2);
	u32 col_zero_size = 0, size_colloc = 0;

	col_mv_aligned_width = ALIGN(col_mv_aligned_width,
		BUFFER_ALIGNMENT_SIZE(16));
	col_zero_aligned_width = ALIGN(col_zero_aligned_width,
		BUFFER_ALIGNMENT_SIZE(16));
	col_zero_size = col_zero_aligned_width *
		((frame_height_in_mbs + 1) >> 1);
	col_zero_size = ALIGN(col_zero_size, BUFFER_ALIGNMENT_SIZE(64));
	col_zero_size <<= 1;
	col_zero_size = ALIGN(col_zero_size, BUFFER_ALIGNMENT_SIZE(512));
	size_colloc = col_mv_aligned_width  * ((frame_height_in_mbs + 1) >> 1);
	size_colloc = ALIGN(size_colloc, BUFFER_ALIGNMENT_SIZE(64));
	size_colloc <<= 1;
	size_colloc = ALIGN(size_colloc, BUFFER_ALIGNMENT_SIZE(512));
	size_colloc += (col_zero_size + SIZE_H264D_BUFTAB_T * 2);
	comv_size = size_colloc * yuv_buf_min_count;
	comv_size += BUFFER_ALIGNMENT_SIZE(512);

	return comv_size;
}

static u32 size_h264d_bse_cmd_buf(u32 height)
{
	u32 aligned_height = ALIGN(height, BUFFER_ALIGNMENT_SIZE(32));

	return min_t(u32, (((aligned_height + 15) >> 4) * 3 * 4),
		H264D_MAX_SLICE) *
		SIZE_H264D_BSE_CMD_PER_BUF;
}

static u32 size_h264d_vpp_cmd_buf(u32 height)
{
	u32 size = 0;
	u32 aligned_height = ALIGN(height, BUFFER_ALIGNMENT_SIZE(32));

	size = min_t(u32, (((aligned_height + 15) >> 4) * 3 * 4),
		H264D_MAX_SLICE) *
		SIZE_H264D_VPP_CMD_PER_BUF;
	if (size > VPP_CMD_MAX_SIZE)
		size = VPP_CMD_MAX_SIZE;
	return size;
}

static u32 hfi_iris2_h264d_non_comv_size(u32 width, u32 height,
	u32 num_vpp_pipes)
{
	u32 size;
	u32 size_bse, size_vpp;

	size_bse = size_h264d_bse_cmd_buf(height);
	size_vpp = size_h264d_vpp_cmd_buf(height);
	size = ALIGN(size_bse, VENUS_DMA_ALIGNMENT) +
		ALIGN(size_vpp, VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_HW_PIC(SIZE_H264D_HW_PIC_T), VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_FE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_FE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_FE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_H264D_LB_SE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_SE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_H264D_LB_PE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_VSP_TOP(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H264D_LB_RECON_DMA_METADATA_WR(width, height),
			VENUS_DMA_ALIGNMENT) * 2 +
		ALIGN(SIZE_H264D_QP(width, height), VENUS_DMA_ALIGNMENT);
	size = ALIGN(size, VENUS_DMA_ALIGNMENT);
	return size;
}

static u32 size_h264d_hw_bin_buffer(u32 width, u32 height, u32 delay,
	u32 num_vpp_pipes)
{
	u32 size_yuv, size_bin_hdr, size_bin_res;
	u32 size = 0;
	u32 product;

	product = width * height;
	size_yuv = (product <= BIN_BUFFER_THRESHOLD) ?
			((BIN_BUFFER_THRESHOLD * 3) >> 1) :
			((product * 3) >> 1);

	size_bin_hdr = size_yuv * H264_CABAC_HDR_RATIO_HD_TOT;
	size_bin_res = size_yuv * H264_CABAC_RES_RATIO_HD_TOT;
	size_bin_hdr = size_bin_hdr * (((((u32)(delay)) & 31) / 10) + 2) / 2;
	size_bin_res = size_bin_res * (((((u32)(delay)) & 31) / 10) + 2) / 2;
	size_bin_hdr = ALIGN(size_bin_hdr / num_vpp_pipes,
		VENUS_DMA_ALIGNMENT) * num_vpp_pipes;
	size_bin_res = ALIGN(size_bin_res / num_vpp_pipes,
		VENUS_DMA_ALIGNMENT) * num_vpp_pipes;
	size = size_bin_hdr + size_bin_res;
	return size;
}

static u32 calculate_h264d_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, bool is_interlaced, u32 delay, u32 num_vpp_pipes)
{
	u32 aligned_width = ALIGN(width, BUFFER_ALIGNMENT_SIZE(16));
	u32 aligned_height = ALIGN(height, BUFFER_ALIGNMENT_SIZE(16));
	u32 size = 0;

	if (!is_interlaced)
		size = size_h264d_hw_bin_buffer(aligned_width, aligned_height,
						delay, num_vpp_pipes);
	else
		size = 0;

	return size;
}

static u32 size_h265d_bse_cmd_buf(u32 width, u32 height)
{
	u32 size;

	size = (ALIGN(width, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) *
		(ALIGN(height, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) *
		NUM_HW_PIC_BUF;
	size = min_t(u32, size, H265D_MAX_SLICE + 1);
	size = 2 * size * SIZE_H265D_BSE_CMD_PER_BUF;
	size = ALIGN(size, VENUS_DMA_ALIGNMENT);

	return size;
}

static u32 size_h265d_vpp_cmd_buf(u32 width, u32 height)
{
	u32 size = 0;

	size = (ALIGN(width, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) *
		(ALIGN(height, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) *
		NUM_HW_PIC_BUF;
	size = min_t(u32, size, H265D_MAX_SLICE + 1);
	size = ALIGN(size, 4);
	size = 2 * size * SIZE_H265D_VPP_CMD_PER_BUF;
	size = ALIGN(size, VENUS_DMA_ALIGNMENT);
	if (size > VPP_CMD_MAX_SIZE)
		size = VPP_CMD_MAX_SIZE;
	return size;
}

static u32 hfi_iris2_h265d_comv_size(u32 width, u32 height,
	u32 yuv_buf_count_min)
{
	u32 size = 0;

	size = ALIGN(((((width + 15) >> 4) * ((height + 15) >> 4)) << 8),
		BUFFER_ALIGNMENT_SIZE(512));
	size *= yuv_buf_count_min;
	size += BUFFER_ALIGNMENT_SIZE(512);

	return size;
}

static u32 hfi_iris2_h265d_non_comv_size(u32 width, u32 height,
	u32 num_vpp_pipes)
{
	u32 size_bse, size_vpp;
	u32 size = 0;

	size_bse = size_h265d_bse_cmd_buf(width, height);
	size_vpp = size_h265d_vpp_cmd_buf(width, height);
	size = ALIGN(size_bse, VENUS_DMA_ALIGNMENT) +
		ALIGN(size_vpp, VENUS_DMA_ALIGNMENT) +
		ALIGN(NUM_HW_PIC_BUF * 20 * 22 * 4, VENUS_DMA_ALIGNMENT) +
		ALIGN(2 * sizeof(u16) *
		(ALIGN(width, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS) *
			(ALIGN(height, LCU_MAX_SIZE_PELS) / LCU_MIN_SIZE_PELS),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_HW_PIC(SIZE_H265D_HW_PIC_T), VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_FE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_FE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_FE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_H265D_LB_SE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_H265D_LB_SE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_PE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_VSP_TOP(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_H265D_LB_VSP_LEFT(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_H265D_LB_RECON_DMA_METADATA_WR(width, height),
			VENUS_DMA_ALIGNMENT) * 4 +
		ALIGN(SIZE_H265D_QP(width, height), VENUS_DMA_ALIGNMENT);
	size = ALIGN(size, VENUS_DMA_ALIGNMENT);
	return size;
}

static u32 size_h265d_hw_bin_buffer(u32 width, u32 height, u32 delay,
	u32 num_vpp_pipes)
{
	u32 size = 0;
	u32 size_yuv, size_bin_hdr, size_bin_res;
	u32 product;

	product = width * height;
	size_yuv = (product <= BIN_BUFFER_THRESHOLD) ?
		((BIN_BUFFER_THRESHOLD * 3) >> 1) :
		((product * 3) >> 1);
	size_bin_hdr = size_yuv * H265_CABAC_HDR_RATIO_HD_TOT;
	size_bin_res = size_yuv * H265_CABAC_RES_RATIO_HD_TOT;
	size_bin_hdr = size_bin_hdr * (((((u32)(delay)) & 31) / 10) + 2) / 2;
	size_bin_res = size_bin_res * (((((u32)(delay)) & 31) / 10) + 2) / 2;
	size_bin_hdr = ALIGN(size_bin_hdr / num_vpp_pipes,
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes;
	size_bin_res = ALIGN(size_bin_res / num_vpp_pipes,
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes;
	size = size_bin_hdr + size_bin_res;

	return size;
}

static u32 calculate_h265d_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, bool is_interlaced, u32 delay, u32 num_vpp_pipes)
{
	u32 aligned_width = ALIGN(width, BUFFER_ALIGNMENT_SIZE(16));
	u32 aligned_height = ALIGN(height, BUFFER_ALIGNMENT_SIZE(16));
	u32 size = 0;

	if (!is_interlaced)
		size = size_h265d_hw_bin_buffer(aligned_width, aligned_height,
						delay, num_vpp_pipes);
	else
		size = 0;

	return size;
}

static u32 calculate_vpxd_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, bool is_interlaced, u32 delay, u32 num_vpp_pipes)
{
	u32 aligned_width = ALIGN(width, BUFFER_ALIGNMENT_SIZE(16));
	u32 aligned_height = ALIGN(height, BUFFER_ALIGNMENT_SIZE(16));
	u32 size = 0;
	u32 size_yuv = aligned_width * aligned_height * 3 / 2;

	if (!is_interlaced) {
		/* binbuffer1_size + binbufer2_size */
		u32 binbuffer1_size = 0, binbuffer2_size = 0;

		binbuffer1_size = ALIGN(max_t(u32, size_yuv,
			((BIN_BUFFER_THRESHOLD * 3) >> 1)) *
			VPX_DECODER_FRAME_CONCURENCY_LVL *
			VPX_DECODER_FRAME_BIN_HDR_BUDGET_RATIO_NUM /
			VPX_DECODER_FRAME_BIN_HDR_BUDGET_RATIO_DEN,
			VENUS_DMA_ALIGNMENT);
		binbuffer2_size = ALIGN(max_t(u32, size_yuv,
			((BIN_BUFFER_THRESHOLD * 3) >> 1)) *
			VPX_DECODER_FRAME_CONCURENCY_LVL *
			VPX_DECODER_FRAME_BIN_RES_BUDGET_RATIO_NUM /
			VPX_DECODER_FRAME_BIN_RES_BUDGET_RATIO_DEN,
			VENUS_DMA_ALIGNMENT);
		size = binbuffer1_size + binbuffer2_size;
		size = size * num_vpp_pipes;
	} else {
		size = 0;
	}

	return size;
}

static u32 calculate_mpeg2d_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, bool is_interlaced, u32 delay, u32 num_vpp_pipes)
{
	return 0;
}

static u32 calculate_enc_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 work_mode, u32 lcu_size, u32 num_vpp_pipes)
{
	u32 aligned_width, aligned_height, bitstream_size;
	u32 total_bitbin_buffers = 0, size_singlePipe, bitbin_size = 0;
	u32 sao_bin_buffer_size, padded_bin_size, size = 0;

	aligned_width = ALIGN(width, lcu_size);
	aligned_height = ALIGN(height, lcu_size);
	bitstream_size = msm_vidc_encoder_output_size(inst);

	bitstream_size = ALIGN(bitstream_size, VENUS_DMA_ALIGNMENT);
	if (1) { // TODO: work_mode == HFI_WORKMODE_2) {
		total_bitbin_buffers = 3;
		bitbin_size = bitstream_size * 17 / 10;
		bitbin_size = ALIGN(bitbin_size, VENUS_DMA_ALIGNMENT);
	} else {
		total_bitbin_buffers = 1;
		bitstream_size = aligned_width * aligned_height * 3;
		bitbin_size = ALIGN(bitstream_size, VENUS_DMA_ALIGNMENT);
	}
	if (num_vpp_pipes > 2)
		size_singlePipe = bitbin_size / 2;
	else
		size_singlePipe = bitbin_size;
	// TODO
	//if (inst->rc_type == RATE_CONTROL_LOSSLESS)
	//	size_singlePipe <<= 1;
	size_singlePipe = ALIGN(size_singlePipe, VENUS_DMA_ALIGNMENT);
	sao_bin_buffer_size = (64 * (((width + BUFFER_ALIGNMENT_SIZE(32)) *
		(height + BUFFER_ALIGNMENT_SIZE(32))) >> 10)) + 384;
	padded_bin_size = ALIGN(size_singlePipe, VENUS_DMA_ALIGNMENT);
	size_singlePipe = sao_bin_buffer_size + padded_bin_size;
	size_singlePipe = ALIGN(size_singlePipe, VENUS_DMA_ALIGNMENT);
	bitbin_size = size_singlePipe * num_vpp_pipes;
	size = ALIGN(bitbin_size, VENUS_DMA_ALIGNMENT) * total_bitbin_buffers
			+ 512;

	return size;
}

static u32 calculate_h264e_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 work_mode, u32 num_vpp_pipes)
{
	return calculate_enc_scratch_size(inst, width, height, work_mode, 16,
		num_vpp_pipes);
}

static u32 calculate_h265e_scratch_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 work_mode, u32 num_vpp_pipes)
{
	return calculate_enc_scratch_size(inst, width, height, work_mode, 32,
		num_vpp_pipes);
}

static u32 calculate_h264d_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 min_buf_count,
	bool split_mode_enabled, u32 num_vpp_pipes)
{
	u32 co_mv_size = 0, nonco_mv_size = 0;
	u32 vpss_lb_size = 0;
	u32 size = 0;

	co_mv_size = hfi_iris2_h264d_comv_size(width, height, min_buf_count);
	nonco_mv_size = hfi_iris2_h264d_non_comv_size(width, height,
			num_vpp_pipes);
	if (split_mode_enabled)
		vpss_lb_size = size_vpss_lb(width, height, num_vpp_pipes);
	size = co_mv_size + nonco_mv_size + vpss_lb_size;
	return size;
}

static u32 calculate_h265d_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 min_buf_count,
	bool split_mode_enabled, u32 num_vpp_pipes)
{
	u32 co_mv_size = 0, nonco_mv_size = 0;
	u32 vpss_lb_size = 0;
	u32 size = 0;

	co_mv_size = hfi_iris2_h265d_comv_size(width, height, min_buf_count);
	nonco_mv_size =
		hfi_iris2_h265d_non_comv_size(width, height, num_vpp_pipes);
	if (split_mode_enabled)
		vpss_lb_size = size_vpss_lb(width, height, num_vpp_pipes);

	size = co_mv_size + nonco_mv_size + vpss_lb_size +
			HDR10_HIST_EXTRADATA_SIZE;
	return size;
}

static u32 calculate_vp9d_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 min_buf_count,
	bool split_mode_enabled, u32 num_vpp_pipes)
{
	u32 vpss_lb_size = 0;
	u32 size = 0;

	size = ALIGN(SIZE_VPXD_LB_FE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_VPXD_LB_SE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_VP9D_LB_VSP_TOP(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VPXD_LB_FE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		2 * ALIGN(SIZE_VPXD_LB_RECON_DMA_METADATA_WR(width, height),
				VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VPXD_LB_SE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VP9D_LB_PE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VP9D_LB_FE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT);
	if (split_mode_enabled)
		vpss_lb_size = size_vpss_lb(width, height, num_vpp_pipes);

	size += vpss_lb_size + HDR10_HIST_EXTRADATA_SIZE;
	return size;
}

static u32 calculate_mpeg2d_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 min_buf_count,
	bool split_mode_enabled, u32 num_vpp_pipes)
{
	u32 vpss_lb_size = 0;
	u32 size = 0;

	size = ALIGN(SIZE_VPXD_LB_FE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_VPXD_LB_SE_LEFT_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes +
		ALIGN(SIZE_VP8D_LB_VSP_TOP(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VPXD_LB_FE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		2 * ALIGN(SIZE_VPXD_LB_RECON_DMA_METADATA_WR(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VPXD_LB_SE_TOP_CTRL(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VP8D_LB_PE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT) +
		ALIGN(SIZE_VP8D_LB_FE_TOP_DATA(width, height),
			VENUS_DMA_ALIGNMENT);
	if (split_mode_enabled)
		vpss_lb_size = size_vpss_lb(width, height, num_vpp_pipes);

	size += vpss_lb_size;
	return size;
}

static u32 calculate_enc_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 lcu_size, u32 num_ref, bool ten_bit,
	u32 num_vpp_pipes, bool is_h265)
{
	u32 line_buf_ctrl_size, line_buf_data_size, leftline_buf_ctrl_size;
	u32 line_buf_sde_size, sps_pps_slice_hdr, topline_buf_ctrl_size_FE;
	u32 leftline_buf_ctrl_size_FE, line_buf_recon_pix_size;
	u32 leftline_buf_recon_pix_size, lambda_lut_size, override_buffer_size;
	u32 col_mv_buf_size, vpp_reg_buffer_size, ir_buffer_size;
	u32 vpss_line_buf, leftline_buf_meta_recony, col_rc_buf_size;
	u32 h265e_framerc_bufsize, h265e_lcubitcnt_bufsize;
	u32 h265e_lcubitmap_bufsize, se_stats_bufsize;
	u32 bse_reg_buffer_size, bse_slice_cmd_buffer_size, slice_info_bufsize;
	u32 line_buf_ctrl_size_buffid2, slice_cmd_buffer_size;
	u32 width_lcu_num, height_lcu_num, width_coded, height_coded;
	u32 frame_num_lcu, linebuf_meta_recon_uv, topline_bufsize_fe_1stg_sao;
	u32 output_mv_bufsize = 0, temp_scratch_mv_bufsize = 0;
	u32 size, bit_depth, num_LCUMB;
	u32 vpss_lineBufferSize_1 = 0;
	u32 width_mb_num = ((width + 15) >> 4);
	u32 height_mb_num = ((height + 15) >> 4);

	width_lcu_num = ((width)+(lcu_size)-1) / (lcu_size);
	height_lcu_num = ((height)+(lcu_size)-1) / (lcu_size);
	frame_num_lcu = width_lcu_num * height_lcu_num;
	width_coded = width_lcu_num * (lcu_size);
	height_coded = height_lcu_num * (lcu_size);
	num_LCUMB = (height_coded / lcu_size) * ((width_coded + lcu_size * 8) / lcu_size);
	slice_info_bufsize = (256 + (frame_num_lcu << 4));
	slice_info_bufsize = ALIGN(slice_info_bufsize, VENUS_DMA_ALIGNMENT);
	line_buf_ctrl_size = ALIGN(width_coded, VENUS_DMA_ALIGNMENT);
	line_buf_ctrl_size_buffid2 = ALIGN(width_coded, VENUS_DMA_ALIGNMENT);

	bit_depth = ten_bit ? 10 : 8;
	line_buf_data_size = (((((bit_depth * width_coded + 1024) +
	(VENUS_DMA_ALIGNMENT - 1)) & (~(VENUS_DMA_ALIGNMENT - 1))) * 1) +
	(((((bit_depth * width_coded + 1024) >> 1) +
	(VENUS_DMA_ALIGNMENT - 1)) &
	(~(VENUS_DMA_ALIGNMENT - 1))) * 2));
	leftline_buf_ctrl_size = (is_h265) ?
		((height_coded + (BUFFER_ALIGNMENT_SIZE(32))) /
		BUFFER_ALIGNMENT_SIZE(32) * 4 * 16) :
		((height_coded + 15) / 16 * 5 * 16);
	if (num_vpp_pipes > 1) {
		leftline_buf_ctrl_size += BUFFER_ALIGNMENT_SIZE(512);
		leftline_buf_ctrl_size = ALIGN(leftline_buf_ctrl_size,
			BUFFER_ALIGNMENT_SIZE(512)) * num_vpp_pipes;
	}
	leftline_buf_ctrl_size = ALIGN(leftline_buf_ctrl_size,
		VENUS_DMA_ALIGNMENT);
	leftline_buf_recon_pix_size = (((ten_bit + 1) * 2 *
		(height_coded)+VENUS_DMA_ALIGNMENT) +
	(VENUS_DMA_ALIGNMENT << (num_vpp_pipes - 1)) - 1) &
		(~((VENUS_DMA_ALIGNMENT << (num_vpp_pipes - 1)) - 1)) * 1;
	topline_buf_ctrl_size_FE = (is_h265) ? (64 * (width_coded >> 5)) :
		(VENUS_DMA_ALIGNMENT + 16 * (width_coded >> 4));
	topline_buf_ctrl_size_FE = ALIGN(topline_buf_ctrl_size_FE,
		VENUS_DMA_ALIGNMENT);
	leftline_buf_ctrl_size_FE = (((VENUS_DMA_ALIGNMENT + 64 *
		(height_coded >> 4)) +
		(VENUS_DMA_ALIGNMENT << (num_vpp_pipes - 1)) - 1) &
		(~((VENUS_DMA_ALIGNMENT << (num_vpp_pipes - 1)) - 1)) * 1) *
		num_vpp_pipes;
	leftline_buf_meta_recony = (VENUS_DMA_ALIGNMENT + 64 *
		((height_coded) / (8 * (ten_bit ? 4 : 8))));
	leftline_buf_meta_recony = ALIGN(leftline_buf_meta_recony,
		VENUS_DMA_ALIGNMENT);
	leftline_buf_meta_recony = leftline_buf_meta_recony *
		num_vpp_pipes;
	linebuf_meta_recon_uv = (VENUS_DMA_ALIGNMENT + 64 *
		((height_coded) / (4 * (ten_bit ? 4 : 8))));
	linebuf_meta_recon_uv = ALIGN(linebuf_meta_recon_uv,
		VENUS_DMA_ALIGNMENT);
	linebuf_meta_recon_uv = linebuf_meta_recon_uv *
		num_vpp_pipes;
	line_buf_recon_pix_size = ((ten_bit ? 3 : 2) * width_coded);
	line_buf_recon_pix_size = ALIGN(line_buf_recon_pix_size,
		VENUS_DMA_ALIGNMENT);
	slice_cmd_buffer_size = ALIGN(20480, VENUS_DMA_ALIGNMENT);
	sps_pps_slice_hdr = 2048 + 4096;
	col_mv_buf_size = (is_h265) ? (16 * ((frame_num_lcu << 2) +
		BUFFER_ALIGNMENT_SIZE(32))) :
		(3 * 16 * (width_lcu_num * height_lcu_num +
		BUFFER_ALIGNMENT_SIZE(32)));
	col_mv_buf_size = ALIGN(col_mv_buf_size, VENUS_DMA_ALIGNMENT)
		* (num_ref + 1);
	col_rc_buf_size = (((width_mb_num + 7) >> 3) *
		16 * 2 * height_mb_num);
	col_rc_buf_size = ALIGN(col_rc_buf_size,
		VENUS_DMA_ALIGNMENT) * HFI_MAX_COL_FRAME;
	h265e_framerc_bufsize = (is_h265) ? (256 + 16 *
		(14 + (((height_coded >> 5) + 7) >> 3))) :
		(256 + 16 * (14 + (((height_coded >> 4) + 7) >> 3)));
	h265e_framerc_bufsize *= 6;   /* multiply by max numtilescol*/
	if (num_vpp_pipes > 1)
		h265e_framerc_bufsize = ALIGN(h265e_framerc_bufsize,
			VENUS_DMA_ALIGNMENT) * num_vpp_pipes;

	h265e_framerc_bufsize = ALIGN(h265e_framerc_bufsize,
		BUFFER_ALIGNMENT_SIZE(512)) * HFI_MAX_COL_FRAME;
	h265e_lcubitcnt_bufsize = (256 + 4 * frame_num_lcu);
	h265e_lcubitcnt_bufsize = ALIGN(h265e_lcubitcnt_bufsize,
		VENUS_DMA_ALIGNMENT);
	h265e_lcubitmap_bufsize = 256 + (frame_num_lcu >> 3);
	h265e_lcubitmap_bufsize = ALIGN(h265e_lcubitmap_bufsize,
		VENUS_DMA_ALIGNMENT);
	line_buf_sde_size = 256 + 16 * (width_coded >> 4);
	line_buf_sde_size = ALIGN(line_buf_sde_size, VENUS_DMA_ALIGNMENT);
	if ((width_coded * height_coded) > (4096 * 2160))
		se_stats_bufsize = 0;
	else if ((width_coded * height_coded) > (1920 * 1088))
		se_stats_bufsize = (40 * 4 * frame_num_lcu + 256 + 256);
	else
		se_stats_bufsize = (1024 * frame_num_lcu + 256 + 256);

	se_stats_bufsize = ALIGN(se_stats_bufsize, VENUS_DMA_ALIGNMENT) * 2;
	bse_slice_cmd_buffer_size = ((((8192 << 2) + 7) & (~7)) * 6);
	bse_reg_buffer_size = ((((512 << 3) + 7) & (~7)) * 4);
	vpp_reg_buffer_size = ((((HFI_VENUS_VPPSG_MAX_REGISTERS << 3) + 31) &
		(~31)) * 10);
	lambda_lut_size = (256 * 11);
	override_buffer_size = 16 * ((num_LCUMB + 7) >> 3);
	override_buffer_size = ALIGN(override_buffer_size,
		VENUS_DMA_ALIGNMENT) * 2;
	ir_buffer_size = (((frame_num_lcu << 1) + 7) & (~7)) * 3;
	vpss_lineBufferSize_1 = ((((8192) >> 2) << 5) * num_vpp_pipes) + 64;
	vpss_line_buf = (((((max(width_coded, height_coded) + 3) >> 2) << 5)
		 + 256) * 16) + vpss_lineBufferSize_1;
	topline_bufsize_fe_1stg_sao = (16 * (width_coded >> 5));
	topline_bufsize_fe_1stg_sao = ALIGN(topline_bufsize_fe_1stg_sao,
		VENUS_DMA_ALIGNMENT);
	size = line_buf_ctrl_size + line_buf_data_size +
		line_buf_ctrl_size_buffid2 + leftline_buf_ctrl_size +
		vpss_line_buf + col_mv_buf_size + topline_buf_ctrl_size_FE +
		leftline_buf_ctrl_size_FE + line_buf_recon_pix_size +
		leftline_buf_recon_pix_size + leftline_buf_meta_recony +
		linebuf_meta_recon_uv + col_rc_buf_size +
		h265e_framerc_bufsize + h265e_lcubitcnt_bufsize +
		h265e_lcubitmap_bufsize + line_buf_sde_size +
		topline_bufsize_fe_1stg_sao + override_buffer_size +
		bse_reg_buffer_size + vpp_reg_buffer_size +
		sps_pps_slice_hdr + slice_cmd_buffer_size +
		bse_slice_cmd_buffer_size + ir_buffer_size + slice_info_bufsize
		+ lambda_lut_size + se_stats_bufsize + temp_scratch_mv_bufsize
		+ output_mv_bufsize + 1024;
	return size;
}

static u32 calculate_h264e_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 num_ref, bool ten_bit, u32 num_vpp_pipes)
{
	return calculate_enc_scratch1_size(inst, width, height, 16,
		num_ref, ten_bit, num_vpp_pipes, false);
}

static u32 calculate_h265e_scratch1_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 num_ref, bool ten_bit, u32 num_vpp_pipes)
{
	return calculate_enc_scratch1_size(inst, width, height, 32,
		num_ref, ten_bit, num_vpp_pipes, true);
}

static u32 hfi_ubwc_calc_metadata_plane_stride(u32 width,
	u32 metadata_stride_multi, u32 tile_width_pels)
{
	return ALIGN(((width + (tile_width_pels - 1)) / tile_width_pels),
		metadata_stride_multi);
}

static u32 hfi_ubwc_metadata_plane_bufheight(u32 height,
	u32 metadata_height_multi, u32 tile_height_pels)
{
	return ALIGN(((height + (tile_height_pels - 1)) / tile_height_pels),
		metadata_height_multi);
}

static u32 hfi_ubwc_metadata_plane_buffer_size(u32 metadata_stride,
	u32 metadata_buf_height)
{
	return ALIGN(metadata_stride * metadata_buf_height,
		BUFFER_ALIGNMENT_4096_BYTES);
}

static u32 calculate_enc_scratch2_size(struct msm_vidc_inst *inst,
	u32 width, u32 height, u32 num_ref, bool ten_bit)
{
	u32 aligned_width, aligned_height, chroma_height, ref_buf_height;
	u32 luma_size, chroma_size;
	u32 metadata_stride, meta_buf_height, meta_size_y, meta_size_c;
	u32 ref_luma_stride_bytes, ref_chroma_height_bytes;
	u32 ref_buf_size = 0, ref_stride;
	u32 size;

	if (!ten_bit) {
		aligned_height = ALIGN(height, HFI_VENUS_HEIGHT_ALIGNMENT);
		chroma_height = height >> 1;
		chroma_height = ALIGN(chroma_height,
			HFI_VENUS_HEIGHT_ALIGNMENT);
		aligned_width = ALIGN(width, HFI_VENUS_WIDTH_ALIGNMENT);
		metadata_stride = hfi_ubwc_calc_metadata_plane_stride(width,
			64, HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_WIDTH);
		meta_buf_height = hfi_ubwc_metadata_plane_bufheight(height,
			16, HFI_COLOR_FORMAT_YUV420_NV12_UBWC_Y_TILE_HEIGHT);
		meta_size_y = hfi_ubwc_metadata_plane_buffer_size(
			metadata_stride, meta_buf_height);
		meta_size_c = hfi_ubwc_metadata_plane_buffer_size(
			metadata_stride, meta_buf_height);
		size = (aligned_height + chroma_height) * aligned_width +
			meta_size_y + meta_size_c;
		size = (size * (num_ref+3)) + 4096;
	} else {
		ref_buf_height = (height + (HFI_VENUS_HEIGHT_ALIGNMENT - 1))
			& (~(HFI_VENUS_HEIGHT_ALIGNMENT - 1));
		ref_luma_stride_bytes = ((width + SYSTEM_LAL_TILE10 - 1) /
			SYSTEM_LAL_TILE10) * SYSTEM_LAL_TILE10;
		ref_stride = 4 * (ref_luma_stride_bytes / 3);
		ref_stride = (ref_stride + (BUFFER_ALIGNMENT_SIZE(128) - 1)) &
			(~(BUFFER_ALIGNMENT_SIZE(128) - 1));
		luma_size = ref_buf_height * ref_stride;
		ref_chroma_height_bytes = (((height + 1) >> 1) +
			(BUFFER_ALIGNMENT_SIZE(32) - 1)) &
			(~(BUFFER_ALIGNMENT_SIZE(32) - 1));
		chroma_size = ref_stride * ref_chroma_height_bytes;
		luma_size = (luma_size + (BUFFER_ALIGNMENT_4096_BYTES - 1)) &
			(~(BUFFER_ALIGNMENT_4096_BYTES - 1));
		chroma_size = (chroma_size +
			(BUFFER_ALIGNMENT_4096_BYTES - 1)) &
			(~(BUFFER_ALIGNMENT_4096_BYTES - 1));
		ref_buf_size = luma_size + chroma_size;
		metadata_stride = hfi_ubwc_calc_metadata_plane_stride(
			width,
			VENUS_METADATA_STRIDE_MULTIPLE,
			HFI_COLOR_FORMAT_YUV420_TP10_UBWC_Y_TILE_WIDTH);
		meta_buf_height = hfi_ubwc_metadata_plane_bufheight(
			height,
			VENUS_METADATA_HEIGHT_MULTIPLE,
			HFI_COLOR_FORMAT_YUV420_TP10_UBWC_Y_TILE_HEIGHT);
		meta_size_y = hfi_ubwc_metadata_plane_buffer_size(
			metadata_stride, meta_buf_height);
		meta_size_c = hfi_ubwc_metadata_plane_buffer_size(
			metadata_stride, meta_buf_height);
		size = ref_buf_size + meta_size_y + meta_size_c;
		size = (size * (num_ref+3)) + 4096;
	}
	return size;
}

static u32 calculate_enc_persist_size(void)
{
	return HFI_IRIS2_ENC_PERSIST_SIZE;
}

static u32 calculate_h264d_persist1_size(void)
{
	u32 size = 0;

	size = ALIGN((SIZE_SLIST_BUF_H264 * NUM_SLIST_BUF_H264),
			VENUS_DMA_ALIGNMENT);
	return size;
}

static u32 calculate_h265d_persist1_size(void)
{
	u32 size = 0;

	size = ALIGN((SIZE_SLIST_BUF_H265 * NUM_SLIST_BUF_H265 + H265_NUM_TILE
			* sizeof(u32)), VENUS_DMA_ALIGNMENT);
	return size;
}

static u32 calculate_vp9d_persist1_size(void)
{
	u32 size = 0;

	size = ALIGN(VP9_NUM_PROBABILITY_TABLE_BUF * VP9_PROB_TABLE_SIZE,
			VENUS_DMA_ALIGNMENT) +
			ALIGN(HFI_IRIS2_VP9D_COMV_SIZE, VENUS_DMA_ALIGNMENT) +
			ALIGN(MAX_SUPERFRAME_HEADER_LEN, VENUS_DMA_ALIGNMENT) +
			ALIGN(VP9_UDC_HEADER_BUF_SIZE, VENUS_DMA_ALIGNMENT) +
			ALIGN(VP9_NUM_FRAME_INFO_BUF * CCE_TILE_OFFSET_SIZE,
			VENUS_DMA_ALIGNMENT);
	return size;
}

static u32 calculate_mpeg2d_persist1_size(void)
{
	return QMATRIX_SIZE + MP2D_QPDUMP_SIZE;
}

/* decoder internal buffers */
u32 msm_vidc_decoder_scratch_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format *f;
	bool is_interlaced;
	u32 vpp_delay;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}

	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	is_interlaced = false; //(inst->pic_struct == MSM_VIDC_PIC_STRUCT_MAYBE_INTERLACED);
	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264) {
		size = calculate_h264d_scratch_size(inst, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		size = calculate_h265d_scratch_size(inst, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_MPEG2) {
		size = calculate_mpeg2d_scratch_size(inst, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_VP9) {
		size = calculate_vpxd_scratch_size(inst, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	}

	return size;
}

u32 msm_vidc_decoder_scratch_1_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, out_min_count, num_vpp_pipes;
	struct v4l2_format *f;
	u32 vpp_delay;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}

	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	out_min_count = inst->buffers.output.min_count;
	out_min_count = max(vpp_delay + 1, out_min_count);

	if (inst->codec == MSM_VIDC_H264) {
		size = calculate_h264d_scratch1_size(inst, width, height,
			out_min_count, is_secondary_output_mode(inst),
			num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		size = calculate_h265d_scratch1_size(inst, width, height,
			out_min_count, is_secondary_output_mode(inst),
			num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_MPEG2) {
		size = calculate_mpeg2d_scratch1_size(inst, width, height,
			out_min_count, is_secondary_output_mode(inst),
			num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_VP9) {
		size = calculate_vp9d_scratch1_size(inst, width, height,
			out_min_count, is_secondary_output_mode(inst),
			num_vpp_pipes);
	}

	return size;
}

u32 msm_vidc_decoder_persist_1_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	if (inst->codec == MSM_VIDC_H264) {
		size = calculate_h264d_persist1_size();
	} else if (inst->codec == MSM_VIDC_HEVC) {
		size = calculate_h265d_persist1_size();
	} else if (inst->codec == MSM_VIDC_MPEG2) {
		size = calculate_mpeg2d_persist1_size();
	} else if (inst->codec == MSM_VIDC_VP9) {
		size = calculate_vp9d_persist1_size();
	}

	return size;
}

/* encoder internal buffers */
u32 msm_vidc_encoder_scratch_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}

	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264) {
		size = calculate_h264e_scratch_size(inst, width, height,
				2, //inst->clk_data.work_mode,
				num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		size = calculate_h265e_scratch_size(inst, width, height,
				2, //inst->clk_data.work_mode,
				num_vpp_pipes);
	}

	return size;
}

u32 msm_vidc_encoder_scratch_1_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_ref, num_vpp_pipes;
	bool is_tenbit = false;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: Instance is null!", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		d_vpr_e("%s: invalid capabilities\n", __func__);
		return size;
	}

	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	num_ref = 4; //msm_vidc_get_num_ref_frames(inst);
	is_tenbit = false; //(inst->bit_depth == MSM_VIDC_BIT_DEPTH_10);

	if (inst->codec == MSM_VIDC_H264) {
		size = calculate_h264e_scratch1_size(inst, width, height,
				num_ref, is_tenbit, num_vpp_pipes);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		size = calculate_h265e_scratch1_size(inst, width, height,
				num_ref, is_tenbit, num_vpp_pipes);
	}

	return size;
}

u32 msm_vidc_encoder_scratch_2_size_iris2(struct msm_vidc_inst *inst)
{
	u32 width, height, num_ref;
	bool is_tenbit = false;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	num_ref = 4; //msm_vidc_get_num_ref_frames(inst);
	is_tenbit = false; //(inst->bit_depth == MSM_VIDC_BIT_DEPTH_10);

	return calculate_enc_scratch2_size(inst, width, height,
			num_ref, is_tenbit);
}

u32 msm_vidc_encoder_persist_size_iris2(struct msm_vidc_inst *inst)
{
	return calculate_enc_persist_size();
}

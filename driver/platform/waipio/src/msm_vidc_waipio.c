// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/of.h>

#include "msm_vidc_waipio.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_control.h"
#include "hfi_property.h"

#define DDR_TYPE_LPDDR4 0x6
#define DDR_TYPE_LPDDR4X 0x7
#define DDR_TYPE_LPDDR5 0x8
#define DDR_TYPE_LPDDR5X 0x9

#define UBWC_CONFIG(mco, mlo, hbo, bslo, bso, rs, mc, ml, hbb, bsl, bsp) \
{	\
	.override_bit_info.max_channel_override = mco,	\
	.override_bit_info.mal_length_override = mlo,	\
	.override_bit_info.hb_override = hbo,	\
	.override_bit_info.bank_swzl_level_override = bslo,	\
	.override_bit_info.bank_spreading_override = bso,	\
	.override_bit_info.reserved = rs,	\
	.max_channels = mc,	\
	.mal_length = ml,	\
	.highest_bank_bit = hbb,	\
	.bank_swzl_level = bsl,	\
	.bank_spreading = bsp,	\
}

#define ENC     MSM_VIDC_ENCODER
#define DEC     MSM_VIDC_DECODER
#define H264    MSM_VIDC_H264
#define HEVC    MSM_VIDC_HEVC
#define VP9     MSM_VIDC_VP9
#define MPEG2   MSM_VIDC_MPEG2
#define CODECS_ALL     (MSM_VIDC_H264 | MSM_VIDC_HEVC | \
			MSM_VIDC_VP9 | MSM_VIDC_MPEG2)

static struct msm_platform_core_capability core_data_waipio[] = {
	/* {type, value} */
	{ENC_CODECS, H264|HEVC},
	{DEC_CODECS, H264|HEVC|VP9|MPEG2},
	{MAX_SESSION_COUNT, 16},
	{MAX_SECURE_SESSION_COUNT, 3},
	{MAX_MBPF, 173056},	/* (8192x4320)/256 + (4096x2176)/256*/
	{MAX_MBPS, 7833600},	/* max_load
					 * 7680x4320@60fps or 3840x2176@240fps
					 * which is greater than 4096x2176@120fps,
					 * 8192x4320@48fps
					 */
	{MAX_MBPF_HQ, 8160}, /* ((1920x1088)/256) */
	{MAX_MBPS_HQ, 489600}, /* ((1920x1088)/256)@60fps */
	{MAX_MBPF_B_FRAME, 32640}, /* 3840x2176/256 */
	{MAX_MBPS_B_FRAME, 1958400}, /* 3840x2176/256 MBs@60fps */
	{NUM_VPP_PIPE, 4},
	{SW_PC, 0},
	{SW_PC_DELAY, 1500}, /* 1500 ms */
	{FW_UNLOAD, 0},
	{FW_UNLOAD_DELAY, 1000}, /* 1000 ms */
	{HW_RESPONSE_TIMEOUT, 3000}, /* 1000 ms */
	{DEBUG_TIMEOUT, 0},
	{PREFIX_BUF_COUNT_PIX, 18},
	{PREFIX_BUF_SIZE_PIX, 13434880}, /* Calculated by VENUS_BUFFER_SIZE for 4096x2160 UBWC */
	{PREFIX_BUF_COUNT_NON_PIX, 1},
	{PREFIX_BUF_SIZE_NON_PIX, 209715200}, /*
		 * Internal buffer size is calculated for secure decode session
		 * of resolution 4k (4096x2160)
		 * Internal buf size = calculate_scratch_size() +
		 *	calculate_scratch1_size() + calculate_persist1_size()
		 * Take maximum between VP9 10bit, HEVC 10bit, AVC, MPEG2 secure
		 * decoder sessions
		 */
	{PAGEFAULT_NON_FATAL, 1},
	{PAGETABLE_CACHING, 0},
	{DCVS, 1},
	{DECODE_BATCH, 1},
	{DECODE_BATCH_TIMEOUT, 200},
	{AV_SYNC_WINDOW_SIZE, 40},
};

static struct msm_platform_inst_capability instance_data_waipio[] = {
	/* {cap, domain, codec,
	 *      min, max, step_or_mask, value,
	 *      v4l2_id, hfi_id,
	 *      flags,
	 *      parents,
	 *      children,
	 *      adjust, set}
	 */
	{CODEC},
	{FRAME_WIDTH, ENC|DEC, CODECS_ALL, 128, 8192, 1, 1920},
	{FRAME_HEIGHT, ENC|DEC, CODECS_ALL, 128, 8192, 1, 1080},
	{PIX_FMTS, ENC, CODECS_ALL,
		MSM_VIDC_FMT_NV12,
		MSM_VIDC_FMT_NV12_TP10_UBWC,
		MSM_VIDC_FMT_NV12 | MSM_VIDC_FMT_NV21 | MSM_VIDC_FMT_NV12_UBWC |
		MSM_VIDC_FMT_NV12_P010 | MSM_VIDC_FMT_NV12_TP10_UBWC,
		MSM_VIDC_FMT_NV12_UBWC},
	{PIX_FMTS, DEC, CODECS_ALL,
		MSM_VIDC_FMT_NV12,
		MSM_VIDC_FMT_NV12_TP10_UBWC,
		MSM_VIDC_FMT_NV12 | MSM_VIDC_FMT_NV21 | MSM_VIDC_FMT_NV12_UBWC |
		MSM_VIDC_FMT_NV12_P010 | MSM_VIDC_FMT_NV12_TP10_UBWC,
		MSM_VIDC_FMT_NV12_UBWC},
	{MIN_BUFFERS_INPUT, ENC|DEC, CODECS_ALL, 0, 64, 1, 4,
		V4L2_CID_MIN_BUFFERS_FOR_OUTPUT},
	{MIN_BUFFERS_OUTPUT, ENC|DEC, CODECS_ALL, 0, 64, 1, 4,
		V4L2_CID_MIN_BUFFERS_FOR_CAPTURE},
	{DECODE_ORDER, DEC, CODECS_ALL, 0, 1, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_DECODE_ORDER,
		HFI_PROP_DECODE_ORDER_OUTPUT},
	{THUMBNAIL_MODE, DEC, CODECS_ALL, 0, 1, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_SYNC_FRAME_DECODE,
		HFI_PROP_THUMBNAIL_MODE},
	{SECURE_MODE, ENC|DEC, CODECS_ALL, 0, 1, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_SECURE,
		HFI_PROP_SECURE},
	{LOWLATENCY_MODE, ENC|DEC, CODECS_ALL, 0, 1, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE},
	{LOWLATENCY_HINT, DEC, CODECS_ALL, 0, 1, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_HINT},
	{BUF_SIZE_LIMIT, ENC|DEC, CODECS_ALL, 0, 0x0fffffff, 1, 0,
		V4L2_CID_MPEG_VIDC_VIDEO_BUFFER_SIZE_LIMIT},

	/* (8192 * 4320) / 256 */
	{MBPF, ENC|DEC, CODECS_ALL, 64, 138240, 1, 138240},
	/* ((1920 * 1088) / 256) * 960 fps */
	{MBPS, ENC|DEC, CODECS_ALL, 64, 7833600, 1, 7833600},
	{FRAME_RATE, ENC|DEC, CODECS_ALL, 1, 960, 1, 30},

	{BIT_RATE, ENC|DEC, CODECS_ALL,
		1, 220000000, 1, 20000000,
		V4L2_CID_MPEG_VIDEO_BITRATE, HFI_PROP_TOTAL_BITRATE,
		CAP_FLAG_DYNAMIC_ALLOWED,
		/* TO DO parents */ {0},
		{LAYER_BITRATE, SLICE_BYTE},
		msm_vidc_adjust_bitrate, msm_vidc_set_s32},

	{BIT_RATE, ENC, HEVC,
		1, 160000000, 1, 20000000,
		V4L2_CID_MPEG_VIDEO_BITRATE, HFI_PROP_TOTAL_BITRATE,
		CAP_FLAG_DYNAMIC_ALLOWED,
		/* TO DO parents */{0},
		{LAYER_BITRATE, SLICE_BYTE},
		msm_vidc_adjust_bitrate, msm_vidc_set_s32},

	{ENTROPY_MODE, ENC, H264,
		V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
		V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC,
		BIT(V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC) |
		BIT(V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC),
		V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC,
		V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE, HFI_PROP_CABAC_SESSION,
		CAP_FLAG_MENU,
		{CODEC, PROFILE},
		{BIT_RATE},
		msm_vidc_adjust_entropy_mode, msm_vidc_set_u32},

	{PROFILE, ENC|DEC, H264,
		V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
		V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10,
		BIT(V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE) |
		BIT(V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE) |
		BIT(V4L2_MPEG_VIDEO_H264_PROFILE_MAIN) |
		BIT(V4L2_MPEG_VIDEO_H264_PROFILE_HIGH) |
		BIT(V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10),
		V4L2_MPEG_VIDEO_H264_PROFILE_HIGH,
		V4L2_CID_MPEG_VIDEO_H264_PROFILE, HFI_PROP_PROFILE,
		CAP_FLAG_ROOT | CAP_FLAG_MENU,
		{0},
		{ENTROPY_MODE},
		NULL, msm_vidc_set_u32},

	{LEVEL, ENC|DEC, H264,
		V4L2_MPEG_VIDEO_H264_LEVEL_1_0,
		V4L2_MPEG_VIDEO_H264_LEVEL_6_2,
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_1_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_1B) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_1_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_1_2) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_1_3) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_2_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_2_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_2_2) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_3_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_3_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_3_2) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_4_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_4_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_4_2) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_5_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_5_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_5_2) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_6_0) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_6_1) |
		BIT(V4L2_MPEG_VIDEO_H264_LEVEL_6_2),
		V4L2_MPEG_VIDEO_H264_LEVEL_6_2,
		V4L2_CID_MPEG_VIDEO_H264_LEVEL, HFI_PROP_LEVEL,
		/* TO DO(AS) */
		CAP_FLAG_ROOT | CAP_FLAG_MENU,
		{0},
		{0},
		NULL, msm_vidc_set_u32},

	{CABAC_BITRATE, ENC, H264, 1, 160000000, 1, 20000000},
	{VBV_DELAY, ENC, CODECS_ALL,
		0, 1000, 500, 0,
		V4L2_CID_MPEG_VIDEO_VBV_DELAY},

	{SCALE_X, ENC, CODECS_ALL, 8192, 65536, 1, 8192},
	{SCALE_Y, ENC, CODECS_ALL, 8192, 65536, 1, 8192},
	{SCALE_X, DEC, CODECS_ALL, 65536, 65536, 1, 65536},
	{SCALE_Y, DEC, CODECS_ALL, 65536, 65536, 1, 65536},
	{B_FRAME, ENC, H264|HEVC, 0, 1, 1, 0},
	{HIER_P_LAYERS, ENC, H264|HEVC, 0, 6, 1, 0},
	{LTR_COUNT, ENC, H264|HEVC, 0, 2, 1, 0},
	/* ((4096 * 2304) / 256) * 60 fps */
	{POWER_SAVE_MBPS, ENC, CODECS_ALL,
		0, 2211840, 1, 2211840},
	{I_FRAME_QP, ENC, H264|HEVC, 0, 51, 1, 10},
	{P_FRAME_QP, ENC, H264|HEVC, 0, 51, 1, 20},
	{B_FRAME_QP, ENC, H264|HEVC, 0, 51, 1, 20},
	{I_FRAME_QP, ENC, VP9, 0, 127, 1, 20},
	{P_FRAME_QP, ENC, VP9, 0, 127, 1, 40},
	{B_FRAME_QP, ENC, VP9, 0, 127, 1, 40},
	/* 10 slices */
	{SLICE_BYTE, ENC, H264|HEVC, 1, 10, 1, 10},
	{SLICE_MB, ENC, H264|HEVC, 1, 10, 1, 10},

	/* Mpeg2 decoder specific */
	{FRAME_WIDTH, DEC, MPEG2, 128, 1920, 1, 1920},
	{FRAME_HEIGHT, DEC, MPEG2, 128, 1920, 1, 1080},
	/* (1920 * 1088) / 256 */
	{MBPF, DEC, MPEG2, 64, 8160, 1, 8160},
	/* ((1920 * 1088) / 256) * 30*/
	{MBPS, DEC, MPEG2, 64, 244800, 1, 244800},
	{FRAME_RATE, DEC, MPEG2, 1, 30, 1, 30},
	{BIT_RATE, DEC, MPEG2, 1, 40000000, 1, 20000000},

	/* Secure usecase specific */
	{SECURE_FRAME_WIDTH, ENC|DEC, CODECS_ALL, 128, 4096, 1, 1920},
	{SECURE_FRAME_HEIGHT, ENC|DEC, CODECS_ALL, 128, 4096, 1, 1080},
	/* (4096 * 2304) / 256 */
	{SECURE_MBPF, ENC|DEC, CODECS_ALL, 64, 36864, 1, 36864},
	{SECURE_BIT_RATE, ENC|DEC, CODECS_ALL, 1, 40000000, 1, 20000000},

	/* Batch Mode Decode */
	{BATCH_MBPF, DEC, CODECS_ALL, 64, 34816, 1, 34816},
	/* (4096 * 2176) / 256 */
	{BATCH_FRAME_RATE, DEC, CODECS_ALL, 1, 120, 1, 120},

	/* Lossless encoding usecase specific */
	{LOSSLESS_FRAME_WIDTH, ENC, H264|HEVC, 128, 4096, 1, 1920},
	{LOSSLESS_FRAME_HEIGHT, ENC, H264|HEVC, 128, 4096, 1, 1080},
	/* (4096 * 2304) / 256 */
	{LOSSLESS_MBPF, ENC, H264|HEVC, 64, 36864, 1, 36864},

	/* All intra encoding usecase specific */
	{ALL_INTRA_FRAME_RATE, ENC, H264|HEVC, 1, 240, 1, 30},

	/* Image specific */
	{HEVC_IMAGE_FRAME_WIDTH, ENC, HEVC, 128, 512, 1, 512},
	{HEVC_IMAGE_FRAME_HEIGHT, ENC, HEVC, 128, 512, 1, 512},
	{HEIC_IMAGE_FRAME_WIDTH, ENC, HEVC, 512, 16384, 1, 16384},
	{HEIC_IMAGE_FRAME_HEIGHT, ENC, HEVC, 512, 16384, 1, 16384},

	{MB_CYCLES_VSP, ENC, CODECS_ALL, 25, 25, 1, 25},
	{MB_CYCLES_VPP, ENC, CODECS_ALL, 675, 675, 1, 675},
	{MB_CYCLES_LP, ENC, CODECS_ALL, 320, 320, 1, 320},
	{MB_CYCLES_VSP, DEC, CODECS_ALL, 25, 25, 1, 25},
	{MB_CYCLES_VSP, DEC, VP9, 60, 60, 1, 60},
	{MB_CYCLES_VPP, DEC, CODECS_ALL, 200, 200, 1, 200},
	{MB_CYCLES_LP, DEC, CODECS_ALL, 200, 200, 1, 200},
	{MB_CYCLES_FW, ENC|DEC, CODECS_ALL, 326389, 326389, 1, 326389},
	{MB_CYCLES_FW_VPP, ENC|DEC, CODECS_ALL, 44156, 44156, 1, 44156},
};

/*
 * Custom conversion coefficients for resolution: 176x144 negative
 * coeffs are converted to s4.9 format
 * (e.g. -22 converted to ((1 << 13) - 22)
 * 3x3 transformation matrix coefficients in s4.9 fixed point format
 */
static u32 vpe_csc_custom_matrix_coeff[MAX_MATRIX_COEFFS] = {
	440, 8140, 8098, 0, 460, 52, 0, 34, 463
};

/* offset coefficients in s9 fixed point format */
static u32 vpe_csc_custom_bias_coeff[MAX_BIAS_COEFFS] = {
	53, 0, 4
};

/* clamping value for Y/U/V([min,max] for Y/U/V) */
static u32 vpe_csc_custom_limit_coeff[MAX_LIMIT_COEFFS] = {
	16, 235, 16, 240, 16, 240
};

/* Default UBWC config for LPDDR5 */
static struct msm_vidc_ubwc_config_data ubwc_config_waipio[] = {
	UBWC_CONFIG(1, 1, 1, 0, 0, 0, 8, 32, 16, 0, 0),
};

static struct msm_vidc_platform_data waipio_data = {
	.core_data = core_data_waipio,
	.core_data_size = ARRAY_SIZE(core_data_waipio),
	.instance_data = instance_data_waipio,
	.instance_data_size = ARRAY_SIZE(instance_data_waipio),
	.csc_data.vpe_csc_custom_bias_coeff = vpe_csc_custom_bias_coeff,
	.csc_data.vpe_csc_custom_matrix_coeff = vpe_csc_custom_matrix_coeff,
	.csc_data.vpe_csc_custom_limit_coeff = vpe_csc_custom_limit_coeff,
	.ubwc_config = ubwc_config_waipio,
};

static int msm_vidc_init_data(struct msm_vidc_core *core)
{
	int rc = 0;
	struct msm_vidc_ubwc_config_data *ubwc_config;
	u32 ddr_type;

	if (!core || !core->platform) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s: initialize waipio data\n", __func__);

	ubwc_config = waipio_data.ubwc_config;
	ddr_type = of_fdt_get_ddrtype();
	if (ddr_type == -ENOENT)
		d_vpr_e("Failed to get ddr type, use LPDDR5\n");

	if (ddr_type == DDR_TYPE_LPDDR4 || ddr_type == DDR_TYPE_LPDDR4X)
		ubwc_config->highest_bank_bit = 0xf;
	d_vpr_h("%s: DDR Type 0x%x hbb 0x%x\n", __func__,
		ddr_type, ubwc_config->highest_bank_bit);

	core->platform->data = waipio_data;

	return rc;
}

int msm_vidc_init_platform_waipio(struct msm_vidc_core *core)
{
	int rc = 0;

	rc = msm_vidc_init_data(core);
	if (rc)
		return rc;

	return 0;
}

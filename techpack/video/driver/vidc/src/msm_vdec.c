// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <media/v4l2_vidc_extensions.h>
#include "msm_media_info.h"
#include <linux/v4l2-common.h>

#include "msm_vdec.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_power.h"
#include "msm_vidc_control.h"
#include "msm_vidc_memory.h"
#include "venus_hfi.h"
#include "hfi_packet.h"
/* TODO: update based on clips */
#define MAX_DEC_BATCH_SIZE 6
#define SKIP_BATCH_WINDOW 100

static const u32 msm_vdec_subscribe_for_psc_avc[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_CODED_FRAMES,
	HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
	HFI_PROP_PIC_ORDER_CNT_TYPE,
	HFI_PROP_PROFILE,
	HFI_PROP_LEVEL,
	HFI_PROP_SIGNAL_COLOR_INFO,
};

static const u32 msm_vdec_subscribe_for_psc_hevc[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
	HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
	HFI_PROP_PROFILE,
	HFI_PROP_LEVEL,
	HFI_PROP_TIER,
	HFI_PROP_SIGNAL_COLOR_INFO,
};

static const u32 msm_vdec_subscribe_for_psc_vp9[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
	HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
	HFI_PROP_PROFILE,
	HFI_PROP_LEVEL,
};

static const u32 msm_vdec_input_subscribe_for_properties[] = {
	HFI_PROP_NO_OUTPUT,
	HFI_PROP_SUBFRAME_INPUT,
};

static const u32 msm_vdec_output_subscribe_for_properties[] = {
	HFI_PROP_WORST_COMPRESSION_RATIO,
	HFI_PROP_WORST_COMPLEXITY_FACTOR,
	HFI_PROP_PICTURE_TYPE,
	HFI_PROP_DPB_LIST,
	HFI_PROP_CABAC_SESSION,
	HFI_PROP_FENCE,
};

static const u32 msm_vdec_internal_buffer_type[] = {
	MSM_VIDC_BUF_BIN,
	MSM_VIDC_BUF_COMV,
	MSM_VIDC_BUF_NON_COMV,
	MSM_VIDC_BUF_LINE,
};

struct msm_vdec_prop_type_handle {
	u32 type;
	int (*handle)(struct msm_vidc_inst *inst, enum msm_vidc_port_type port);
};

static int msm_vdec_codec_change(struct msm_vidc_inst *inst, u32 v4l2_codec)
{
	int rc = 0;
	bool create_inst_handler = false;

	if (!inst->codec)
		create_inst_handler = true;

	if (inst->codec && inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat == v4l2_codec)
		return 0;

	i_vpr_h(inst, "%s: codec changed from %s to %s\n",
		__func__, v4l2_pixelfmt_name(inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat),
		v4l2_pixelfmt_name(v4l2_codec));

	inst->codec = v4l2_codec_to_driver(v4l2_codec, __func__);
	inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat = v4l2_codec;
	rc = msm_vidc_update_debug_str(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		goto exit;

	if (create_inst_handler) {
		rc = msm_vidc_ctrl_handler_init(inst, true);
		if(rc)
			goto exit;
	} else {
		rc = msm_vidc_ctrl_handler_update(inst);
		if(rc)
			goto exit;
	}

	rc = msm_vidc_update_buffer_count(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_update_buffer_count(inst, OUTPUT_PORT);
	if (rc)
		return rc;

exit:
	return rc;
}

static int msm_vdec_set_bitstream_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	resolution = inst->fmts[INPUT_PORT].fmt.pix_mp.width << 16 |
		inst->fmts[INPUT_PORT].fmt.pix_mp.height;
	i_vpr_h(inst, "%s: width: %d height: %d\n", __func__,
			inst->fmts[INPUT_PORT].fmt.pix_mp.width,
			inst->fmts[INPUT_PORT].fmt.pix_mp.height);
	inst->subcr_params[port].bitstream_resolution = resolution;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BITSTREAM_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&resolution,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_linear_stride_scanline(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 stride_y, scanline_y, stride_uv, scanline_uv;
	u32 payload[2];

	if (inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat !=
		V4L2_PIX_FMT_NV12 &&
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat !=
		V4L2_PIX_FMT_VIDC_P010 &&
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat !=
		V4L2_PIX_FMT_NV21)
		return 0;

	stride_y = inst->fmts[OUTPUT_PORT].fmt.pix_mp.width;
	scanline_y = inst->fmts[OUTPUT_PORT].fmt.pix_mp.height;
	stride_uv = stride_y;
	scanline_uv = scanline_y / 2;

	payload[0] = stride_y << 16 | scanline_y;
	payload[1] = stride_uv << 16 | scanline_uv;
	i_vpr_h(inst, "%s: stride_y: %d scanline_y: %d "
		"stride_uv: %d, scanline_uv: %d", __func__,
		stride_y, scanline_y, stride_uv, scanline_uv);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LINEAR_STRIDE_SCANLINE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, OUTPUT_PORT),
			HFI_PAYLOAD_U64,
			&payload,
			sizeof(u64));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_crop_offsets(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 left_offset, top_offset, right_offset, bottom_offset;
	u32 payload[2] = {0};

	left_offset = inst->crop.left;
	top_offset = inst->crop.top;
	right_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.width -
		inst->crop.width);
	bottom_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.height -
		inst->crop.height);

	if (inst->psc_or_last_flag_discarded) {
		left_offset = 1;
		top_offset = 1;
		right_offset = 1;
		bottom_offset = 1;
		i_vpr_e(inst, "%s: set invalid crop offsets to receive ipsc\n", __func__);
	}

	payload[0] = left_offset << 16 | top_offset;
	payload[1] = right_offset << 16 | bottom_offset;
	i_vpr_h(inst, "%s: left_offset: %d top_offset: %d "
		"right_offset: %d bottom_offset: %d", __func__,
		left_offset, top_offset, right_offset, bottom_offset);
	inst->subcr_params[port].crop_offsets[0] = payload[0];
	inst->subcr_params[port].crop_offsets[1] = payload[1];
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_OFFSETS,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&payload,
			sizeof(u64));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_bit_depth(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 colorformat;
	u32 bitdepth = 8 << 16 | 8;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	colorformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	if (colorformat == V4L2_PIX_FMT_VIDC_P010 ||
	    colorformat == V4L2_PIX_FMT_VIDC_TP10C)
		bitdepth = 10 << 16 | 10;

	inst->subcr_params[port].bit_depth = bitdepth;
	msm_vidc_update_cap_value(inst, BIT_DEPTH, bitdepth, __func__);

	if (!inst->capabilities->cap[BIT_DEPTH].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_LUMA_CHROMA_BIT_DEPTH is not supported\n", __func__);
		return 0;
	}

	i_vpr_h(inst, "%s: bit depth: %#x", __func__, bitdepth);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&bitdepth,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}
//todo: enable when needed
/*
static int msm_vdec_set_cabac(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 cabac = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	cabac = inst->capabilities->cap[ENTROPY_MODE].value;
	inst->subcr_params[port].cabac = cabac;
	i_vpr_h(inst, "%s: entropy mode: %d", __func__, cabac);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CABAC_SESSION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&cabac,
			sizeof(u32));
	if (rc)
		i_vpr_e(inst, "%s: set property failed\n", __func__);

	return rc;
}
*/
static int msm_vdec_set_coded_frames(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 coded_frames = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (inst->capabilities->cap[CODED_FRAMES].value ==
			CODED_FRAMES_PROGRESSIVE)
		coded_frames = HFI_BITMASK_FRAME_MBS_ONLY_FLAG;
	inst->subcr_params[port].coded_frames = coded_frames;

	if (!inst->capabilities->cap[CODED_FRAMES].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_CODED_FRAMES is not supported\n", __func__);
		return 0;
	}

	i_vpr_h(inst, "%s: coded frames: %d", __func__, coded_frames);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CODED_FRAMES,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&coded_frames,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_min_output_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 min_output;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	min_output = inst->buffers.output.min_count;
	inst->subcr_params[port].fw_min_count = min_output;
	i_vpr_h(inst, "%s: firmware min output count: %d",
		__func__, min_output);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&min_output,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}
	return rc;
}

static int msm_vdec_set_picture_order_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 poc = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	inst->subcr_params[port].pic_order_cnt = poc;
	i_vpr_h(inst, "%s: picture order count: %d", __func__, poc);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIC_ORDER_CNT_TYPE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&poc,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_colorspace(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 primaries = MSM_VIDC_PRIMARIES_RESERVED;
	u32 matrix_coeff = MSM_VIDC_MATRIX_COEFF_RESERVED;
	u32 transfer_char = MSM_VIDC_TRANSFER_RESERVED;
	u32 full_range = V4L2_QUANTIZATION_DEFAULT;
	u32 colour_description_present_flag = 0;
	u32 video_signal_type_present_flag = 0, color_info = 0;
	/* Unspecified video format */
	u32 video_format = 5;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (inst->codec != MSM_VIDC_H264 &&
		inst->codec != MSM_VIDC_HEVC &&
		inst->codec != MSM_VIDC_HEIC)
		return 0;

	if (inst->fmts[port].fmt.pix_mp.colorspace != V4L2_COLORSPACE_DEFAULT ||
	    inst->fmts[port].fmt.pix_mp.ycbcr_enc != V4L2_YCBCR_ENC_DEFAULT ||
	    inst->fmts[port].fmt.pix_mp.xfer_func != V4L2_XFER_FUNC_DEFAULT) {
		colour_description_present_flag = 1;
		video_signal_type_present_flag = 1;
		primaries = v4l2_color_primaries_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.colorspace, __func__);
		matrix_coeff = v4l2_matrix_coeff_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.ycbcr_enc, __func__);
		transfer_char = v4l2_transfer_char_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.xfer_func, __func__);
	}

	if (inst->fmts[port].fmt.pix_mp.quantization !=
	    V4L2_QUANTIZATION_DEFAULT) {
		video_signal_type_present_flag = 1;
		full_range = inst->fmts[port].fmt.pix_mp.quantization ==
			V4L2_QUANTIZATION_FULL_RANGE ? 1 : 0;
	}

	color_info = (matrix_coeff & 0xFF) |
		((transfer_char << 8) & 0xFF00) |
		((primaries << 16) & 0xFF0000) |
		((colour_description_present_flag << 24) & 0x1000000) |
		((full_range << 25) & 0x2000000) |
		((video_format << 26) & 0x1C000000) |
		((video_signal_type_present_flag << 29) & 0x20000000);

	inst->subcr_params[port].color_info = color_info;
	i_vpr_h(inst, "%s: color info: %#x\n", __func__, color_info);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_SIGNAL_COLOR_INFO,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&color_info,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_profile(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 profile;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	profile = inst->capabilities->cap[PROFILE].value;
	inst->subcr_params[port].profile = profile;
	i_vpr_h(inst, "%s: profile: %d", __func__, profile);

	if (!inst->capabilities->cap[PROFILE].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_PROFILE is not supported\n", __func__);
		return 0;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_PROFILE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&profile,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_level(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 level;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	level = inst->capabilities->cap[LEVEL].value;
	inst->subcr_params[port].level = level;
	i_vpr_h(inst, "%s: level: %d", __func__, level);

	if (!inst->capabilities->cap[LEVEL].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_LEVEL is not supported\n", __func__);
		return 0;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_LEVEL,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&level,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_tier(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 tier;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	tier = inst->capabilities->cap[HEVC_TIER].value;
	inst->subcr_params[port].tier = tier;
	i_vpr_h(inst, "%s: tier: %d", __func__, tier);

	if (!inst->capabilities->cap[HEVC_TIER].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_TIER is not supported\n", __func__);
		return 0;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_TIER,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&tier,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_colorformat(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 pixelformat;
	enum msm_vidc_colorformat_type colorformat;
	u32 hfi_colorformat;

	pixelformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	colorformat = v4l2_colorformat_to_driver(pixelformat, __func__);
	hfi_colorformat = get_hfi_colorformat(inst, colorformat);
	i_vpr_h(inst, "%s: hfi colorformat: %d",
		__func__, hfi_colorformat);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_COLOR_FORMAT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, OUTPUT_PORT),
			HFI_PAYLOAD_U32,
			&hfi_colorformat,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_output_order(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 output_order = 0;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (inst->capabilities->cap[THUMBNAIL_MODE].value ||
		(inst->capabilities->cap[DISPLAY_DELAY_ENABLE].value &&
		!inst->capabilities->cap[DISPLAY_DELAY].value))
		output_order = 1;

	if (!inst->capabilities->cap[DISPLAY_DELAY_ENABLE].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_DECODE_ORDER_OUTPUT is not supported\n",
			__func__);
		return 0;
	}

	i_vpr_h(inst, "%s: output order: %d", __func__, output_order);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_DECODE_ORDER_OUTPUT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&output_order,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_rap_frame(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 rap_frame = true;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[RAP_FRAME].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_DEC_START_FROM_RAP_FRAME is not supported\n",
			__func__);
		return 0;
	}

	rap_frame = inst->capabilities->cap[RAP_FRAME].value;
	i_vpr_h(inst, "%s: start from rap frame: %d", __func__, rap_frame);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_DEC_START_FROM_RAP_FRAME,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&rap_frame,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_thumbnail_mode(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 thumbnail_mode = 0;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[THUMBNAIL_MODE].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_THUMBNAIL_MODE is not supported\n",
			__func__);
		return 0;
	}

	thumbnail_mode = inst->capabilities->cap[THUMBNAIL_MODE].value;
	i_vpr_h(inst, "%s: thumbnail mode: %d", __func__, thumbnail_mode);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_THUMBNAIL_MODE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&thumbnail_mode,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_conceal_color_8bit(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 conceal_color_8bit;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[CONCEAL_COLOR_8BIT].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_CONCEAL_COLOR_8BIT is not supported\n",
			__func__);
		return 0;
	}

	conceal_color_8bit = inst->capabilities->cap[CONCEAL_COLOR_8BIT].value;
	i_vpr_h(inst, "%s: conceal color 8bit: %#x",
		__func__, conceal_color_8bit);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CONCEAL_COLOR_8BIT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&conceal_color_8bit,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_conceal_color_10bit(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 conceal_color_10bit;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[CONCEAL_COLOR_10BIT].hfi_id) {
		i_vpr_h(inst, "%s: HFI_PROP_CONCEAL_COLOR_10BIT is not supported\n",
			__func__);
		return 0;
	}

	conceal_color_10bit = inst->capabilities->cap[CONCEAL_COLOR_10BIT].value;
	i_vpr_h(inst, "%s: conceal color 10bit: %#x",
		__func__, conceal_color_10bit);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CONCEAL_COLOR_10BIT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&conceal_color_10bit,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return rc;
}

static int msm_vdec_set_host_max_buf_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 count = DEFAULT_MAX_HOST_BUF_COUNT;

	if (is_image_session(inst))
		count = DEFAULT_MAX_HOST_BURST_BUF_COUNT;

	i_vpr_h(inst, "%s: count: %u port: %u\n", __func__, count, port);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_HOST_MAX_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&count,
			sizeof(u32));
	if (rc) {
		i_vpr_e(inst, "%s: set property failed\n", __func__);
		return rc;
	}

	return 0;
}

static int msm_vdec_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_set_stage(inst, STAGE);
	if (rc)
		return rc;

	rc = msm_vidc_set_pipe(inst, PIPE);
	if (rc)
		return rc;

	rc = msm_vdec_set_output_order(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_thumbnail_mode(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_rap_frame(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_conceal_color_8bit(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_conceal_color_10bit(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_host_max_buf_count(inst, INPUT_PORT);
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_set_output_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vdec_set_colorformat(inst);
	if (rc)
		return rc;

	rc = msm_vdec_set_linear_stride_scanline(inst);
	if (rc)
		return rc;

	rc = msm_vdec_set_host_max_buf_count(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_set_session_priority(inst, PRIORITY);
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 i = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_vdec_internal_buffer_type); i++) {
		rc = msm_vidc_get_internal_buffers(inst, msm_vdec_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_vdec_get_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_create_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(msm_vdec_internal_buffer_type); i++) {
		rc = msm_vidc_create_internal_buffers(inst, msm_vdec_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return 0;
}

static int msm_vdec_create_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return 0;
}

static int msm_vdec_queue_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(msm_vdec_internal_buffer_type); i++) {
		rc = msm_vidc_queue_internal_buffers(inst, msm_vdec_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return 0;
}

static int msm_vdec_queue_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return 0;
}

static int msm_vdec_release_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 i = 0;

	i_vpr_h(inst, "%s()\n",__func__);

	for (i = 0; i < ARRAY_SIZE(msm_vdec_internal_buffer_type); i++) {
		rc = msm_vidc_release_internal_buffers(inst, msm_vdec_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return 0;
}

static int msm_vdec_subscribe_input_port_settings_change(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i, j;
	u32 subscribe_psc_size;
	const u32 *psc;
	static const struct msm_vdec_prop_type_handle prop_type_handle_arr[] = {
		{HFI_PROP_BITSTREAM_RESOLUTION,          msm_vdec_set_bitstream_resolution   },
		{HFI_PROP_CROP_OFFSETS,                  msm_vdec_set_crop_offsets           },
		{HFI_PROP_LUMA_CHROMA_BIT_DEPTH,         msm_vdec_set_bit_depth              },
		{HFI_PROP_CODED_FRAMES,                  msm_vdec_set_coded_frames           },
		{HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,    msm_vdec_set_min_output_count       },
		{HFI_PROP_PIC_ORDER_CNT_TYPE,            msm_vdec_set_picture_order_count    },
		{HFI_PROP_SIGNAL_COLOR_INFO,             msm_vdec_set_colorspace             },
		{HFI_PROP_PROFILE,                       msm_vdec_set_profile                },
		{HFI_PROP_LEVEL,                         msm_vdec_set_level                  },
		{HFI_PROP_TIER,                          msm_vdec_set_tier                   },
	};

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	i_vpr_h(inst, "%s()\n", __func__);

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	if (inst->codec == MSM_VIDC_H264) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_avc);
		psc = msm_vdec_subscribe_for_psc_avc;
	} else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_hevc);
		psc = msm_vdec_subscribe_for_psc_hevc;
	} else if (inst->codec == MSM_VIDC_VP9) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_vp9);
		psc = msm_vdec_subscribe_for_psc_vp9;
	} else {
		i_vpr_e(inst, "%s: unsupported codec: %d\n", __func__, inst->codec);
		psc = NULL;
		return -EINVAL;
	}

	if (!psc || !subscribe_psc_size) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	for (i = 0; i < subscribe_psc_size; i++)
		payload[i + 1] = psc[i];
	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			((subscribe_psc_size + 1) *
			sizeof(u32)));

	for (i = 0; i < subscribe_psc_size; i++) {
		/* set session properties */
		for (j = 0; j < ARRAY_SIZE(prop_type_handle_arr); j++) {
			if (prop_type_handle_arr[j].type == psc[i]) {
				rc = prop_type_handle_arr[j].handle(inst, port);
				if (rc)
					goto exit;
				break;
			}
		}

		/* is property type unknown ? */
		if (j == ARRAY_SIZE(prop_type_handle_arr))
			i_vpr_e(inst, "%s: unknown property %#x\n", __func__, psc[i]);
	}

exit:
	return rc;
}

static int msm_vdec_subscribe_property(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 payload[32] = {0};
	u32 i, count = 0;
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	payload[0] = HFI_MODE_PROPERTY;

	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_vdec_input_subscribe_for_properties); i++) {
			payload[count + 1] = msm_vdec_input_subscribe_for_properties[i];
			count++;
		}
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_vdec_output_subscribe_for_properties); i++) {
			allow = msm_vidc_allow_property(inst,
				msm_vdec_output_subscribe_for_properties[i]);
			if (allow) {
				payload[count + 1] = msm_vdec_output_subscribe_for_properties[i];
				count++;
			}
			msm_vidc_update_property_cap(inst,
				msm_vdec_output_subscribe_for_properties[i], allow);
		}
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;
	return rc;
}

static int msm_vdec_subscribe_metadata(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 payload[32] = {0};
	u32 i, count = 0;
	struct msm_vidc_inst_capability *capability;
	const u32 metadata_input_list[] = {
		META_OUTBUF_FENCE,
		/*
		 * when fence enabled, client needs output buffer_tag
		 * in input metadata buffer done.
		 */
		META_BUF_TAG,
		META_PICTURE_TYPE,
	};
	const u32 metadata_output_list[] = {
		META_DPB_MISR,
		META_OPB_MISR,
		META_INTERLACE,
		META_TIMESTAMP,
		META_CONCEALED_MB_CNT,
		META_HIST_INFO,
		META_SEI_MASTERING_DISP,
		META_SEI_CLL,
		META_HDR10PLUS,
		/*
		 * client needs input buffer tag in output metadata buffer done.
		 */
		META_BUF_TAG,
		META_DPB_TAG_LIST,
		META_SUBFRAME_OUTPUT,
		META_DEC_QP_METADATA,
		META_MAX_NUM_REORDER_FRAMES,
	};

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;
	/*
	 * TODO
	 * Currently I/P port Meta data subscription is required only when
	 * SW fence enable case, so suppose if any new use-case needs I/P
	 * port Meta data subscription then need to remove below check.
	 */
	if (port == INPUT_PORT && !is_outbuf_fence_enabled(inst)) {
		i_vpr_l(inst, "%s: I/P port meta-data subscription not allowed!", __func__);
		return rc;
	}

	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(metadata_input_list); i++) {
			if (capability->cap[metadata_input_list[i]].value &&
				msm_vidc_allow_metadata(inst, metadata_input_list[i])) {
				payload[count + 1] =
					capability->cap[metadata_input_list[i]].hfi_id;
				count++;
			}
		}
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(metadata_output_list); i++) {
			if (capability->cap[metadata_output_list[i]].value &&
				msm_vidc_allow_metadata(inst, metadata_output_list[i])) {
				payload[count + 1] =
					capability->cap[metadata_output_list[i]].hfi_id;
				count++;
			}
		}
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;
	return rc;
}

static int msm_vdec_set_delivery_mode_metadata(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 payload[32] = {0};
	u32 i, count = 0;
	struct msm_vidc_inst_capability *capability;
	const u32 metadata_input_list[] = {
		META_BUF_TAG,
	};
	const u32 metadata_output_list[] = {
		META_OUTPUT_BUF_TAG,
	};

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;

	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(metadata_input_list); i++) {
			if (capability->cap[metadata_input_list[i]].value) {
				payload[count + 1] =
					capability->cap[metadata_input_list[i]].hfi_id;
				count++;
			}
		}
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(metadata_output_list); i++) {
			if (capability->cap[metadata_output_list[i]].value  &&
				msm_vidc_allow_metadata(inst, metadata_output_list[i])) {
				payload[count + 1] =
					capability->cap[metadata_output_list[i]].hfi_id;
				count++;
			}
		}
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;
	return rc;
}

static int msm_vdec_set_delivery_mode_property(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 payload[32] = {0};
	u32 i, count = 0;
	struct msm_vidc_inst_capability *capability;
	const u32 property_output_list[] = {
		META_OUTBUF_FENCE,
	};
	const u32 property_input_list[] = {};

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_PROPERTY;

	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(property_input_list); i++) {
			if (capability->cap[property_input_list[i]].value) {
				payload[count + 1] =
					capability->cap[property_input_list[i]].hfi_id;
				count++;
			}
		}
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(property_output_list); i++) {
			if (capability->cap[property_output_list[i]].value) {
				payload[count + 1] =
					capability->cap[property_output_list[i]].hfi_id;
				count++;
			}
		}
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_session_resume(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;

	msm_vidc_scale_power(inst, true);

	i_vpr_h(inst, "%s()\n", __func__);
	rc = venus_hfi_session_command(inst,
			HFI_CMD_RESUME,
			port,
			HFI_PAYLOAD_NONE,
			NULL,
			0);
	if (rc)
		return rc;

	return rc;
}

int msm_vdec_init_input_subcr_params(struct msm_vidc_inst *inst)
{
	struct msm_vidc_subscription_params *subsc_params;
	u32 left_offset, top_offset, right_offset, bottom_offset;
	u32 primaries, matrix_coeff, transfer_char;
	u32 full_range = 0, video_format = 0;
	u32 colour_description_present_flag = 0;
	u32 video_signal_type_present_flag = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	subsc_params = &inst->subcr_params[INPUT_PORT];

	subsc_params->bitstream_resolution =
		inst->fmts[INPUT_PORT].fmt.pix_mp.width << 16 |
		inst->fmts[INPUT_PORT].fmt.pix_mp.height;

	left_offset = inst->crop.left;
	top_offset = inst->crop.top;
	right_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.width -
			inst->crop.width);
	bottom_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.height -
			inst->crop.height);
	subsc_params->crop_offsets[0] =
			left_offset << 16 | top_offset;
	subsc_params->crop_offsets[1] =
			right_offset << 16 | bottom_offset;

	subsc_params->fw_min_count = inst->buffers.output.min_count;

	primaries = v4l2_color_primaries_to_driver(inst,
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace, __func__);
	matrix_coeff = v4l2_matrix_coeff_to_driver(inst,
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.ycbcr_enc, __func__);
	transfer_char = v4l2_transfer_char_to_driver(inst,
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.xfer_func, __func__);
	full_range = inst->fmts[OUTPUT_PORT].fmt.pix_mp.quantization ==
		V4L2_QUANTIZATION_FULL_RANGE ? 1 : 0;
	subsc_params->color_info =
		(matrix_coeff & 0xFF) |
		((transfer_char << 8) & 0xFF00) |
		((primaries << 16) & 0xFF0000) |
		((colour_description_present_flag << 24) & 0x1000000) |
		((full_range << 25) & 0x2000000) |
		((video_format << 26) & 0x1C000000) |
		((video_signal_type_present_flag << 29) & 0x20000000);

	subsc_params->profile = inst->capabilities->cap[PROFILE].value;
	subsc_params->level = inst->capabilities->cap[LEVEL].value;
	subsc_params->tier = inst->capabilities->cap[HEVC_TIER].value;
	subsc_params->pic_order_cnt = inst->capabilities->cap[POC].value;
	subsc_params->bit_depth = inst->capabilities->cap[BIT_DEPTH].value;
	if (inst->capabilities->cap[CODED_FRAMES].value ==
			CODED_FRAMES_PROGRESSIVE)
		subsc_params->coded_frames = HFI_BITMASK_FRAME_MBS_ONLY_FLAG;
	else
		subsc_params->coded_frames = 0;

	return 0;
}

static int msm_vdec_read_input_subcr_params(struct msm_vidc_inst *inst)
{
	struct msm_vidc_subscription_params subsc_params;
	struct msm_vidc_core *core;
	u32 width, height;
	u32 primaries, matrix_coeff, transfer_char;
	u32 full_range = 0;
	u32 colour_description_present_flag = 0;
	u32 video_signal_type_present_flag = 0;
	u32 bit_depth;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	subsc_params = inst->subcr_params[INPUT_PORT];
	width = (subsc_params.bitstream_resolution &
		HFI_BITMASK_BITSTREAM_WIDTH) >> 16;
	height = subsc_params.bitstream_resolution &
		HFI_BITMASK_BITSTREAM_HEIGHT;

	inst->fmts[INPUT_PORT].fmt.pix_mp.width = width;
	inst->fmts[INPUT_PORT].fmt.pix_mp.height = height;

	inst->fmts[OUTPUT_PORT].fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, width);
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.height = VIDEO_Y_SCANLINES(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, height);
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.plane_fmt[0].bytesperline =
		VIDEO_Y_STRIDE_BYTES(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, width);
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.plane_fmt[0].sizeimage =
		call_session_op(core, buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	//inst->buffers.output.size = inst->fmts[OUTPUT_PORT].fmt.pix_mp.plane_fmt[0].sizeimage;

	matrix_coeff = subsc_params.color_info & 0xFF;
	transfer_char = (subsc_params.color_info & 0xFF00) >> 8;
	primaries = (subsc_params.color_info & 0xFF0000) >> 16;
	colour_description_present_flag =
		(subsc_params.color_info & 0x1000000) >> 24;
	full_range = (subsc_params.color_info & 0x2000000) >> 25;
	video_signal_type_present_flag =
		(subsc_params.color_info & 0x20000000) >> 29;

	inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.xfer_func = V4L2_XFER_FUNC_DEFAULT;
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.quantization = V4L2_QUANTIZATION_DEFAULT;

	if (video_signal_type_present_flag) {
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.quantization =
			full_range ?
			V4L2_QUANTIZATION_FULL_RANGE :
			V4L2_QUANTIZATION_LIM_RANGE;
		if (colour_description_present_flag) {
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace =
				v4l2_color_primaries_from_driver(inst, primaries, __func__);
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.xfer_func =
				v4l2_transfer_char_from_driver(inst, transfer_char, __func__);
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.ycbcr_enc =
				v4l2_matrix_coeff_from_driver(inst, matrix_coeff, __func__);
		} else {
			i_vpr_h(inst,
				"%s: color description flag is not present\n",
				__func__);
		}
	} else {
		i_vpr_h(inst, "%s: video_signal type is not present\n",
			__func__);
	}

	inst->buffers.output.min_count = subsc_params.fw_min_count;
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	if (is_thumbnail_session(inst) && inst->codec != MSM_VIDC_VP9) {
		if (inst->buffers.output.min_count != 1) {
			i_vpr_e(inst, "%s: invalid min count %d in thumbnail case\n",
				__func__, inst->buffers.output.min_count);
			msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		}
	}
	inst->crop.top = subsc_params.crop_offsets[0] & 0xFFFF;
	inst->crop.left = (subsc_params.crop_offsets[0] >> 16) & 0xFFFF;
	inst->crop.height = inst->fmts[INPUT_PORT].fmt.pix_mp.height -
		(subsc_params.crop_offsets[1] & 0xFFFF);
	inst->crop.width = inst->fmts[INPUT_PORT].fmt.pix_mp.width -
		((subsc_params.crop_offsets[1] >> 16) & 0xFFFF);

	msm_vidc_update_cap_value(inst, PROFILE, subsc_params.profile, __func__);
	msm_vidc_update_cap_value(inst, LEVEL, subsc_params.level, __func__);
	msm_vidc_update_cap_value(inst, HEVC_TIER, subsc_params.tier, __func__);
	msm_vidc_update_cap_value(inst, POC, subsc_params.pic_order_cnt, __func__);

	bit_depth = (subsc_params.bit_depth == BIT_DEPTH_8) ?
		BIT_DEPTH_8 : BIT_DEPTH_10;

	if (bit_depth < inst->capabilities->cap[BIT_DEPTH].min ||
		bit_depth > inst->capabilities->cap[BIT_DEPTH].max) {
		i_vpr_e(inst, "%s: invalid bit_depth %d\n",
			__func__, bit_depth);
		return -EINVAL;
	}
	msm_vidc_update_cap_value(inst, BIT_DEPTH, bit_depth, __func__);

	if (subsc_params.coded_frames & HFI_BITMASK_FRAME_MBS_ONLY_FLAG)
		msm_vidc_update_cap_value(inst, CODED_FRAMES, CODED_FRAMES_PROGRESSIVE, __func__);
	else
		msm_vidc_update_cap_value(inst, CODED_FRAMES, CODED_FRAMES_INTERLACE, __func__);

	/* disable META_OUTBUF_FENCE if session is Interlace type */
	if (inst->capabilities->cap[CODED_FRAMES].value ==
		CODED_FRAMES_INTERLACE) {
		msm_vidc_update_cap_value(inst, META_OUTBUF_FENCE,
			V4L2_MPEG_MSM_VIDC_DISABLE, __func__);
	}

	return 0;
}

int msm_vdec_input_port_settings_change(struct msm_vidc_inst *inst)
{
	u32 rc = 0;
	struct v4l2_event event = {0};

	if (!inst->vb2q[INPUT_PORT].streaming) {
		i_vpr_e(inst, "%s: input port not streaming\n",
			__func__);
		return 0;
	}

	rc = msm_vdec_read_input_subcr_params(inst);
	if (rc)
		return rc;

	event.type = V4L2_EVENT_SOURCE_CHANGE;
	event.u.src_change.changes = V4L2_EVENT_SRC_CH_RESOLUTION;
	v4l2_event_queue_fh(&inst->event_handler, &event);

	rc = msm_vdec_get_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_release_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_create_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_queue_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vidc_set_stage(inst, STAGE);
	if (rc)
		return rc;

	rc = msm_vidc_set_pipe(inst, PIPE);
	if (rc)
		return rc;

	rc = msm_vdec_session_resume(inst, INPUT_PORT);
	if (rc)
		return rc;

	return rc;
}

int msm_vdec_output_port_settings_change(struct msm_vidc_inst *inst)
{
	//todo
	return 0;
}

int msm_vdec_streamoff_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_streamoff(inst, INPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_streamon_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_input_meta_enabled(inst) &&
		!inst->vb2q[INPUT_META_PORT].streaming) {
		i_vpr_e(inst,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;

	rc = msm_vdec_set_input_properties(inst);
	if (rc)
		goto error;

	/* Decide bse vpp delay after work mode */
	//msm_vidc_set_bse_vpp_delay(inst);

	rc = msm_vdec_get_input_internal_buffers(inst);
	if (rc)
		goto error;
	/* check for memory after all buffers calculation */
	//rc = msm_vidc_check_memory_supported(inst);
	if (rc)
		goto error;

	rc = msm_vdec_create_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vdec_queue_input_internal_buffers(inst);
	if (rc)
		goto error;

	if (!inst->ipsc_properties_set) {
		rc = msm_vdec_subscribe_input_port_settings_change(
			inst, INPUT_PORT);
		if (rc)
			return rc;
		inst->ipsc_properties_set = true;
	}

	/**
	 * we need to start fw output port if ipsc / opsc / last flag discarded by driver
	 * also set invalid crop for fw to raise another ipsc after streamon
	 */
	if (inst->psc_or_last_flag_discarded) {
		i_vpr_e(inst, "%s: psc or last flag discarded adjustments\n", __func__);
		rc = msm_vdec_subscribe_input_port_settings_change(inst, INPUT_PORT);
		if (rc)
			return rc;

		memcpy(&inst->subcr_params[OUTPUT_PORT],
				&inst->subcr_params[INPUT_PORT],
				sizeof(inst->subcr_params[OUTPUT_PORT]));
		rc = msm_vdec_subscribe_output_port_settings_change(inst, OUTPUT_PORT);
		if (rc)
			return rc;

		/* resume output only if port is enabled */
		if (inst->vb2q[OUTPUT_PORT].streaming) {
			rc = msm_vdec_session_resume(inst, OUTPUT_PORT);
			if (rc)
				return rc;
		}
		inst->psc_or_last_flag_discarded = false;
	}

	rc = msm_vdec_subscribe_property(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_subscribe_metadata(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_delivery_mode_metadata(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_session_streamon(inst, INPUT_PORT);
	if (rc)
		goto error;

	rc = msm_vidc_flush_ts(inst);
	if (rc)
		goto error;

	rc = msm_vidc_ts_reorder_flush(inst);
	if (rc)
		goto error;

	return 0;

error:
	i_vpr_e(inst, "%s: failed\n", __func__);
	msm_vdec_streamoff_input(inst);
	return rc;
}

static int schedule_batch_work(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	mod_delayed_work(core->batch_workq, &inst->decode_batch.work,
		msecs_to_jiffies(core->capabilities[DECODE_BATCH_TIMEOUT].value));

	return 0;
}

static int cancel_batch_work(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: Invalid arguments\n", __func__);
		return -EINVAL;
	}
	cancel_delayed_work(&inst->decode_batch.work);

	return 0;
}

int msm_vdec_streamoff_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/* cancel pending batch work */
	cancel_batch_work(inst);
	rc = msm_vidc_session_streamoff(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_subscribe_output_port_settings_change(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 payload[32] = {0};
	u32 prop_type, payload_size, payload_type;
	u32 i;
	struct msm_vidc_subscription_params subsc_params;
	u32 subscribe_psc_size = 0;
	const u32 *psc = NULL;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	if (inst->codec == MSM_VIDC_H264) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_avc);
		psc = msm_vdec_subscribe_for_psc_avc;
	} else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_hevc);
		psc = msm_vdec_subscribe_for_psc_hevc;
	} else if (inst->codec == MSM_VIDC_VP9) {
		subscribe_psc_size = ARRAY_SIZE(msm_vdec_subscribe_for_psc_vp9);
		psc = msm_vdec_subscribe_for_psc_vp9;
	} else {
		i_vpr_e(inst, "%s: unsupported codec: %d\n", __func__, inst->codec);
		psc = NULL;
		return -EINVAL;
	}

	if (!psc || !subscribe_psc_size) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	for (i = 0; i < subscribe_psc_size; i++)
		payload[i + 1] = psc[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			((subscribe_psc_size + 1) *
			sizeof(u32)));

	subsc_params = inst->subcr_params[port];
	for (i = 0; i < subscribe_psc_size; i++) {
		payload[0] = 0;
		payload[1] = 0;
		payload_size = 0;
		payload_type = 0;
		prop_type = psc[i];
		switch (prop_type) {
		case HFI_PROP_BITSTREAM_RESOLUTION:
			payload[0] = subsc_params.bitstream_resolution;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_CROP_OFFSETS:
			payload[0] = subsc_params.crop_offsets[0];
			payload[1] = subsc_params.crop_offsets[1];
			payload_size = sizeof(u64);
			payload_type = HFI_PAYLOAD_64_PACKED;
			break;
		case HFI_PROP_LUMA_CHROMA_BIT_DEPTH:
			payload[0] = subsc_params.bit_depth;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_CODED_FRAMES:
			payload[0] = subsc_params.coded_frames;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT:
			payload[0] = subsc_params.fw_min_count;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_PIC_ORDER_CNT_TYPE:
			payload[0] = subsc_params.pic_order_cnt;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_SIGNAL_COLOR_INFO:
			payload[0] = subsc_params.color_info;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_PROFILE:
			payload[0] = subsc_params.profile;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_LEVEL:
			payload[0] = subsc_params.level;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		case HFI_PROP_TIER:
			payload[0] = subsc_params.tier;
			payload_size = sizeof(u32);
			payload_type = HFI_PAYLOAD_U32;
			break;
		default:
			i_vpr_e(inst, "%s: unknown property %#x\n", __func__,
				prop_type);
			prop_type = 0;
			rc = -EINVAL;
			break;
		}
		if (prop_type) {
			rc = venus_hfi_session_property(inst,
					prop_type,
					HFI_HOST_FLAGS_NONE,
					get_hfi_port(inst, port),
					payload_type,
					&payload,
					payload_size);
			if (rc)
				return rc;
		}
	}

	return rc;
}

static int msm_vdec_update_max_map_output_count(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct v4l2_format *f;
	u32 width, height, count;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	/*
	 * adjust max map output count based on resolution
	 * to enhance performance.
	 * For 8K session: count = 20
	 * For 4K session: count = 32
	 * For 1080p session: count = 48
	 * For all remaining sessions: count = 64
	 */
	if (res_is_greater_than(width, height, 4096, 2160))
		count = 20;
	else if (res_is_greater_than(width, height, 1920, 1080))
		count = 32;
	else if (res_is_greater_than(width, height, 1280, 720))
		count = 48;
	else
		count = 64;

	inst->max_map_output_count = count;
	i_vpr_h(inst, "%s: count: %d\n", __func__, inst->max_map_output_count);

	return rc;
}

int msm_vdec_streamon_output(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (is_output_meta_enabled(inst) &&
		!inst->vb2q[OUTPUT_META_PORT].streaming) {
		i_vpr_e(inst,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	if (capability->cap[CODED_FRAMES].value == CODED_FRAMES_INTERLACE &&
		is_ubwc_supported_platform(inst) &&
		!is_ubwc_colorformat(capability->cap[PIX_FMTS].value)) {
		i_vpr_e(inst,
			"%s: interlace with non-ubwc color format is unsupported for ubwc supported platforms\n",
			__func__);
		return -EINVAL;
	}

	rc = msm_vidc_check_session_supported(inst);
	if (rc)
		return rc;

	rc = msm_vdec_update_max_map_output_count(inst);
	if (rc)
		return rc;

	rc = msm_vdec_set_output_properties(inst);
	if (rc)
		goto error;

	if (!inst->opsc_properties_set) {
		memcpy(&inst->subcr_params[OUTPUT_PORT],
			   &inst->subcr_params[INPUT_PORT],
			   sizeof(inst->subcr_params[INPUT_PORT]));
		rc = msm_vdec_subscribe_output_port_settings_change(inst, OUTPUT_PORT);
		if (rc)
			goto error;
		inst->opsc_properties_set = true;
	}

	rc = msm_vdec_subscribe_property(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_subscribe_metadata(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_vdec_set_delivery_mode_property(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_delivery_mode_metadata(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_get_output_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vdec_create_output_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vidc_session_streamon(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_vdec_queue_output_internal_buffers(inst);
	if (rc)
		goto error;

	return 0;

error:
	i_vpr_e(inst, "%s: failed\n", __func__);
	msm_vdec_streamoff_output(inst);
	return rc;
}

static inline enum msm_vidc_allow msm_vdec_allow_queue_deferred_buffers(
	struct msm_vidc_inst *inst)
{
	int count;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return MSM_VIDC_DISALLOW;
	}

	/* do not defer buffers initially to avoid latency issues */
	if (inst->power.buffer_counter <= SKIP_BATCH_WINDOW)
		return MSM_VIDC_ALLOW;

	/* do not defer, if client waiting for last flag FBD */
	if (inst->state != MSM_VIDC_START)
		return MSM_VIDC_ALLOW;

	/* defer qbuf, if pending buffers count less than batch size */
	count = msm_vidc_num_buffers(inst, MSM_VIDC_BUF_OUTPUT, MSM_VIDC_ATTR_DEFERRED);
	if (count < inst->decode_batch.size)
		return MSM_VIDC_DEFER;

	return MSM_VIDC_ALLOW;
}

static int msm_vdec_qbuf_batch(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2)
{
	struct msm_vidc_buffer *buf;
	enum msm_vidc_allow allow;
	int rc;

	if (!inst || !vb2 || !inst->decode_batch.size) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buf = msm_vidc_get_driver_buf(inst, vb2);
	if (!buf)
		return -EINVAL;

	allow = msm_vidc_allow_qbuf(inst, vb2->type);
	if (allow == MSM_VIDC_DISALLOW) {
		i_vpr_e(inst, "%s: qbuf not allowed\n", __func__);
		return -EINVAL;
	} else if (allow == MSM_VIDC_DEFER) {
		print_vidc_buffer(VIDC_LOW, "low ", "qbuf deferred", inst, buf);
		return 0;
	}

	allow = msm_vdec_allow_queue_deferred_buffers(inst);
	if (allow == MSM_VIDC_DISALLOW) {
		i_vpr_e(inst, "%s: queue deferred buffers not allowed\n", __func__);
		return -EINVAL;
	} else if (allow == MSM_VIDC_DEFER) {
		print_vidc_buffer(VIDC_LOW, "low ", "batch-qbuf deferred", inst, buf);
		schedule_batch_work(inst);
		return 0;
	}

	cancel_batch_work(inst);
	rc = msm_vidc_queue_deferred_buffers(inst, MSM_VIDC_BUF_OUTPUT);
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_release_nonref_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 fw_ro_count = 0, nonref_ro_count = 0;
	struct msm_vidc_buffer *ro_buf, *rel_buf, *dummy;
	int i = 0;
	bool found = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/* count num buffers in read_only list */
	list_for_each_entry(ro_buf, &inst->buffers.read_only.list, list)
		fw_ro_count++;

	if (fw_ro_count <= MAX_DPB_COUNT)
		return 0;

	/*
	 * Mark those buffers present in read_only list as non-reference
	 * if that buffer is not part of dpb_list_payload
	 * count such non-ref read only buffers as nonref_ro_count
	 * dpb_list_payload details:
	 * payload[0-1]           : 64 bits base_address of DPB-1
	 * payload[2]             : 32 bits addr_offset  of DPB-1
	 * payload[3]             : 32 bits data_offset  of DPB-1
	 */
	list_for_each_entry(ro_buf, &inst->buffers.read_only.list, list) {
		found = false;
		for (i = 0; (i + 3) < MAX_DPB_LIST_ARRAY_SIZE; i = i + 4) {
			if (ro_buf->device_addr == inst->dpb_list_payload[i] &&
				ro_buf->data_offset == inst->dpb_list_payload[i + 3]) {
				found = true;
				break;
			}
		}
		if (!found) {
			ro_buf->attr &= ~MSM_VIDC_ATTR_READ_ONLY;
			nonref_ro_count++;
		}
	}

	if (nonref_ro_count <= inst->buffers.output.min_count)
		return 0;

	i_vpr_l(inst, "%s: fw ro buf count %d, non-ref ro count %d\n",
		__func__, fw_ro_count, nonref_ro_count);
	/*
	 * move non-ref read only buffers from read_only list to
	 * release list
	 */
	list_for_each_entry_safe(ro_buf, dummy, &inst->buffers.read_only.list, list) {
		if (!(ro_buf->attr & MSM_VIDC_ATTR_READ_ONLY)) {
			list_del(&ro_buf->list);
			INIT_LIST_HEAD(&ro_buf->list);
			list_add_tail(&ro_buf->list, &inst->buffers.release.list);
		}
	}

	/* send release flag along with read only flag for release list bufs*/
	list_for_each_entry(rel_buf, &inst->buffers.release.list, list) {
		/* do not release already pending release buffers */
		if (rel_buf->attr & MSM_VIDC_ATTR_PENDING_RELEASE)
			continue;

		/* fw needs RO flag for FTB release buffer */
		rel_buf->attr |= MSM_VIDC_ATTR_READ_ONLY;
		print_vidc_buffer(VIDC_LOW, "low ", "release buf", inst, rel_buf);
		rc = venus_hfi_release_buffer(inst, rel_buf);
		if (rc)
			return rc;

		/* mark pending release */
		rel_buf->attr |= MSM_VIDC_ATTR_PENDING_RELEASE;
	}

	return rc;
}

int msm_vdec_handle_release_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	/**
	 * RO & release list doesnot take dma ref_count using dma_buf_get().
	 * Dmabuf ptr willbe obsolete when its last ref was last.
	 * Use direct api to print logs instead of calling print_vidc_buffer()
	 * api, which will attempt to dereferrence dmabuf ptr.
	 */
	i_vpr_l(inst,
		"release done: %s: idx %2d fd %3d off %d daddr %#llx size %8d filled %8d flags %#x ts %8lld attr %#x counts(etb ebd ftb fbd) %4llu %4llu %4llu %4llu\n",
		buf_name(buf->type),
		buf->index, buf->fd, buf->data_offset,
		buf->device_addr, buf->buffer_size, buf->data_size,
		buf->flags, buf->timestamp, buf->attr, inst->debug_count.etb,
		inst->debug_count.ebd, inst->debug_count.ftb, inst->debug_count.fbd);
	/* delete the buffer from release list */
	list_del(&buf->list);
	msm_memory_free(inst, buf);

	return rc;
}

static bool is_valid_removable_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_map *map)
{
	bool found = false;
	struct msm_vidc_buffer *buf;

	if (!inst || !map) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (map->refcount != 1)
		return false;

	list_for_each_entry(buf, &inst->buffers.read_only.list, list) {
		if (map->device_addr == buf->device_addr) {
			found = true;
			break;
		}
	}

	list_for_each_entry(buf, &inst->buffers.release.list, list) {
		if (map->device_addr == buf->device_addr) {
			found = true;
			break;
		}
	}

	if (!found)
		return true;

	return false;
}

static int msm_vidc_unmap_excessive_mappings(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_map *map, *temp;
	u32 refcount_one_bufs_count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/*
	 * count entries from map list which are not present in
	 * read_only buffers list, not present in release list
	 * and whose refcount is 1.these are excess mappings
	 * present due to delayed unmap feature.
	 */
	list_for_each_entry(map, &inst->mappings.output.list, list) {
		if (is_valid_removable_buffer(inst, map))
			refcount_one_bufs_count++;
	}

	if (refcount_one_bufs_count <= inst->max_map_output_count)
		return 0;

	/* unmap these buffers as they are stale entries */
	list_for_each_entry_safe(map, temp, &inst->mappings.output.list, list) {
		if (is_valid_removable_buffer(inst, map)) {
			i_vpr_l(inst,
				"%s: type %11s, device_addr %#x, refcount %d, region %d\n",
				__func__, buf_name(map->type), map->device_addr,
				map->refcount, map->region);
			rc = msm_vidc_put_delayed_unmap(inst, map);
			if (rc)
				return rc;
		}
	}
	return rc;
}

int msm_vdec_qbuf(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;

	if (!inst || !vb2 || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (vb2->type == OUTPUT_MPLANE) {
		if (inst->capabilities->cap[DPB_LIST].value) {
			rc = msm_vdec_release_nonref_buffers(inst);
			if (rc)
				return rc;
		}
	}

	if (inst->firmware_priority != (inst->priority_level +
					inst->capabilities->cap[PRIORITY].value * 2))
		msm_vidc_set_session_priority(inst, PRIORITY);

	/* batch decoder output & meta buffer only */
	if (inst->decode_batch.enable && vb2->type == OUTPUT_MPLANE)
		rc = msm_vdec_qbuf_batch(inst, vb2);
	else
		rc = msm_vidc_queue_buffer_single(inst, vb2);
	if (rc)
		return rc;

	if (vb2->type == OUTPUT_MPLANE) {
		rc = msm_vidc_unmap_excessive_mappings(inst);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_vdec_alloc_and_queue_additional_dpb_buffers(struct msm_vidc_inst *inst)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buffer = NULL;
	int i, cur_min_count = 0, rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/* get latest min_count and size */
	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_DPB, __func__);
	if (!buffers)
		return -EINVAL;

	/* get current min_count */
	list_for_each_entry(buffer, &buffers->list, list)
		cur_min_count++;

	/* skip alloc and queue */
	if (cur_min_count >= buffers->min_count)
		return 0;

	i_vpr_h(inst, "%s: dpb buffer count increased from %u -> %u\n",
		__func__, cur_min_count, buffers->min_count);

	/* allocate additional DPB buffers */
	for (i = cur_min_count; i < buffers->min_count; i++) {
		rc = msm_vidc_create_internal_buffer(inst, MSM_VIDC_BUF_DPB, i);
		if (rc)
			return rc;
	}

	/* queue additional DPB buffers */
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_process_cmd(struct msm_vidc_inst *inst, u32 cmd)
{
	int rc = 0;
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;
	enum msm_vidc_port_type port;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (cmd == V4L2_DEC_CMD_STOP) {
		i_vpr_h(inst, "received cmd: drain\n");
		allow = msm_vidc_allow_stop(inst);
		if (allow == MSM_VIDC_DISALLOW)
			return -EBUSY;
		else if (allow == MSM_VIDC_IGNORE)
			return 0;
		else if (allow != MSM_VIDC_ALLOW)
			return -EINVAL;

		msm_vidc_scale_power(inst, true);
		rc = venus_hfi_session_command(inst,
				HFI_CMD_DRAIN,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
		rc = msm_vidc_state_change_stop(inst);
		if (rc)
			return rc;
	} else if (cmd == V4L2_DEC_CMD_START) {
		i_vpr_h(inst, "received cmd: resume\n");
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_META_PORT]);
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_PORT]);

		if (capability->cap[CODED_FRAMES].value == CODED_FRAMES_INTERLACE &&
			is_ubwc_supported_platform(inst) &&
			!is_ubwc_colorformat(capability->cap[PIX_FMTS].value)) {
			i_vpr_e(inst,
				"%s: interlace with non-ubwc color format is unsupported for ubwc supported platforms\n",
				__func__);
			return -EINVAL;
		}

		if (!msm_vidc_allow_start(inst))
			return -EBUSY;
		port = (inst->state == MSM_VIDC_DRAIN_LAST_FLAG) ? INPUT_PORT : OUTPUT_PORT;

		rc = msm_vidc_state_change_start(inst);
		if (rc)
			return rc;

		/* tune power features */
		inst->decode_batch.enable = msm_vidc_allow_decode_batch(inst);
		msm_vidc_allow_dcvs(inst);
		msm_vidc_power_data_reset(inst);

		/* allocate and queue extra dpb buffers */
		rc = msm_vdec_alloc_and_queue_additional_dpb_buffers(inst);
		if (rc)
			return rc;

		/* print final buffer counts & size details */
		msm_vidc_print_buffer_info(inst);

		rc = msm_vdec_session_resume(inst, port);
		if (rc)
			return rc;

	} else {
		i_vpr_e(inst, "%s: unknown cmd %d\n", __func__, cmd);
		return -EINVAL;
	}

	return 0;
}

int msm_vdec_try_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_pix_format_mplane *pixmp = &f->fmt.pix_mp;
	u32 pix_fmt;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	memset(pixmp->reserved, 0, sizeof(pixmp->reserved));
	if (f->type == INPUT_MPLANE) {
		pix_fmt = v4l2_codec_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		if (!pix_fmt) {
			i_vpr_h(inst, "%s: unsupported codec, set default params\n", __func__);
			f->fmt.pix_mp.width = DEFAULT_WIDTH;
			f->fmt.pix_mp.height = DEFAULT_HEIGHT;
			f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
			pix_fmt = v4l2_codec_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		}
	} else if (f->type == OUTPUT_MPLANE) {
		if (inst->vb2q[INPUT_PORT].streaming) {
			f->fmt.pix_mp.height = inst->fmts[INPUT_PORT].fmt.pix_mp.height;
			f->fmt.pix_mp.width = inst->fmts[INPUT_PORT].fmt.pix_mp.width;
		}
		pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		if (!pix_fmt) {
			i_vpr_h(inst, "%s: unsupported format, set default params\n", __func__);
			f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
			f->fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat,
								 DEFAULT_WIDTH);
			f->fmt.pix_mp.height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat,
								 DEFAULT_HEIGHT);
			pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		}
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, f->type);
		return -EINVAL;
	}

	if (pixmp->field == V4L2_FIELD_ANY)
		pixmp->field = V4L2_FIELD_NONE;

	pixmp->num_planes = 1;
	return rc;
}

static bool msm_vidc_check_max_sessions_vp9d(struct msm_vidc_core *core)
{
	u32 vp9d_instance_count = 0;
	struct msm_vidc_inst *inst = NULL;

	core_lock(core, __func__);
	list_for_each_entry(inst, &core->instances, list) {
		if (is_decode_session(inst) &&
			inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat ==
				V4L2_PIX_FMT_VP9)
			vp9d_instance_count++;
	}
	core_unlock(core, __func__);

	if (vp9d_instance_count > MAX_VP9D_INST_COUNT)
		return true;
	return false;
}

int msm_vdec_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;
	u32 codec_align, pix_fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == INPUT_MPLANE) {
		if (inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat !=
			f->fmt.pix_mp.pixelformat) {
			rc = msm_vdec_codec_change(inst, f->fmt.pix_mp.pixelformat);
			if (rc)
				goto err_invalid_fmt;
		}

		if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_VP9) {
			if (msm_vidc_check_max_sessions_vp9d(inst->core)) {
				i_vpr_e(inst,
					"%s: vp9d sessions exceeded max limit %d\n",
					__func__, MAX_VP9D_INST_COUNT);
				rc = -ENOMEM;
				goto err_invalid_fmt;
			}
		}

		fmt = &inst->fmts[INPUT_PORT];
		fmt->type = INPUT_MPLANE;

		codec_align = inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat ==
			V4L2_PIX_FMT_HEVC ? 32 : 16;
		fmt->fmt.pix_mp.width = ALIGN(f->fmt.pix_mp.width, codec_align);
		fmt->fmt.pix_mp.height = ALIGN(f->fmt.pix_mp.height, codec_align);
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.min_count = call_session_op(core,
			min_count, inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.extra_count = call_session_op(core,
			extra_count, inst, MSM_VIDC_BUF_INPUT);
		if (inst->buffers.input.actual_count <
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count) {
			inst->buffers.input.actual_count =
				inst->buffers.input.min_count +
				inst->buffers.input.extra_count;
		}
		inst->buffers.input.size =
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage;

		/* update crop dimensions */
		inst->crop.left = inst->crop.top = 0;
		inst->crop.width = f->fmt.pix_mp.width;
		inst->crop.height = f->fmt.pix_mp.height;
		i_vpr_h(inst,
			"%s: type: INPUT, codec %s width %d height %d size %u min_count %d extra_count %d\n",
			__func__, v4l2_pixelfmt_name(f->fmt.pix_mp.pixelformat),
			f->fmt.pix_mp.width, f->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.input.min_count,
			inst->buffers.input.extra_count);
	} else if (f->type == INPUT_META_PLANE) {
		fmt = &inst->fmts[INPUT_META_PORT];
		fmt->type = INPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		if (is_input_meta_enabled(inst)) {
			fmt->fmt.meta.buffersize = call_session_op(core,
				buffer_size, inst, MSM_VIDC_BUF_INPUT_META);
			inst->buffers.input_meta.min_count =
					inst->buffers.input.min_count;
			inst->buffers.input_meta.extra_count =
					inst->buffers.input.extra_count;
			inst->buffers.input_meta.actual_count =
					inst->buffers.input.actual_count;
			inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
		} else {
			fmt->fmt.meta.buffersize = 0;
			inst->buffers.input_meta.min_count = 0;
			inst->buffers.input_meta.extra_count = 0;
			inst->buffers.input_meta.actual_count = 0;
			inst->buffers.input_meta.size = 0;
		}
		i_vpr_h(inst,
			"%s: type: INPUT_META, size %u min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.input_meta.min_count,
			inst->buffers.input_meta.extra_count);
	} else if (f->type == OUTPUT_MPLANE) {
		fmt = &inst->fmts[OUTPUT_PORT];
		fmt->type = OUTPUT_MPLANE;
		if (inst->vb2q[INPUT_PORT].streaming) {
			f->fmt.pix_mp.height = inst->fmts[INPUT_PORT].fmt.pix_mp.height;
			f->fmt.pix_mp.width = inst->fmts[INPUT_PORT].fmt.pix_mp.width;
		}
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(
			fmt->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width);
		fmt->fmt.pix_mp.height = VIDEO_Y_SCANLINES(
			fmt->fmt.pix_mp.pixelformat,
			f->fmt.pix_mp.height);
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline =
			VIDEO_Y_STRIDE_BYTES(
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat,
			f->fmt.pix_mp.width);
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT);

		if (!inst->vb2q[INPUT_PORT].streaming)
			inst->buffers.output.min_count = call_session_op(core,
				min_count, inst, MSM_VIDC_BUF_OUTPUT);
		inst->buffers.output.extra_count = call_session_op(core,
			extra_count, inst, MSM_VIDC_BUF_OUTPUT);
		if (inst->buffers.output.actual_count <
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count) {
			inst->buffers.output.actual_count =
				inst->buffers.output.min_count +
				inst->buffers.output.extra_count;
		}
		inst->buffers.output.size =
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage;
		pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		msm_vidc_update_cap_value(inst, PIX_FMTS, pix_fmt, __func__);

		/* update crop while input port is not streaming */
		if (!inst->vb2q[INPUT_PORT].streaming) {
			inst->crop.top = 0;
			inst->crop.left = 0;
			inst->crop.width = f->fmt.pix_mp.width;
			inst->crop.height = f->fmt.pix_mp.height;
		}
		i_vpr_h(inst,
			"%s: type: OUTPUT, format %s width %d height %d size %u min_count %d extra_count %d\n",
			__func__, v4l2_pixelfmt_name(fmt->fmt.pix_mp.pixelformat),
			fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.output.min_count,
			inst->buffers.output.extra_count);
	} else if (f->type == OUTPUT_META_PLANE) {
		fmt = &inst->fmts[OUTPUT_META_PORT];
		fmt->type = OUTPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		if (is_output_meta_enabled(inst)) {
			fmt->fmt.meta.buffersize = call_session_op(core,
				buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
			inst->buffers.output_meta.min_count =
					inst->buffers.output.min_count;
			inst->buffers.output_meta.extra_count =
					inst->buffers.output.extra_count;
			inst->buffers.output_meta.actual_count =
					inst->buffers.output.actual_count;
			inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
		} else {
			fmt->fmt.meta.buffersize = 0;
			inst->buffers.output_meta.min_count = 0;
			inst->buffers.output_meta.extra_count = 0;
			inst->buffers.output_meta.actual_count = 0;
			inst->buffers.output_meta.size = 0;
		}
		i_vpr_h(inst,
			"%s: type: OUTPUT_META, size %u min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.output_meta.min_count,
			inst->buffers.output_meta.extra_count);
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, f->type);
		goto err_invalid_fmt;
	}
	memcpy(f, fmt, sizeof(struct v4l2_format));

err_invalid_fmt:
	return rc;
}

int msm_vdec_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	int port;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	port = v4l2_type_to_driver_port(inst, f->type, __func__);
	if (port < 0)
		return -EINVAL;

	memcpy(f, &inst->fmts[port], sizeof(struct v4l2_format));

	return rc;
}

int msm_vdec_s_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s)
{
	if (!inst || !s) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_e(inst, "%s: unsupported\n", __func__);
	return -EINVAL;
}

int msm_vdec_g_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s)
{
	if (!inst || !s) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (s->type != OUTPUT_MPLANE && s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, s->type);
		return -EINVAL;
	}

	switch (s->target) {
	case V4L2_SEL_TGT_CROP_BOUNDS:
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP:
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
	case V4L2_SEL_TGT_COMPOSE_PADDED:
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE:
		s->r.left = inst->crop.left;
		s->r.top = inst->crop.top;
		s->r.width = inst->crop.width;
		s->r.height = inst->crop.height;
		break;
	default:
		i_vpr_e(inst, "%s: invalid target %d\n",
			__func__, s->target);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s: target %d, r [%d, %d, %d, %d]\n",
		__func__, s->target, s->r.top, s->r.left,
		s->r.width, s->r.height);
	return 0;
}

int msm_vdec_s_param(struct msm_vidc_inst *inst,
		struct v4l2_streamparm *s_parm)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability = NULL;
	struct v4l2_fract *timeperframe = NULL;
	u32 q16_rate, max_rate, default_rate;
	u64 us_per_frame = 0, input_rate = 0;
	bool is_frame_rate = false;

	if (!inst || !s_parm) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (s_parm->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		timeperframe = &s_parm->parm.output.timeperframe;
		max_rate = capability->cap[FRAME_RATE].max >> 16;
		default_rate = capability->cap[FRAME_RATE].value >> 16;
		is_frame_rate = true;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		max_rate = capability->cap[OPERATING_RATE].max >> 16;
		default_rate = capability->cap[OPERATING_RATE].value >> 16;
	}

	if (!timeperframe->denominator || !timeperframe->numerator) {
		i_vpr_e(inst, "%s: type %s, invalid rate\n", __func__,
			v4l2_type_name(s_parm->type));
		input_rate = default_rate;
		goto set_default;
	}

	us_per_frame = timeperframe->numerator * (u64)USEC_PER_SEC;
	do_div(us_per_frame, timeperframe->denominator);

	if (!us_per_frame) {
		i_vpr_e(inst, "%s: us_per_frame is zero\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	input_rate = (u64)USEC_PER_SEC;
	do_div(input_rate, us_per_frame);

set_default:
	i_vpr_h(inst, "%s: type %s, %s value %llu\n",
		__func__, v4l2_type_name(s_parm->type),
		is_frame_rate ? "frame rate" : "operating rate", input_rate);

	q16_rate = (u32)input_rate << 16;
	msm_vidc_update_cap_value(inst, is_frame_rate ? FRAME_RATE : OPERATING_RATE,
		q16_rate, __func__);
	if (is_realtime_session(inst) &&
		((s_parm->type == INPUT_MPLANE && inst->vb2q[INPUT_PORT].streaming) ||
		(s_parm->type == OUTPUT_MPLANE && inst->vb2q[OUTPUT_PORT].streaming))) {
		rc = msm_vidc_check_core_mbps(inst);
		if (rc) {
			i_vpr_e(inst, "%s: unsupported load\n", __func__);
			goto reset_rate;
		}
		rc = input_rate > max_rate;
		if (rc) {
			i_vpr_e(inst, "%s: unsupported rate %llu, max %u\n", __func__,
				input_rate, max_rate);
			rc = -ENOMEM;
			goto reset_rate;
		}
	}
	inst->priority_level = MSM_VIDC_PRIORITY_HIGH;

	if (is_frame_rate)
		capability->cap[FRAME_RATE].flags |= CAP_FLAG_CLIENT_SET;
	else
		capability->cap[OPERATING_RATE].flags |= CAP_FLAG_CLIENT_SET;

	return 0;

reset_rate:
	if (rc) {
		i_vpr_e(inst, "%s: setting rate %llu failed, reset to %u\n", __func__,
			input_rate, default_rate);
		msm_vidc_update_cap_value(inst, is_frame_rate ? FRAME_RATE : OPERATING_RATE,
			default_rate << 16, __func__);
	}
exit:
	return rc;
}

int msm_vdec_g_param(struct msm_vidc_inst *inst,
		struct v4l2_streamparm *s_parm)
{
	struct msm_vidc_inst_capability *capability = NULL;
	struct v4l2_fract *timeperframe = NULL;

	if (!inst || !s_parm) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (s_parm->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		timeperframe = &s_parm->parm.output.timeperframe;
		timeperframe->numerator = 1;
		timeperframe->denominator =
			capability->cap[FRAME_RATE].value >> 16;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		timeperframe->numerator = 1;
		timeperframe->denominator =
			capability->cap[OPERATING_RATE].value >> 16;
	}

	i_vpr_h(inst, "%s: type %u, num %u denom %u\n",
		__func__, s_parm->type, timeperframe->numerator,
		timeperframe->denominator);
	return 0;
}

static int msm_vdec_check_colorformat_supported(struct msm_vidc_inst* inst,
		enum msm_vidc_colorformat_type colorformat)
{
	bool supported = true;

	/* do not reject coloformats before streamon */
	if (!inst->vb2q[INPUT_PORT].streaming)
		return true;

	/*
	 * bit_depth 8 bit supports 8 bit colorformats only
	 * bit_depth 10 bit supports 10 bit colorformats only
	 * interlace supports ubwc colorformats only
	 */
	if (inst->capabilities->cap[BIT_DEPTH].value == BIT_DEPTH_8 &&
		!is_8bit_colorformat(colorformat))
		supported = false;
	if (inst->capabilities->cap[BIT_DEPTH].value == BIT_DEPTH_10 &&
		!is_10bit_colorformat(colorformat))
		supported = false;
	if (inst->capabilities->cap[CODED_FRAMES].value ==
		CODED_FRAMES_INTERLACE &&
		is_ubwc_supported_platform(inst) &&
		!is_ubwc_colorformat(colorformat))
		supported = false;

	return supported;
}

int msm_vdec_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 array[32] = {0};
	u32 i = 0;

	if (!inst || !inst->core || !inst->capabilities || !f ||
		f->index >= ARRAY_SIZE(array)) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == INPUT_MPLANE) {
		u32 codecs = core->capabilities[DEC_CODECS].value;
		u32 idx = 0;

		for (i = 0; i <= 31; i++) {
			if (codecs & BIT(i)) {
				if (idx >= ARRAY_SIZE(array))
					break;
				array[idx] = codecs & BIT(i);
				idx++;
			}
		}
		if (!array[f->index])
			return -EINVAL;
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == OUTPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;
		u32 idx = 0;

		for (i = 0; i <= 31; i++) {
			if (formats & BIT(i)) {
				if (idx >= ARRAY_SIZE(array))
					break;
				if (msm_vdec_check_colorformat_supported(inst,
						formats & BIT(i))) {
					array[idx] = formats & BIT(i);
					idx++;
				}
			}
		}
		if (!array[f->index])
			return -EINVAL;
		f->pixelformat = v4l2_colorformat_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		strlcpy(f->description, "colorformat", sizeof(f->description));
	} else if (f->type == INPUT_META_PLANE || f->type == OUTPUT_META_PLANE) {
		if (!f->index) {
			f->pixelformat = V4L2_META_FMT_VIDC;
			strlcpy(f->description, "metadata", sizeof(f->description));
		} else {
			return -EINVAL;
		}
	}
	memset(f->reserved, 0, sizeof(f->reserved));

	i_vpr_h(inst, "%s: index %d, %s: %s, flags %#x\n",
		__func__, f->index, f->description, v4l2_pixelfmt_name(f->pixelformat), f->flags);
	return rc;
}

int msm_vdec_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	INIT_DELAYED_WORK(&inst->decode_batch.work, msm_vidc_batch_handler);
	if (core->capabilities[DECODE_BATCH].value) {
		inst->decode_batch.enable = true;
		inst->decode_batch.size = MAX_DEC_BATCH_SIZE;
	}
	if (core->capabilities[DCVS].value)
		inst->power.dcvs_mode = true;

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
	f->fmt.pix_mp.field = V4L2_FIELD_NONE;
	inst->buffers.input.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	inst->buffers.input.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	inst->crop.left = inst->crop.top = 0;
	inst->crop.width = f->fmt.pix_mp.width;
	inst->crop.height = f->fmt.pix_mp.height;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = 0;
	inst->buffers.input_meta.min_count = 0;
	inst->buffers.input_meta.extra_count = 0;
	inst->buffers.input_meta.actual_count = 0;
	inst->buffers.input_meta.size = 0;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
	f->fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat,
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat,
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline =
		VIDEO_Y_STRIDE_BYTES(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat,
		DEFAULT_WIDTH);
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	f->fmt.pix_mp.field = V4L2_FIELD_NONE;
	f->fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	f->fmt.pix_mp.xfer_func = V4L2_XFER_FUNC_DEFAULT;
	f->fmt.pix_mp.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	f->fmt.pix_mp.quantization = V4L2_QUANTIZATION_DEFAULT;
	inst->buffers.output.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;
	inst->max_map_output_count = MAX_MAP_OUTPUT_COUNT;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = 0;
	inst->buffers.output_meta.min_count = 0;
	inst->buffers.output_meta.extra_count = 0;
	inst->buffers.output_meta.actual_count = 0;
	inst->buffers.output_meta.size = 0;

	inst->priority_level = MSM_VIDC_PRIORITY_LOW;

	rc = msm_vdec_codec_change(inst,
			inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat);
	if (rc)
		return rc;

	return rc;
}

int msm_vdec_inst_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	/* cancel pending batch work */
	cancel_batch_work(inst);
	rc = msm_vidc_ctrl_handler_deinit(inst);
	if (rc)
		return rc;

	return rc;
}

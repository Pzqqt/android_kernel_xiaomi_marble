// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/v4l2_vidc_extensions.h>
#include <media/msm_media_info.h>

#include "msm_vdec.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_control.h"
#include "venus_hfi.h"
#include "hfi_packet.h"

u32 msm_vdec_subscribe_for_port_settings_change[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
	HFI_PROP_CABAC_SESSION,
	HFI_PROP_CODED_FRAMES,
	HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
	HFI_PROP_PIC_ORDER_CNT_TYPE,
	HFI_PROP_SIGNAL_COLOR_INFO,
	HFI_PROP_PROFILE,
	HFI_PROP_LEVEL,
	HFI_PROP_TIER,
};

u32 msm_vdec_subscribe_for_properties[] = {
	HFI_PROP_NO_OUTPUT,
};

u32 msm_vdec_subscribe_for_metadata[] = {
	HFI_PROP_BUFFER_TAG,
};

u32 msm_vdec_deliver_as_metadata[] = {
	HFI_PROP_BUFFER_TAG,
};

static int msm_vdec_codec_change(struct msm_vidc_inst *inst, u32 v4l2_codec)
{
	int rc = 0;

	if (inst->codec && inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat == v4l2_codec)
		return 0;

	s_vpr_h(inst->sid, "%s: codec changed from %#x to %#x\n",
		__func__, inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat, v4l2_codec);

	inst->codec = v4l2_codec_to_driver(v4l2_codec, __func__);
	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_deinit(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_init(inst);
	if(rc)
		goto exit;

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
	s_vpr_h(inst->sid, "%s: width: %d height: %d\n", __func__,
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
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

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
		V4L2_PIX_FMT_VIDC_P010)
		return 0;

	stride_y = inst->fmts[OUTPUT_PORT].fmt.pix_mp.width;
	scanline_y = inst->fmts[OUTPUT_PORT].fmt.pix_mp.height;
	stride_uv = stride_y;
	scanline_uv = scanline_y / 2;

	payload[0] = stride_y << 16 | scanline_y;
	payload[1] = stride_uv << 16 | scanline_uv;
	s_vpr_h(inst->sid, "%s: stride_y: %d scanline_y: %d "
		"stride_uv: %d, scanline_uv: %d", __func__,
		stride_y, scanline_y, stride_uv, scanline_uv);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LINEAR_STRIDE_SCANLINE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, OUTPUT_PORT),
			HFI_PAYLOAD_U64,
			&payload,
			sizeof(u64));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_crop_offsets(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 left_offset, top_offset, right_offset, bottom_offset;
	u64 payload;

	if (inst->fmts[INPUT_PORT].fmt.pix_mp.width <
		inst->crop.width)
		return -EINVAL;

	if (inst->fmts[INPUT_PORT].fmt.pix_mp.height <
		inst->crop.height)
		return -EINVAL;

	left_offset = inst->crop.left;
	top_offset = inst->crop.top;
	right_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.width -
		inst->crop.width);
	bottom_offset = (inst->fmts[INPUT_PORT].fmt.pix_mp.height -
		inst->crop.height);

	payload = (u64)right_offset << 48 | (u64)bottom_offset << 32 |
		(u64)left_offset << 16 | top_offset;
	s_vpr_h(inst->sid, "%s: left_offset: %d top_offset: %d "
		"right_offset: %d bottom_offset: %d", __func__,
		left_offset, top_offset, right_offset, bottom_offset);
	inst->subcr_params[port].crop_offsets = payload;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_OFFSETS,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&payload,
			sizeof(u64));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_bit_depth(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 colorformat;
	u32 bitdepth = 8 << 16 | 8;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	colorformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	if (colorformat == V4L2_PIX_FMT_VIDC_P010 ||
	    colorformat == V4L2_PIX_FMT_VIDC_TP10C)
		bitdepth = 10 << 16 | 10;

	inst->subcr_params[port].bit_depth = bitdepth;
	s_vpr_h(inst->sid, "%s: bit depth: %d", __func__, bitdepth);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&bitdepth,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_cabac(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 cabac = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = msm_vidc_v4l2_menu_to_hfi(inst, ENTROPY_MODE, &cabac);
	if (rc)
		return rc;

	inst->subcr_params[port].cabac = cabac;
	s_vpr_h(inst->sid, "%s: entropy mode: %d", __func__, cabac);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CABAC_SESSION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&cabac,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_coded_frames(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 coded_frames;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	/* (mb_adaptive_frame_field_flag << 1) | frame_mbs_only_flag */
	coded_frames = 0;
	inst->subcr_params[port].coded_frames = coded_frames;
	s_vpr_h(inst->sid, "%s: coded frames: %d", __func__, coded_frames);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CODED_FRAMES,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&coded_frames,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_min_output_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 min_output;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	min_output = inst->buffers.output.min_count;
	inst->subcr_params[port].fw_min_count = min_output;
	s_vpr_h(inst->sid, "%s: firmware min output count: %d",
		__func__, min_output);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&min_output,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_picture_order_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 poc = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	inst->subcr_params[port].pic_order_cnt = poc;
	s_vpr_h(inst->sid, "%s: picture order count: %d", __func__, poc);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIC_ORDER_CNT_TYPE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&poc,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_colorspace(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 colorspace, xfer_func, ycbcr_enc, color_info;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	colorspace = inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace;
	xfer_func = inst->fmts[OUTPUT_PORT].fmt.pix_mp.xfer_func;
	ycbcr_enc = inst->fmts[OUTPUT_PORT].fmt.pix_mp.ycbcr_enc;

	color_info = ((ycbcr_enc << 16) & 0xFF0000) |
		((xfer_func << 8) & 0xFF00) | (colorspace & 0xFF);
	inst->subcr_params[port].color_info = color_info;
	s_vpr_h(inst->sid, "%s: color info: %d", __func__, color_info);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_SIGNAL_COLOR_INFO,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&color_info,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_profile(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 profile;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	profile = inst->capabilities->cap[PROFILE].value;
	inst->subcr_params[port].profile = profile;
	s_vpr_h(inst->sid, "%s: profile: %d", __func__, profile);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PROFILE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&profile,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_level(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 level;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	level = inst->capabilities->cap[LEVEL].value;
	inst->subcr_params[port].level = level;
	s_vpr_h(inst->sid, "%s: level: %d", __func__, level);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LEVEL,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&level,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_tier(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 tier;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	tier = inst->capabilities->cap[HEVC_TIER].value;
	inst->subcr_params[port].tier = tier;
	s_vpr_h(inst->sid, "%s: tier: %d", __func__, tier);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_TIER,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&tier,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

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
	s_vpr_h(inst->sid, "%s: hfi colorformat: %d",
		__func__, hfi_colorformat);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_COLOR_FORMAT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, OUTPUT_PORT),
			HFI_PAYLOAD_U32,
			&hfi_colorformat,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_stage(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 stage = 0;
	struct msm_vidc_core *core = inst->core;

	rc = call_session_op(core, decide_work_mode, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_mode failed %d\n",
			__func__);
		return -EINVAL;
	}

	stage = inst->stage;
	s_vpr_h(inst->sid, "%s: stage: %d", __func__, stage);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_STAGE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&stage,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_pipe(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 pipe;
	struct msm_vidc_core *core = inst->core;

	rc = call_session_op(core, decide_work_route, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_route failed\n",
			__func__);
		return -EINVAL;
	}

	pipe = inst->pipe;
	s_vpr_h(inst->sid, "%s: pipe: %d", __func__, pipe);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIPE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&inst->pipe,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_output_order(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 output_order;

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	output_order = inst->capabilities->cap[DISPLAY_DELAY_ENABLE].value;
	s_vpr_h(inst->sid, "%s: output order: %d", __func__, output_order);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_DECODE_ORDER_OUTPUT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&output_order,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_secure_mode(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 secure_mode;

	secure_mode = inst->capabilities->cap[SECURE_MODE].value;
	s_vpr_h(inst->sid, "%s: secure mode: %d", __func__, secure_mode);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_SECURE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&secure_mode,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_thumbnail_mode(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 thumbnail_mode = 0;

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: thumbnail mode: %d", __func__, thumbnail_mode);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_THUMBNAIL_MODE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&thumbnail_mode,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_realtime(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 realtime = 1;  //todo

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: priority: %d", __func__, realtime);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_REALTIME,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&realtime,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_conceal_color_8bit(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 conceal_color_8bit;

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	conceal_color_8bit = inst->capabilities->cap[CONCEAL_COLOR_8BIT].value;
	s_vpr_h(inst->sid, "%s: conceal color 8bit: %#x",
		__func__, conceal_color_8bit);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CONCEAL_COLOR_8BIT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&conceal_color_8bit,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_conceal_color_10bit(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 conceal_color_10bit;

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	conceal_color_10bit = inst->capabilities->cap[CONCEAL_COLOR_8BIT].value;
	s_vpr_h(inst->sid, "%s: conceal color 10bit: %#x",
		__func__, conceal_color_10bit);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CONCEAL_COLOR_10BIT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&conceal_color_10bit,
			sizeof(u32));
	if (rc)
		s_vpr_e(inst->sid, "%s: set property failed\n", __func__);

	return rc;
}

static int msm_vdec_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vdec_set_output_order(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_secure_mode(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_thumbnail_mode(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_realtime(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_conceal_color_8bit(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_conceal_color_10bit(inst, INPUT_PORT);
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

	rc = msm_vdec_set_stage(inst);
	if (rc)
		return rc;

	rc = msm_vdec_set_pipe(inst);
	if (rc)
		return rc;

	rc = msm_vdec_set_linear_stride_scanline(inst);
	if (rc)
		return rc;

	return rc;
}

static int msm_vdec_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	/*
	 * TODO: Remove the hack of sending bigger buffer sizes
	 * once internal buffer calculations are finalised
	 */
	inst->buffers.bin.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_BIN) + 100000000;
	inst->buffers.comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_COMV) + 100000000;
	inst->buffers.non_comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_NON_COMV) + 100000000;
	inst->buffers.line.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_LINE) + 100000000;
	inst->buffers.persist.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_PERSIST) + 100000000;

	inst->buffers.bin.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_BIN);
	inst->buffers.comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_COMV);
	inst->buffers.non_comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_NON_COMV);
	inst->buffers.line.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_LINE);
	inst->buffers.persist.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_PERSIST);

	s_vpr_h(inst->sid, "internal buffer: min     size\n");
	s_vpr_h(inst->sid, "bin  buffer: %d      %d\n",
		inst->buffers.bin.min_count,
		inst->buffers.bin.size);
	s_vpr_h(inst->sid, "comv  buffer: %d      %d\n",
		inst->buffers.comv.min_count,
		inst->buffers.comv.size);
	s_vpr_h(inst->sid, "non_comv  buffer: %d      %d\n",
		inst->buffers.non_comv.min_count,
		inst->buffers.non_comv.size);
	s_vpr_h(inst->sid, "line buffer: %d      %d\n",
		inst->buffers.line.min_count,
		inst->buffers.line.size);
	s_vpr_h(inst->sid, "persist buffer: %d      %d\n",
		inst->buffers.persist.min_count,
		inst->buffers.persist.size);

	return rc;
}

static int msm_vdec_create_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}

static int msm_vdec_queue_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}


static int msm_vdec_release_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_subscribe_port_settings_change(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change);
	     i++)
		payload[i + 1] = msm_vdec_subscribe_for_port_settings_change[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change) + 1) *
			sizeof(u32));

	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change);
		i++) {
		switch (msm_vdec_subscribe_for_port_settings_change[i]) {
		case HFI_PROP_BITSTREAM_RESOLUTION:
			rc = msm_vdec_set_bitstream_resolution(inst, port);
			break;
		case HFI_PROP_CROP_OFFSETS:
			rc = msm_vdec_set_crop_offsets(inst, port);
			break;
		case HFI_PROP_LUMA_CHROMA_BIT_DEPTH:
			rc = msm_vdec_set_bit_depth(inst, port);
			break;
		case HFI_PROP_CABAC_SESSION:
			rc = msm_vdec_set_cabac(inst, port);
			break;
		case HFI_PROP_CODED_FRAMES:
			rc = msm_vdec_set_coded_frames(inst, port);
			break;
		case HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT:
			rc = msm_vdec_set_min_output_count(inst, port);
			break;
		case HFI_PROP_PIC_ORDER_CNT_TYPE:
			rc = msm_vdec_set_picture_order_count(inst, port);
			break;
		case HFI_PROP_SIGNAL_COLOR_INFO:
			rc = msm_vdec_set_colorspace(inst, port);
			break;
		case HFI_PROP_PROFILE:
			rc = msm_vdec_set_profile(inst, port);
			break;
		case HFI_PROP_LEVEL:
			rc = msm_vdec_set_level(inst, port);
			break;
		case HFI_PROP_TIER:
			rc = msm_vdec_set_tier(inst, port);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_vdec_subscribe_for_port_settings_change[i]);
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int msm_vdec_subscribe_property(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PROPERTY;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_properties); i++)
		payload[i + 1] = msm_vdec_subscribe_for_properties[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_properties) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_subscribe_metadata(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_metadata); i++)
		payload[i + 1] = msm_vdec_subscribe_for_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_metadata) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_set_delivery_mode_metadata(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_deliver_as_metadata); i++)
		payload[i + 1] = msm_vdec_deliver_as_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_deliver_as_metadata) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_session_resume(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	rc = venus_hfi_session_command(inst,
			HFI_CMD_RESUME,
			port,
			HFI_PAYLOAD_NONE,
			NULL,
			0);

	return rc;
}

static msm_vdec_update_input_properties(struct msm_vidc_inst *inst)
{
	struct msm_vidc_subscription_params subsc_params;
	u32 width, height;

	subsc_params = inst->subcr_params[INPUT_PORT];
	width = (subsc_params.bitstream_resolution &
		HFI_BITMASK_BITSTREAM_WIDTH) >> 16;
	height = subsc_params.bitstream_resolution &
		HFI_BITMASK_BITSTREAM_HEIGHT;

	inst->fmts[INPUT_PORT].fmt.pix_mp.width = width;
	inst->fmts[INPUT_PORT].fmt.pix_mp.height = height;

	inst->fmts[OUTPUT_PORT].fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, __func__),
		width);
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat,	__func__),
		height);
	//inst->fmts[OUTPUT_PORT].fmt.pix_mp.bytesperline =
		//inst->fmts[OUTPUT_PORT].fmt.pix_mp.width;

	inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace =
		(subsc_params.color_info & 0xFF0000) >> 16;
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.xfer_func =
		(subsc_params.color_info & 0xFF00) >> 8;
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.ycbcr_enc =
		subsc_params.color_info & 0xFF;

	inst->buffers.output.min_count = subsc_params.fw_min_count;

	inst->crop.top = subsc_params.crop_offsets & 0xFFFF;
	inst->crop.left = (subsc_params.crop_offsets >> 16) & 0xFFFF;
	inst->crop.height = inst->fmts[INPUT_PORT].fmt.pix_mp.height -
		((subsc_params.crop_offsets >> 32) & 0xFFFF);
	inst->crop.width = inst->fmts[INPUT_PORT].fmt.pix_mp.width -
		((subsc_params.crop_offsets >> 48) & 0xFFFF);

	inst->capabilities->cap[PROFILE].value = subsc_params.profile;
	inst->capabilities->cap[LEVEL].value = subsc_params.level;
	inst->capabilities->cap[HEVC_TIER].value = subsc_params.tier;
	inst->capabilities->cap[ENTROPY_MODE].value = subsc_params.cabac;
	inst->capabilities->cap[POC].value = subsc_params.pic_order_cnt;

	return 0;
}

int msm_vdec_input_port_settings_change(struct msm_vidc_inst *inst)
{
	u32 rc = 0;
	struct v4l2_event event = {0};

	if (!inst->vb2q[INPUT_PORT].streaming) {
		s_vpr_e(inst->sid, "%s: input port not streaming\n",
			__func__);
		return 0;
	}

	rc = msm_vdec_update_input_properties(inst);
	if (rc)
		return rc;

	event.type = V4L2_EVENT_SOURCE_CHANGE;
	event.u.src_change.changes = V4L2_EVENT_SRC_CH_RESOLUTION;
	v4l2_event_queue_fh(&inst->event_handler, &event);

	rc = msm_vdec_release_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_get_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_create_input_internal_buffers(inst);
	if (rc)
		return rc;

	rc = msm_vdec_queue_input_internal_buffers(inst);
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

int msm_vdec_stop_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_stop(inst, INPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_start_input(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	s_vpr_h(inst->sid, "%s()\n", __func__);

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;
	//rc = msm_vidc_check_scaling_supported(inst);
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

	//msm_vidc_update_dcvs(inst);
	//msm_vidc_update_batching(inst);
	//msm_vidc_scale_power(inst);

	rc = msm_vdec_create_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vdec_queue_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vdec_subscribe_property(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_delivery_mode_metadata(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = venus_hfi_start(inst, INPUT_PORT);
	if (rc)
		goto error;

	s_vpr_h(inst->sid, "%s: done\n", __func__);
	return 0;

error:
	s_vpr_e(inst->sid, "%s: failed\n", __func__);
	msm_vdec_stop_input(inst);
	return rc;
}

int msm_vdec_stop_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_stop(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_start_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vdec_set_output_properties(inst);
	if (rc)
		goto error;

	rc = msm_vdec_subscribe_metadata(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = venus_hfi_start(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	d_vpr_h("%s: done\n", __func__);
	return 0;

error:
	msm_vdec_stop_output(inst);
	return rc;
}

static int msm_vdec_qbuf_batch(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2)
{
	int rc = 0;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	return rc;
}

int msm_vdec_qbuf(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;

	if (inst->decode_batch.enable)
		rc = msm_vdec_qbuf_batch(inst, vb2);
	else
		rc = msm_vidc_queue_buffer(inst, vb2);

	return rc;
}

int msm_vdec_process_cmd(struct msm_vidc_inst *inst, u32 cmd)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (cmd == V4L2_DEC_CMD_STOP) {
		rc = venus_hfi_session_command(inst,
				HFI_CMD_DRAIN,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
	} else {
		d_vpr_e("%s: unknown cmd %d\n", __func__, cmd);
		return -EINVAL;
	}

	return 0;
}

int msm_vdec_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (inst->state == MSM_VIDC_START) {
		d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
		return -EINVAL;
	}

	if (f->type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		if (inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat !=
			f->fmt.pix_mp.pixelformat) {
			s_vpr_e(inst->sid,
				"%s: codec changed from %#x to %#x\n", __func__,
				inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat,
				f->fmt.pix_mp.pixelformat);
			rc = msm_vdec_codec_change(inst, f->fmt.pix_mp.pixelformat);
			if (rc)
				goto err_invalid_fmt;
		}
		fmt = &inst->fmts[INPUT_PORT];
		fmt->type = INPUT_MPLANE;
		fmt->fmt.pix_mp.width = ALIGN(f->fmt.pix_mp.width, 16);
		fmt->fmt.pix_mp.height = ALIGN(f->fmt.pix_mp.height, 16);
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
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

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		//update_log_ctxt(inst->sid, inst->session_type,
		//	mplane->pixelformat);
		s_vpr_h(inst->sid,
			"%s: input: codec %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width,
			f->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.input.min_count,
			inst->buffers.input.extra_count);

		//msm_vidc_update_dcvs(inst);
		//msm_vidc_update_batching(inst);

	} else if (f->type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[INPUT_META_PORT];
		fmt->type = INPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		fmt->fmt.meta.buffersize = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_INPUT_META);
		inst->buffers.input_meta.min_count =
				inst->buffers.input.min_count;
		inst->buffers.input_meta.extra_count =
				inst->buffers.input.extra_count;
		inst->buffers.input_meta.actual_count =
				inst->buffers.input.actual_count;
		inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
		s_vpr_h(inst->sid,
			"%s: input meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.input_meta.min_count,
			inst->buffers.input_meta.extra_count);
	} else if (f->type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_PORT];
		fmt->type = OUTPUT_MPLANE;
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.width = VENUS_Y_STRIDE(
			v4l2_colorformat_to_media(
			fmt->fmt.pix_mp.pixelformat, __func__),
			f->fmt.pix_mp.width);
		fmt->fmt.pix_mp.height = VENUS_Y_SCANLINES(
			v4l2_colorformat_to_media(
			fmt->fmt.pix_mp.pixelformat, __func__),
			f->fmt.pix_mp.height);
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline =
			fmt->fmt.pix_mp.width;
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
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

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		s_vpr_h(inst->sid,
			"%s: output: format %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.width,
			fmt->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.output.min_count,
			inst->buffers.output.extra_count);
	} else if (f->type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_META_PORT];
		fmt->type = OUTPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		fmt->fmt.meta.buffersize = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_OUTPUT_META);
		inst->buffers.output_meta.min_count =
				inst->buffers.output.min_count;
		inst->buffers.output_meta.extra_count =
				inst->buffers.output.extra_count;
		inst->buffers.output_meta.actual_count =
				inst->buffers.output.actual_count;
		inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
		s_vpr_h(inst->sid,
			"%s: output meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.output_meta.min_count,
			inst->buffers.output_meta.extra_count);
	} else {
		s_vpr_e(inst->sid, "%s: invalid type %d\n", __func__, f->type);
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

	d_vpr_h("%s()\n", __func__);
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

int msm_vdec_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 array[32] = {0};
	u32 i = 0, idx = 0;

	if (!inst || !inst->core || !inst->capabilities || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == INPUT_MPLANE) {
		u32 codecs = core->capabilities[DEC_CODECS].value;

		while (codecs) {
			if (idx > 31)
				break;
			if (codecs & BIT(i)) {
				array[idx] = codecs & BIT(i);
				idx++;
			}
			i++;
			codecs >>= 1;
		}
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == OUTPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;

		while (formats) {
			if (idx > 31)
				break;
			if (formats & BIT(i)) {
				array[idx] = formats & BIT(i);
				idx++;
			}
			i++;
			formats >>= 1;
		}
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

	s_vpr_h(inst->sid, "%s: index %d, %s : %#x, flags %#x\n",
		__func__, f->index, f->description, f->pixelformat, f->flags);
	return rc;
}

int msm_vdec_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *f;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	INIT_DELAYED_WORK(&inst->decode_batch.work, msm_vidc_batch_handler);

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
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
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_INPUT_META);
	inst->buffers.input_meta.min_count = inst->buffers.input.min_count;
	inst->buffers.input_meta.extra_count = inst->buffers.input.extra_count;
	inst->buffers.input_meta.actual_count = inst->buffers.input.actual_count;
	inst->buffers.input_meta.size = f->fmt.meta.buffersize;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
	f->fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = f->fmt.pix_mp.width;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_OUTPUT_META);
	inst->buffers.output_meta.min_count = inst->buffers.output.min_count;
	inst->buffers.output_meta.extra_count = inst->buffers.output.extra_count;
	inst->buffers.output_meta.actual_count = inst->buffers.output.actual_count;
	inst->buffers.output_meta.size = f->fmt.meta.buffersize;

	inst->prop.frame_rate = DEFAULT_FPS << 16;
	inst->prop.operating_rate = DEFAULT_FPS << 16;
	inst->stage = MSM_VIDC_STAGE_2;
	inst->pipe = MSM_VIDC_PIPE_4;

	rc = msm_vdec_codec_change(inst,
			inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat);

	return rc;
}


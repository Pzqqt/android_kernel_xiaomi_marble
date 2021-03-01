// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#include "hfi_property.h"
#include "hfi_buffer_iris2.h"
#include "msm_vidc_buffer_iris2.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "msm_media_info.h"

static u32 msm_vidc_decoder_bin_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format *f;
	bool is_interlaced;
	u32 vpp_delay;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;

	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	if (inst->capabilities->cap[CODED_FRAMES].value ==
			CODED_FRAMES_PROGRESSIVE)
		is_interlaced = false;
	else
		is_interlaced = true;
	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_BIN_H264D(size, width, height,
			is_interlaced, vpp_delay, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_BIN_H265D(size, width, height,
			0, vpp_delay, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_BIN_VP9D(size, width, height,
			0, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, out_min_count, vpp_delay;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;
	out_min_count = inst->buffers.output.min_count;
	out_min_count = max(vpp_delay + 1, out_min_count);

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_COMV_H264D(size, width, height, out_min_count);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_COMV_H265D(size, width, height, out_min_count);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_non_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct msm_vidc_core* core;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_NON_COMV_H264D(size, width, height, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_NON_COMV_H265D(size, width, height, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_line_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, out_min_count, num_vpp_pipes, vpp_delay;
	struct v4l2_format *f;
	bool is_opb;
	u32 color_fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	color_fmt = v4l2_colorformat_to_driver(
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, __func__);
	if (is_linear_colorformat(color_fmt))
		is_opb = true;
	else
		is_opb = false;
	/*
	 * assume worst case, since color format is unknown at this
	 * time
	 */
	is_opb = true;

	if (inst->decode_vpp_delay.enable)
		vpp_delay = inst->decode_vpp_delay.size;
	else
		vpp_delay = DEFAULT_BSE_VPP_DELAY;

	f = &inst->fmts[INPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	out_min_count = inst->buffers.output.min_count;
	out_min_count = max(vpp_delay + 1, out_min_count);
	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_LINE_H264D(size, width, height, is_opb,
			num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_LINE_H265D(size, width, height, is_opb,
			num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_LINE_VP9D(size, width, height, out_min_count,
			is_opb, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_persist_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_PERSIST_H264D(size);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_PERSIST_H265D(size);
	else if (inst->codec == MSM_VIDC_VP9)
		HFI_BUFFER_PERSIST_VP9D(size);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_decoder_dpb_size_iris2(struct msm_vidc_inst *inst)
{

	u32 size = 0;
	u32 color_fmt, v4l2_fmt;
	u32 width, height;
	struct v4l2_format *f;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	color_fmt = inst->capabilities->cap[PIX_FMTS].value;
	if (!is_linear_colorformat(color_fmt))
		return size;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (color_fmt == MSM_VIDC_FMT_NV12) {
		v4l2_fmt = V4L2_PIX_FMT_VIDC_NV12C;
		HFI_NV12_UBWC_IL_CALC_BUF_SIZE_V2(size, width, height,
			VIDEO_Y_STRIDE_BYTES(v4l2_fmt, width), VIDEO_Y_SCANLINES(v4l2_fmt, height),
			VIDEO_UV_STRIDE_BYTES(v4l2_fmt, width), VIDEO_UV_SCANLINES(v4l2_fmt, height),
			VIDEO_Y_META_STRIDE(v4l2_fmt, width), VIDEO_Y_META_SCANLINES(v4l2_fmt,
				height),
			VIDEO_UV_META_STRIDE(v4l2_fmt, width), VIDEO_UV_META_SCANLINES(v4l2_fmt,
				height));
	} else if (color_fmt == MSM_VIDC_FMT_P010) {
		v4l2_fmt = V4L2_PIX_FMT_VIDC_TP10C;
		HFI_YUV420_TP10_UBWC_CALC_BUF_SIZE(size,
			VIDEO_Y_STRIDE_BYTES(v4l2_fmt, width), VIDEO_Y_SCANLINES(v4l2_fmt, height),
			VIDEO_UV_STRIDE_BYTES(v4l2_fmt, width), VIDEO_UV_SCANLINES(v4l2_fmt, height),
			VIDEO_Y_META_STRIDE(v4l2_fmt, width), VIDEO_Y_META_SCANLINES(v4l2_fmt,
				height),
			VIDEO_UV_META_STRIDE(v4l2_fmt, width), VIDEO_UV_META_SCANLINES(v4l2_fmt,
				height));
	}

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

/* encoder internal buffers */
static u32 msm_vidc_encoder_bin_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes, stage;
	struct v4l2_format *f;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	stage = inst->capabilities->cap[STAGE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_BIN_H264E(size, width, height, stage, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_BIN_H265E(size, width, height, stage, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, num_ref = 0, num_recon = 0;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(num_recon, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_AVC);
		num_ref = num_recon - 1;
		HFI_BUFFER_COMV_H264E(size, width, height, num_ref);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(num_recon, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_HEVC);
		num_ref = num_recon - 1;
		HFI_BUFFER_COMV_H265E(size, width, height, num_ref);
	}

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_non_comv_size_iris2(struct msm_vidc_inst* inst)
{
	struct msm_vidc_core* core;
	u32 size = 0;
	u32 width, height, num_vpp_pipes;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_NON_COMV_H264E(size, width, height, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_NON_COMV_H265E(size, width, height, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_line_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, pixelformat, num_vpp_pipes;
	bool is_tenbit = false;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid core capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_LINE_H264E(size, width, height, is_tenbit, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_LINE_H265E(size, width, height, is_tenbit, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_dpb_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;
	u32 width, height, pixelformat;
	struct v4l2_format *f;
	bool is_tenbit;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_DPB_H264E(size, width, height);
	else if (inst->codec == MSM_VIDC_HEVC)
		HFI_BUFFER_DPB_H265E(size, width, height, is_tenbit);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_arp_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	HFI_BUFFER_ARP_ENC(size);
	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_vpss_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	bool ds_enable, rot_enable, flip_enable, is_tenbit;
	u32 width, height, pixelformat;
	struct v4l2_format* f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}
	ds_enable = false; // TODO: fixme
	rot_enable = false; // TODO: fixme
	flip_enable = false; // TODO: fixme
	is_tenbit = false;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	pixelformat = f->fmt.pix_mp.pixelformat;
	if (pixelformat == MSM_VIDC_FMT_P010 ||
		pixelformat == MSM_VIDC_FMT_TP10C)
		is_tenbit = true;
	else
		is_tenbit = false;

	HFI_BUFFER_VPSS_ENC(size, width, height, ds_enable,
		rot_enable, flip_enable, is_tenbit);
	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

int msm_buffer_size_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int size = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	if (is_decode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
			size = msm_vidc_decoder_input_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
			size = msm_vidc_decoder_output_size(inst);
			break;
		case MSM_VIDC_BUF_INPUT_META:
			size = msm_vidc_decoder_input_meta_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT_META:
			size = msm_vidc_decoder_output_meta_size(inst);
			break;
		case MSM_VIDC_BUF_BIN:
			size = msm_vidc_decoder_bin_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_COMV:
			size = msm_vidc_decoder_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_NON_COMV:
			size = msm_vidc_decoder_non_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_LINE:
			size = msm_vidc_decoder_line_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_PERSIST:
			size = msm_vidc_decoder_persist_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_DPB:
			size = msm_vidc_decoder_dpb_size_iris2(inst);
			break;
		default:
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
			size = msm_vidc_encoder_input_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
			size = msm_vidc_encoder_output_size(inst);
			break;
		case MSM_VIDC_BUF_INPUT_META:
			size = msm_vidc_encoder_input_meta_size(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT_META:
			size = msm_vidc_encoder_output_meta_size(inst);
			break;
		case MSM_VIDC_BUF_BIN:
			size = msm_vidc_encoder_bin_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_COMV:
			size = msm_vidc_encoder_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_NON_COMV:
			size = msm_vidc_encoder_non_comv_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_LINE:
			size = msm_vidc_encoder_line_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_DPB:
			size = msm_vidc_encoder_dpb_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_ARP:
			size = msm_vidc_encoder_arp_size_iris2(inst);
			break;
		case MSM_VIDC_BUF_VPSS:
			size = msm_vidc_encoder_vpss_size_iris2(inst);
			break;
		default:
			break;
		}
	}

	return size;
}

static int msm_buffer_encoder_dpb_count(struct msm_vidc_inst *inst)
{
	int count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (inst->codec == MSM_VIDC_H264) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(count, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_AVC);
	} else if (inst->codec == MSM_VIDC_HEVC) {
		// TODO: replace zeros with appropriate variables
		HFI_IRIS2_ENC_RECON_BUF_COUNT(count, 0, 0, 0, 0, 0,
			HFI_CODEC_ENCODE_HEVC);
	}
	return count;
}

static int msm_buffer_decoder_dpb_count(struct msm_vidc_inst *inst)
{
	int count = 0;
	u32 color_fmt;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	color_fmt = inst->capabilities->cap[PIX_FMTS].value;
	if (is_linear_colorformat(color_fmt))
		count = inst->buffers.output.min_count;

	return count;
}

int msm_buffer_min_count_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (is_decode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
			count = msm_vidc_input_min_count(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
			count = msm_vidc_output_min_count(inst);
			break;
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
		case MSM_VIDC_BUF_PERSIST:
			count = msm_vidc_internal_buffer_count(inst, buffer_type);
			break;
		case MSM_VIDC_BUF_DPB:
			count = msm_buffer_decoder_dpb_count(inst);
			break;
		default:
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
			count = msm_vidc_input_min_count(inst);
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
			count = msm_vidc_output_min_count(inst);
			break;
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
		case MSM_VIDC_BUF_ARP:
		case MSM_VIDC_BUF_VPSS:
			count = 1;
			break;
		case MSM_VIDC_BUF_DPB:
			count = msm_buffer_encoder_dpb_count(inst);
		default:
			break;
		}
	}

	return count;
}

int msm_buffer_extra_count_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
	case MSM_VIDC_BUF_INPUT_META:
		count = msm_vidc_input_extra_count(inst);
		break;
	case MSM_VIDC_BUF_OUTPUT:
	case MSM_VIDC_BUF_OUTPUT_META:
		count = msm_vidc_output_extra_count(inst);
		break;
	default:
		break;
	}

	return count;
}

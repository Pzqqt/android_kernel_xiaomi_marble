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
#include "msm_vidc_control.h"

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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
		HFI_BUFFER_BIN_H264E(size, inst->hfi_rc_type, width,
			height, stage, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
		HFI_BUFFER_BIN_H265E(size, inst->hfi_rc_type, width,
			height, stage, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_get_recon_buf_count(struct msm_vidc_inst *inst)
{
	u32 num_buf_recon = 0;
	s32 n_bframe, ltr_count, hp_layers = 0, hb_layers = 0;
	bool is_hybrid_hp = false;
	u32 hfi_codec = 0;

	n_bframe = inst->capabilities->cap[B_FRAME].value;
	ltr_count = inst->capabilities->cap[LTR_COUNT].value;

	if (inst->hfi_layer_type == HFI_HIER_B) {
		hb_layers = inst->capabilities->cap[ENH_LAYER_COUNT].value + 1;
	} else {
		hp_layers = inst->capabilities->cap[ENH_LAYER_COUNT].value + 1;
		if (inst->hfi_layer_type == HFI_HIER_P_HYBRID_LTR)
			is_hybrid_hp = true;
	}

	if (inst->codec == MSM_VIDC_H264)
		hfi_codec = HFI_CODEC_ENCODE_AVC;
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
		hfi_codec = HFI_CODEC_ENCODE_HEVC;

	HFI_IRIS2_ENC_RECON_BUF_COUNT(num_buf_recon, n_bframe, ltr_count,
			hp_layers, hb_layers, is_hybrid_hp, hfi_codec);

	return num_buf_recon;
}

static u32 msm_vidc_encoder_comv_size_iris2(struct msm_vidc_inst* inst)
{
	u32 size = 0;
	u32 width, height, num_recon = 0;
	struct v4l2_format* f;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	num_recon = msm_vidc_get_recon_buf_count(inst);
	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_COMV_H264E(size, width, height, num_recon);
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
		HFI_BUFFER_COMV_H265E(size, width, height, num_recon);

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
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
		HFI_BUFFER_NON_COMV_H265E(size, width, height, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_line_size_iris2(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 size = 0;
	u32 width, height, pixfmt, num_vpp_pipes;
	bool is_tenbit = false;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return size;
	}
	core = inst->core;
	if (!core->capabilities || !inst->capabilities) {
		i_vpr_e(inst, "%s: invalid capabilities\n", __func__);
		return size;
	}
	num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	pixfmt = inst->capabilities->cap[PIX_FMTS].value;

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;
	is_tenbit = (pixfmt == MSM_VIDC_FMT_P010 || pixfmt == MSM_VIDC_FMT_TP10C);

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_LINE_H264E(size, width, height, is_tenbit, num_vpp_pipes);
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
		HFI_BUFFER_LINE_H265E(size, width, height, is_tenbit, num_vpp_pipes);

	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

static u32 msm_vidc_encoder_dpb_size_iris2(struct msm_vidc_inst *inst)
{
	u32 size = 0;
	u32 width, height, pixfmt;
	struct v4l2_format *f;
	bool is_tenbit;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	pixfmt = inst->capabilities->cap[PIX_FMTS].value;
	is_tenbit = (pixfmt == MSM_VIDC_FMT_P010 || pixfmt == MSM_VIDC_FMT_TP10C);

	if (inst->codec == MSM_VIDC_H264)
		HFI_BUFFER_DPB_H264E(size, width, height);
	else if (inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)
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
	bool ds_enable = false, is_tenbit = false, blur = false;
	u32 rotation_val = HFI_ROTATION_NONE;
	u32 width, height, driver_colorfmt;
	struct v4l2_format* f;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	ds_enable = is_scaling_enabled(inst);
	msm_vidc_v4l2_to_hfi_enum(inst, ROTATION, &rotation_val);

	f = &inst->fmts[OUTPUT_PORT];
	if (is_rotation_90_or_270(inst)) {
		/*
		 * output width and height are rotated,
		 * so unrotate them to use as arguments to
		 * HFI_BUFFER_VPSS_ENC.
		 */
		width = f->fmt.pix_mp.height;
		height = f->fmt.pix_mp.width;
	} else {
		width = f->fmt.pix_mp.width;
		height = f->fmt.pix_mp.height;
	}

	f = &inst->fmts[INPUT_PORT];
	driver_colorfmt = v4l2_colorformat_to_driver(
			f->fmt.pix_mp.pixelformat, __func__);
	is_tenbit = is_10bit_colorformat(driver_colorfmt);
	if (inst->capabilities->cap[BLUR_TYPES].value != VIDC_BLUR_NONE)
		blur = true;

	HFI_BUFFER_VPSS_ENC(size, width, height, ds_enable, blur, is_tenbit);
	i_vpr_l(inst, "%s: size %d\n", __func__, size);
	return size;
}

struct msm_vidc_buf_type_handle {
	enum msm_vidc_buffer_type type;
	u32 (*handle)(struct msm_vidc_inst *inst);
};

int msm_buffer_size_iris2(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int i;
	u32 size = 0, buf_type_handle_size = 0;
	const struct msm_vidc_buf_type_handle *buf_type_handle_arr = NULL;
	static const struct msm_vidc_buf_type_handle dec_buf_type_handle[] = {
		{MSM_VIDC_BUF_INPUT,           msm_vidc_decoder_input_size              },
		{MSM_VIDC_BUF_OUTPUT,          msm_vidc_decoder_output_size             },
		{MSM_VIDC_BUF_INPUT_META,      msm_vidc_decoder_input_meta_size         },
		{MSM_VIDC_BUF_OUTPUT_META,     msm_vidc_decoder_output_meta_size        },
		{MSM_VIDC_BUF_BIN,             msm_vidc_decoder_bin_size_iris2          },
		{MSM_VIDC_BUF_COMV,            msm_vidc_decoder_comv_size_iris2         },
		{MSM_VIDC_BUF_NON_COMV,        msm_vidc_decoder_non_comv_size_iris2     },
		{MSM_VIDC_BUF_LINE,            msm_vidc_decoder_line_size_iris2         },
		{MSM_VIDC_BUF_PERSIST,         msm_vidc_decoder_persist_size_iris2      },
		{MSM_VIDC_BUF_DPB,             msm_vidc_decoder_dpb_size_iris2          },
	};
	static const struct msm_vidc_buf_type_handle enc_buf_type_handle[] = {
		{MSM_VIDC_BUF_INPUT,           msm_vidc_encoder_input_size              },
		{MSM_VIDC_BUF_OUTPUT,          msm_vidc_encoder_output_size             },
		{MSM_VIDC_BUF_INPUT_META,      msm_vidc_encoder_input_meta_size         },
		{MSM_VIDC_BUF_OUTPUT_META,     msm_vidc_encoder_output_meta_size        },
		{MSM_VIDC_BUF_BIN,             msm_vidc_encoder_bin_size_iris2          },
		{MSM_VIDC_BUF_COMV,            msm_vidc_encoder_comv_size_iris2         },
		{MSM_VIDC_BUF_NON_COMV,        msm_vidc_encoder_non_comv_size_iris2     },
		{MSM_VIDC_BUF_LINE,            msm_vidc_encoder_line_size_iris2         },
		{MSM_VIDC_BUF_DPB,             msm_vidc_encoder_dpb_size_iris2          },
		{MSM_VIDC_BUF_ARP,             msm_vidc_encoder_arp_size_iris2          },
		{MSM_VIDC_BUF_VPSS,            msm_vidc_encoder_vpss_size_iris2         },
	};

	if (is_decode_session(inst)) {
		buf_type_handle_size = ARRAY_SIZE(dec_buf_type_handle);
		buf_type_handle_arr = dec_buf_type_handle;
	} else if (is_encode_session(inst)) {
		buf_type_handle_size = ARRAY_SIZE(enc_buf_type_handle);
		buf_type_handle_arr = enc_buf_type_handle;
	}

	/* handle invalid session */
	if (!buf_type_handle_arr || !buf_type_handle_size) {
		i_vpr_e(inst, "%s: invalid session %d\n", __func__, inst->domain);
		return size;
	}

	/* fetch buffer size */
	for (i = 0; i < buf_type_handle_size; i++) {
		if (buf_type_handle_arr[i].type == buffer_type) {
			size = buf_type_handle_arr[i].handle(inst);
			break;
		}
	}

	/* handle unknown buffer type */
	if (i == buf_type_handle_size) {
		i_vpr_e(inst, "%s: unknown buffer type %#x\n", __func__, buffer_type);
		goto exit;
	}

	i_vpr_l(inst, "buffer_size: type: %11s,  size: %9u\n", buf_name(buffer_type), size);

exit:
	return size;
}

static int msm_vidc_input_min_count_iris2(struct msm_vidc_inst* inst)
{
	u32 input_min_count = 0;
	u32 total_hb_layer = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (is_decode_session(inst)) {
		input_min_count = MIN_DEC_INPUT_BUFFERS;
	} else if (is_encode_session(inst)) {
		total_hb_layer = is_hierb_type_requested(inst) ?
			inst->capabilities->cap[ENH_LAYER_COUNT].value + 1 : 0;
		if (inst->codec == MSM_VIDC_H264 &&
			!inst->capabilities->cap[LAYER_ENABLE].value) {
			total_hb_layer = 0;
		}
		HFI_IRIS2_ENC_MIN_INPUT_BUF_COUNT(input_min_count,
			total_hb_layer);
	} else {
		i_vpr_e(inst, "%s: invalid domain %d\n", __func__, inst->domain);
		return 0;
	}

	if (is_thumbnail_session(inst) || is_image_session(inst))
		input_min_count = 1;

	return input_min_count;
}

static int msm_buffer_dpb_count(struct msm_vidc_inst *inst)
{
	int count = 0;
	u32 color_fmt;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	/* decoder dpb buffer count */
	if (is_decode_session(inst)) {
		color_fmt = inst->capabilities->cap[PIX_FMTS].value;
		if (is_linear_colorformat(color_fmt))
			count = inst->buffers.output.min_count;

		return count;
	}

	/* encoder dpb buffer count */
	return msm_vidc_get_recon_buf_count(inst);
}

int msm_buffer_min_count_iris2(struct msm_vidc_inst *inst,
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
		count = msm_vidc_input_min_count_iris2(inst);
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
	case MSM_VIDC_BUF_ARP:
	case MSM_VIDC_BUF_VPSS:
		count = msm_vidc_internal_buffer_count(inst, buffer_type);
		break;
	case MSM_VIDC_BUF_DPB:
		count = msm_buffer_dpb_count(inst);
		break;
	default:
		break;
	}

	i_vpr_l(inst, "  min_count: type: %11s, count: %9u\n", buf_name(buffer_type), count);
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

	i_vpr_l(inst, "extra_count: type: %11s, count: %9u\n", buf_name(buffer_type), count);
	return count;
}

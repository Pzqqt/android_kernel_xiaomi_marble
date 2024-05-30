// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "msm_media_info.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"

/* Generic function for all targets. Not being used for iris2 */
u32 msm_vidc_input_min_count(struct msm_vidc_inst* inst)
{
	u32 input_min_count = 0;
	u32 hb_enh_layer = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (is_decode_session(inst)) {
		input_min_count = MIN_DEC_INPUT_BUFFERS;
	} else if (is_encode_session(inst)) {
		input_min_count = MIN_ENC_INPUT_BUFFERS;
		if (is_hierb_type_requested(inst)) {
			hb_enh_layer =
				inst->capabilities->cap[ENH_LAYER_COUNT].value;
			if (inst->codec == MSM_VIDC_H264 &&
				!inst->capabilities->cap[LAYER_ENABLE].value) {
				hb_enh_layer = 0;
			}
			if (hb_enh_layer)
				input_min_count = (1 << hb_enh_layer) + 2;
		}
	} else {
		i_vpr_e(inst, "%s: invalid domain %d\n",
			__func__, inst->domain);
		return 0;
	}

	if (is_thumbnail_session(inst) || is_image_session(inst))
		input_min_count = 1;

	return input_min_count;
}

u32 msm_vidc_output_min_count(struct msm_vidc_inst *inst)
{
	u32 output_min_count;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (!is_decode_session(inst) && !is_encode_session(inst))
		return 0;

	if (is_thumbnail_session(inst))
		return 1;

	if (is_encode_session(inst))
		return MIN_ENC_OUTPUT_BUFFERS;

	switch (inst->codec) {
	case MSM_VIDC_H264:
	case MSM_VIDC_HEVC:
		output_min_count = 4;
		break;
	case MSM_VIDC_VP9:
		output_min_count = 9;
		break;
	case MSM_VIDC_HEIC:
		output_min_count = 3;
		break;
	default:
		output_min_count = 4;
	}

	if (inst->buffers.output.min_count)
		output_min_count = max(inst->buffers.output.min_count,
							output_min_count);

	return output_min_count;
}

u32 msm_vidc_input_extra_count(struct msm_vidc_inst *inst)
{
	u32 count = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return 0;
	}
	core = inst->core;

	/*
	 * no extra buffers for thumbnail session because
	 * neither dcvs nor batching will be enabled
	 */
	if (is_thumbnail_session(inst) || is_image_session(inst))
		return 0;

	if (is_decode_session(inst)) {
		/*
		 * if decode batching enabled, ensure minimum batch size
		 * count of input buffers present on input port
		 */
		if (core->capabilities[DECODE_BATCH].value &&
			inst->decode_batch.enable) {
			if (inst->buffers.input.min_count < inst->decode_batch.size) {
				count = inst->decode_batch.size -
					inst->buffers.input.min_count;
			}
		}
	} else if (is_encode_session(inst)) {
		/* add dcvs buffers, if platform supports dcvs */
		if (core->capabilities[DCVS].value)
			count = DCVS_ENC_EXTRA_INPUT_BUFFERS;
	}

	return count;
}

u32 msm_vidc_output_extra_count(struct msm_vidc_inst *inst)
{
	u32 count = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return 0;
	}
	core = inst->core;

	/*
	 * no extra buffers for thumbnail session because
	 * neither dcvs nor batching will be enabled
	 */
	if (is_thumbnail_session(inst) || is_image_session(inst))
		return 0;

	if (is_decode_session(inst)) {
		/* add dcvs buffers, if platform supports dcvs */
		if (core->capabilities[DCVS].value)
			count = DCVS_DEC_EXTRA_OUTPUT_BUFFERS;
		/*
		 * if decode batching enabled, ensure minimum batch size
		 * count of extra output buffers added on output port
		 */
		if (core->capabilities[DECODE_BATCH].value &&
			inst->decode_batch.enable &&
			count < inst->decode_batch.size)
			count = inst->decode_batch.size;

	}

	return count;
}

u32 msm_vidc_internal_buffer_count(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	u32 count = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (is_encode_session(inst))
		return 1;

	if (is_decode_session(inst)) {
		if (buffer_type == MSM_VIDC_BUF_BIN ||
			buffer_type == MSM_VIDC_BUF_LINE ||
			buffer_type == MSM_VIDC_BUF_PERSIST) {
			count = 1;
		} else if (buffer_type == MSM_VIDC_BUF_COMV ||
			buffer_type == MSM_VIDC_BUF_NON_COMV) {
			if (inst->codec == MSM_VIDC_H264 ||
				inst->codec == MSM_VIDC_HEVC ||
				inst->codec == MSM_VIDC_HEIC)
				count = 1;
			else
				count = 0;
		} else {
			i_vpr_e(inst, "%s: unsupported buffer type %s\n",
				__func__, buf_name(buffer_type));
			count = 0;
		}
	}

	return count;
}

u32 msm_vidc_decoder_input_size(struct msm_vidc_inst *inst)
{
	u32 frame_size, num_mbs;
	u32 div_factor = 1;
	u32 base_res_mbs = NUM_MBS_4k;
	struct v4l2_format *f;
	u32 bitstream_size_overwrite = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	bitstream_size_overwrite =
		inst->capabilities->cap[BITSTREAM_SIZE_OVERWRITE].value;
	if (bitstream_size_overwrite) {
		frame_size = bitstream_size_overwrite;
		i_vpr_h(inst, "client configured bitstream buffer size %d\n",
			frame_size);
		return frame_size;
	}

	/*
	 * Decoder input size calculation:
	 * For 8k resolution, buffer size is calculated as 8k mbs / 4 and
	 * for 8k cases we expect width/height to be set always.
	 * In all other cases, buffer size is calculated as
	 * 4k mbs for VP8/VP9 and 4k / 2 for remaining codecs.
	 */
	f = &inst->fmts[INPUT_PORT];
	num_mbs = msm_vidc_get_mbs_per_frame(inst);
	if (num_mbs > NUM_MBS_4k) {
		div_factor = 4;
		base_res_mbs = inst->capabilities->cap[MBPF].value;
	} else {
		base_res_mbs = NUM_MBS_4k;
		if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_VP9)
			div_factor = 1;
		else
			div_factor = 2;
	}

	if (is_secure_session(inst))
		div_factor = div_factor << 1;

	/* For image session, use the actual resolution to calc buffer size */
	if (is_image_session(inst)) {
		base_res_mbs = num_mbs;
		div_factor = 1;
	}

	frame_size = base_res_mbs * MB_SIZE_IN_PIXEL * 3 / 2 / div_factor;

	 /* multiply by 10/8 (1.25) to get size for 10 bit case */
	if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_VP9 ||
		f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC ||
		f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEIC)
		frame_size = frame_size + (frame_size >> 2);

	i_vpr_h(inst, "set input buffer size to %d\n", frame_size);

	return ALIGN(frame_size, SZ_4K);
}

u32 msm_vidc_decoder_output_size(struct msm_vidc_inst *inst)
{
	u32 size;
	struct v4l2_format *f;

	f = &inst->fmts[OUTPUT_PORT];
	size = VIDEO_RAW_BUFFER_SIZE(f->fmt.pix_mp.pixelformat,
			f->fmt.pix_mp.width,
			f->fmt.pix_mp.height, true);
	return size;
}

u32 msm_vidc_decoder_input_meta_size(struct msm_vidc_inst *inst)
{
	return ALIGN(16 * 1024, SZ_4K);
}

u32 msm_vidc_decoder_output_meta_size(struct msm_vidc_inst *inst)
{
	return ALIGN(16 * 1024, SZ_4K);
}

u32 msm_vidc_encoder_input_size(struct msm_vidc_inst *inst)
{
	u32 size;
	struct v4l2_format *f;

	f = &inst->fmts[INPUT_PORT];
	size = VIDEO_RAW_BUFFER_SIZE(f->fmt.pix_mp.pixelformat,
			f->fmt.pix_mp.width,
			f->fmt.pix_mp.height, true);
	return size;
}

u32 msm_vidc_encoder_output_size(struct msm_vidc_inst *inst)
{
	u32 frame_size;
	u32 mbs_per_frame;
	u32 width, height;
	struct v4l2_format *f;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	f = &inst->fmts[OUTPUT_PORT];
	/*
	 * Encoder output size calculation: 32 Align width/height
	 * For heic session : YUVsize * 2
	 * For resolution <= 480x360p : YUVsize * 2
	 * For resolution > 360p & <= FHD : YUVsize : if CAC disabled
	 * For resolution > 360p & <= 4K : YUVsize / 2
	 * For resolution > 4k : YUVsize / 4
	 * Initially frame_size = YUVsize * 2;
	 */

	width = ALIGN(f->fmt.pix_mp.width, BUFFER_ALIGNMENT_SIZE(32));
	height = ALIGN(f->fmt.pix_mp.height, BUFFER_ALIGNMENT_SIZE(32));
	mbs_per_frame = NUM_MBS_PER_FRAME(width, height);
	frame_size = (width * height * 3);

	/* Image session: 2 x yuv size */
	if (is_image_session(inst) ||
		inst->capabilities->cap[BITRATE_MODE].value == V4L2_MPEG_VIDEO_BITRATE_MODE_CQ)
		goto skip_calc;

	if (mbs_per_frame <= NUM_MBS_360P)
		(void)frame_size; /* Default frame_size = YUVsize * 2 */
	else if (inst->capabilities->cap[CONTENT_ADAPTIVE_CODING].value ==
				V4L2_MPEG_MSM_VIDC_DISABLE && mbs_per_frame <= NUM_MBS_FHD)
		frame_size = frame_size >> 1;
	else if (mbs_per_frame <= NUM_MBS_4k)
		frame_size = frame_size >> 2;
	else
		frame_size = frame_size >> 3;

	/*if ((inst->rc_type == RATE_CONTROL_OFF) ||
		(inst->rc_type == V4L2_MPEG_VIDEO_BITRATE_MODE_CQ))
		frame_size = frame_size << 1;

	if (inst->rc_type == RATE_CONTROL_LOSSLESS)
		frame_size = (width * height * 9) >> 2; */

skip_calc:
	/* multiply by 10/8 (1.25) to get size for 10 bit case */
	if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC ||
		f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEIC)
		frame_size = frame_size + (frame_size >> 2);

	return ALIGN(frame_size, SZ_4K);
}

static inline u32 ROI_METADATA_SIZE(
	u32 width, u32 height, u32 lcu_size) {
	u32 lcu_width = 0;
	u32 lcu_height = 0;
	u32 n_shift = 0;

	while (lcu_size && !(lcu_size & 0x1)) {
		n_shift++;
		lcu_size = lcu_size >> 1;
	}
	lcu_width = (width + (lcu_size - 1)) >> n_shift;
	lcu_height = (height + (lcu_size - 1)) >> n_shift;

	return (((lcu_width + 7) >> 3) << 3) * lcu_height * 2;
}

u32 msm_vidc_encoder_input_meta_size(struct msm_vidc_inst *inst)
{
	u32 size = 0;
	u32 lcu_size = 0;
	struct v4l2_format *f;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	size = ALIGN(16 * 1024, SZ_4K);

	if (inst->capabilities->cap[META_ROI_INFO].value) {
		lcu_size = 16;

		f = &inst->fmts[OUTPUT_PORT];
		if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC)
			lcu_size = 32;

		f = &inst->fmts[INPUT_PORT];
		size += ROI_METADATA_SIZE(f->fmt.pix_mp.width,
			f->fmt.pix_mp.height, lcu_size);
		size = ALIGN(size, SZ_4K);
	}
	return size;
}

u32 msm_vidc_encoder_output_meta_size(struct msm_vidc_inst *inst)
{
	return ALIGN(16 * 1024, SZ_4K);
}

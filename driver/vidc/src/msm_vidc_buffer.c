// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_media_info.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"

u32 msm_vidc_input_min_count(struct msm_vidc_inst* inst)
{
	u32 input_min_count = 0;
	//struct v4l2_ctrl *max_layer = NULL;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	if (is_decode_session(inst)) {
		input_min_count = MIN_DEC_INPUT_BUFFERS;
	} else if (is_encode_session(inst)) {
		input_min_count = MIN_ENC_INPUT_BUFFERS;
	} else {
		i_vpr_e(inst, "%s: invalid domain\n",
			__func__, inst->domain);
		return 0;
	}

	if (is_thumbnail_session(inst))
		input_min_count = 1;

	//if (is_grid_session(inst))
	//	input_min_count = 2;

	//if (is_hier_b_session(inst)) {
		//max_layer = get_ctrl(inst,
		//	V4L2_CID_MPEG_VIDC_VIDEO_HEVC_MAX_HIER_CODING_LAYER);
		//input_min_count = (1 << (max_layer->val - 1)) + 2;
	//}

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
		return inst->codec == MSM_VIDC_VP9 ? 8 : 1;

	if (is_decode_session(inst)) {
		switch (inst->codec) {
		case MSM_VIDC_H264:
		case MSM_VIDC_HEVC:
			output_min_count = 4;
			break;
		case MSM_VIDC_VP9:
			output_min_count = 9;
			break;
		default:
			output_min_count = 4;
		}
	} else {
		output_min_count = MIN_ENC_OUTPUT_BUFFERS;
	}

	//if (is_vpp_delay_allowed(inst)) {
	//	output_min_count =
	//		max(output_min_count, (u32)MAX_BSE_VPP_DELAY);
	//	output_min_count =
	//		max(output_min_count, (u32)(msm_vidc_vpp_delay & 0x1F));
	//}

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
	if (is_thumbnail_session(inst))
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
		/* add dcvs buffers */
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
	if (is_thumbnail_session(inst))
		return 0;

	if (is_decode_session(inst)) {
		/* add dcvs buffers */
		count = DCVS_DEC_EXTRA_OUTPUT_BUFFERS;
		/*
		 * if decode batching enabled, ensure minimum batch size
		 * count of extra output buffers added on output port
		 */
		if (core->capabilities[DECODE_BATCH].value &&
			inst->decode_batch.enable &&
			count < inst->decode_batch.size)
			count = inst->decode_batch.size;

	} else if (is_encode_session(inst)) {
		/* add heif buffers */
		//count = 8
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

	if (is_decode_session(inst)) {
		if (buffer_type == MSM_VIDC_BUF_BIN ||
			buffer_type == MSM_VIDC_BUF_LINE ||
			buffer_type == MSM_VIDC_BUF_PERSIST) {
			count = 1;
		} else if (buffer_type == MSM_VIDC_BUF_COMV ||
			buffer_type == MSM_VIDC_BUF_NON_COMV) {
			if (inst->codec == MSM_VIDC_HEVC ||
				inst->codec == MSM_VIDC_H264)
				count = 1;
			else
				count = 0;
		} else {
			d_vpr_e("%s: unsupported buffer type %#x\n",
				__func__, buffer_type);
			count = 0;
		}
	}
	//todo: add enc support if needed

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
		d_vpr_e("%s: invalid params\n");
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

	/* For HEIF image, use the actual resolution to calc buffer size */
	/* TODO: fix me
	if (is_heif_decoder(inst)) {
		base_res_mbs = num_mbs;
		div_factor = 1;
	}
	*/

	frame_size = base_res_mbs * MB_SIZE_IN_PIXEL * 3 / 2 / div_factor;

	 /* multiply by 10/8 (1.25) to get size for 10 bit case */
	if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_VP9 ||
		f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC)
		frame_size = frame_size + (frame_size >> 2);

	i_vpr_h(inst, "set input buffer size to %d\n", frame_size);

	return ALIGN(frame_size, SZ_4K);
}

u32 msm_vidc_decoder_output_size(struct msm_vidc_inst *inst)
{
	u32 size;
	u32 format;
	struct v4l2_format *f;

	f = &inst->fmts[OUTPUT_PORT];
	format = v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__);
	size = VENUS_BUFFER_SIZE(format, f->fmt.pix_mp.width,
			f->fmt.pix_mp.height);
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
	u32 format;
	struct v4l2_format *f;

	f = &inst->fmts[INPUT_PORT];
	format = v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__);
	size = VENUS_BUFFER_SIZE_USED(format, f->fmt.pix_mp.width,
			f->fmt.pix_mp.height, false);
	return size;
}

u32 msm_vidc_encoder_output_size(struct msm_vidc_inst *inst)
{
	u32 frame_size;
	u32 mbs_per_frame;
	u32 width, height;
	struct v4l2_format *f;

	f = &inst->fmts[OUTPUT_PORT];
	/*
	 * Encoder output size calculation: 32 Align width/height
	 * For resolution < 720p : YUVsize * 4
	 * For resolution > 720p & <= 4K : YUVsize / 2
	 * For resolution > 4k : YUVsize / 4
	 * Initially frame_size = YUVsize * 2;
	 */

	/* if (is_grid_session(inst)) {
		f->fmt.pix_mp.width = f->fmt.pix_mp.height = HEIC_GRID_DIMENSION;
	} */
	width = ALIGN(f->fmt.pix_mp.width, BUFFER_ALIGNMENT_SIZE(32));
	height = ALIGN(f->fmt.pix_mp.height, BUFFER_ALIGNMENT_SIZE(32));
	mbs_per_frame = NUM_MBS_PER_FRAME(width, height);
	frame_size = (width * height * 3);

	if (mbs_per_frame < NUM_MBS_720P)
		frame_size = frame_size << 1;
	else if (mbs_per_frame <= NUM_MBS_4k)
		frame_size = frame_size >> 2;
	else
		frame_size = frame_size >> 3;

	/*if ((inst->rc_type == RATE_CONTROL_OFF) ||
		(inst->rc_type == V4L2_MPEG_VIDEO_BITRATE_MODE_CQ))
		frame_size = frame_size << 1;

	if (inst->rc_type == RATE_CONTROL_LOSSLESS)
		frame_size = (width * height * 9) >> 2; */

	/* multiply by 10/8 (1.25) to get size for 10 bit case */
	if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC)
		frame_size = frame_size + (frame_size >> 2);

	return ALIGN(frame_size, SZ_4K);
}

u32 msm_vidc_encoder_input_meta_size(struct msm_vidc_inst *inst)
{
	return ALIGN(1 * 1024 * 1024, SZ_4K);
}

u32 msm_vidc_encoder_output_meta_size(struct msm_vidc_inst *inst)
{
	return ALIGN(16 * 1024, SZ_4K);
}

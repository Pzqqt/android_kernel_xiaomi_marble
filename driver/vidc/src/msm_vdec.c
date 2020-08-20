// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/msm_vidc_utils.h>
#include <media/msm_media_info.h>

#include "msm_vdec.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"

static int msm_vdec_codec_change(struct msm_vidc_inst *inst, u32 codec)
{
	int rc = 0;
	int i;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	inst->capabilities = NULL;
	for (i = 0; i < core->codecs_count; i++) {
		if (core->inst_caps[i].domain == MSM_VIDC_DECODER &&
		    core->inst_caps[i].codec == get_vidc_codec_from_v4l2(
				inst->fmts[INPUT_PORT].fmt.pix.pixelformat)) {
			s_vpr_h(inst->sid, "%s: changed capabilities to %#x caps\n",
				__func__, inst->fmts[INPUT_PORT].fmt.pix.pixelformat);
			inst->capabilities = &core->inst_caps[i];
		}
	}
	if (!inst->capabilities) {
		s_vpr_e(inst->sid, "%s: capabilities not found\n", __func__);
		return -EINVAL;
	}
	return rc;
}

static int msm_vdec_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int msm_vdec_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	inst->buffers.scratch.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_SCRATCH);
	inst->buffers.scratch_1.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_SCRATCH_1);
	inst->buffers.persist_1.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_PERSIST_1);

	inst->buffers.scratch.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_SCRATCH);
	inst->buffers.scratch_1.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_SCRATCH_1);
	inst->buffers.persist_1.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_PERSIST_1);

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

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH_1);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_PERSIST_1);
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

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH_1);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_PERSIST_1);
	if (rc)
		return rc;

	return 0;
}
/*
static int msm_vdec_release_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH_1);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_PERSIST_1);
	if (rc)
		return rc;

	return 0;
}
*/
int msm_vdec_stop_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	return rc;
}

int msm_vdec_start_input(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;
	//rc = msm_vidc_check_scaling_supported(inst);
	if (rc)
		goto error;
	rc = call_session_op(core, decide_work_mode, inst);
	if (rc)
		goto error;
	rc = call_session_op(core, decide_work_route, inst);
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

	rc = msm_vdec_set_input_properties(inst);
	if (rc)
		goto error;
	rc = msm_vdec_create_input_internal_buffers(inst);
	if (rc)
		goto error;
	rc = msm_vdec_queue_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = venus_hfi_start_input(inst);
	if (rc)
		goto error;

	rc = msm_vidc_change_inst_state(inst, MSM_VIDC_START_INPUT);
	if (rc)
		goto error;

	d_vpr_h("%s: done\n", __func__);
	return 0;

error:
	msm_vdec_stop_input(inst);
	return rc;
}

int msm_vdec_stop_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	return rc;
}

int msm_vdec_start_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	return rc;
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

	if (f->type == INPUT_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		if (inst->fmts[INPUT_PORT].fmt.pix.pixelformat !=
			f->fmt.pix.pixelformat) {
			s_vpr_e(inst->sid,
				"%s: codec changed from %#x to %#x\n", __func__,
				inst->fmts[INPUT_PORT].fmt.pix.pixelformat,
				f->fmt.pix.pixelformat);
			rc = msm_vdec_codec_change(inst, f->fmt.pix.pixelformat);
			if (rc)
				goto err_invalid_fmt;
		}
		fmt = &inst->fmts[INPUT_PORT];
		fmt->type = INPUT_PLANE;
		fmt->fmt.pix.width = ALIGN(f->fmt.pix.width, 16);
		fmt->fmt.pix.height = ALIGN(f->fmt.pix.height, 16);
		fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;
		fmt->fmt.pix.bytesperline = 0;
		fmt->fmt.pix.sizeimage = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_BUF_INPUT);
		if (inst->buffers.input.actual_count <
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count) {
			inst->buffers.input.actual_count =
				inst->buffers.input.min_count +
				inst->buffers.input.extra_count;
		}
		inst->buffers.input.size = fmt->fmt.pix.sizeimage;

		/* update crop dimensions */
		inst->crop.x = inst->crop.y = 0;
		inst->crop.width = f->fmt.pix.width;
		inst->crop.height = f->fmt.pix.height;

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		//update_log_ctxt(inst->sid, inst->session_type,
		//	mplane->pixelformat);
		s_vpr_h(inst->sid,
			"%s: input: codec %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, f->fmt.pix.pixelformat, f->fmt.pix.width,
			f->fmt.pix.height, fmt->fmt.pix.sizeimage,
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
		fmt->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
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
	} else if (f->type == OUTPUT_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_PORT];
		fmt->type = OUTPUT_PLANE;
		fmt->fmt.pix.pixelformat = f->fmt.pix.pixelformat;
		fmt->fmt.pix.width = VENUS_Y_STRIDE(
			get_media_colorformat_from_v4l2(fmt->fmt.pix.pixelformat),
			f->fmt.pix.width);
		fmt->fmt.pix.height = VENUS_Y_SCANLINES(
			get_media_colorformat_from_v4l2(fmt->fmt.pix.pixelformat),
			f->fmt.pix.height);
		fmt->fmt.pix.bytesperline = fmt->fmt.pix.width;
		fmt->fmt.pix.sizeimage = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_OUTPUT);
		inst->buffers.output.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_BUF_OUTPUT);
		inst->buffers.output.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_BUF_OUTPUT);
		if (inst->buffers.output.actual_count <
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count) {
			inst->buffers.output.actual_count =
				inst->buffers.output.min_count +
				inst->buffers.output.extra_count;
		}
		inst->buffers.output.size = fmt->fmt.pix.sizeimage;

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		s_vpr_h(inst->sid,
			"%s: output: format %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.pix.pixelformat, fmt->fmt.pix.width,
			fmt->fmt.pix.height, fmt->fmt.pix.sizeimage,
			inst->buffers.output.min_count,
			inst->buffers.output.extra_count);
	} else if (f->type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_META_PORT];
		fmt->type = OUTPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
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
			"%s: input meta: size %d min_count %d extra_count %d\n",
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

	port = msm_vidc_get_port_from_v4l2_type(f->type);
	if (port < 0) {
		d_vpr_e("%s: invalid format type %d\n", __func__, f->type);
		return -EINVAL;
	}
	memcpy(f, &inst->fmts[port], sizeof(struct v4l2_format));

	return rc;
}

int msm_vdec_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	enum msm_vidc_codec_type codec;
	enum msm_vidc_colorformat_type colorformat;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->capabilities || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->index >=
		sizeof(inst->capabilities->cap[PIX_FMTS].step_or_mask) * 8) {
		d_vpr_e("%s: invalid index %d\n", __func__, f->index);
		return -EINVAL;
	}
	memset(f->reserved, 0, sizeof(f->reserved));

	if (f->type == INPUT_PLANE) {
		codec = core->capabilities[DEC_CODECS].value & f->index;
		f->pixelformat = get_v4l2_codec_from_vidc(codec);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == OUTPUT_PLANE) {
		colorformat = f->index &
			inst->capabilities->cap[PIX_FMTS].step_or_mask;
		f->pixelformat = get_v4l2_colorformat_from_vidc(colorformat);
		if (!f->pixelformat)
			return -EINVAL;
		strlcpy(f->description, "colorformat", sizeof(f->description));
	} else if (f->type == INPUT_META_PLANE || f->type == OUTPUT_META_PLANE) {
		if (!f->index) {
			f->pixelformat = V4L2_PIX_FMT_VIDC_META;
			strlcpy(f->description, "metadata", sizeof(f->description));
		} else {
			return -EINVAL;
		}
	}

	return rc;
}

int msm_vdec_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i;
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
	f->type = INPUT_PLANE;
	f->fmt.pix.width = DEFAULT_WIDTH;
	f->fmt.pix.height = DEFAULT_HEIGHT;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix.bytesperline = 0;
	f->fmt.pix.sizeimage = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	inst->buffers.input.size = f->fmt.pix.sizeimage;

	inst->crop.x = inst->crop.y = 0;
	inst->crop.width = f->fmt.pix.width;
	inst->crop.height = f->fmt.pix.height;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_INPUT_META);
	inst->buffers.input_meta.min_count = inst->buffers.input.min_count;
	inst->buffers.input_meta.extra_count = inst->buffers.input.extra_count;
	inst->buffers.input_meta.actual_count = inst->buffers.input.actual_count;
	inst->buffers.input_meta.size = f->fmt.meta.buffersize;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_PLANE;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_NV12_UBWC;
	f->fmt.pix.width = VENUS_Y_STRIDE(
		get_media_colorformat_from_v4l2(f->fmt.pix.pixelformat), DEFAULT_WIDTH);
	f->fmt.pix.height = VENUS_Y_SCANLINES(
		get_media_colorformat_from_v4l2(f->fmt.pix.pixelformat), DEFAULT_HEIGHT);
	f->fmt.pix.bytesperline = f->fmt.pix.width;
	f->fmt.pix.sizeimage = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix.sizeimage;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_OUTPUT_META);
	inst->buffers.output_meta.min_count = inst->buffers.output.min_count;
	inst->buffers.output_meta.extra_count = inst->buffers.output.extra_count;
	inst->buffers.output_meta.actual_count = inst->buffers.output.actual_count;
	inst->buffers.output_meta.size = f->fmt.meta.buffersize;

	inst->prop.frame_rate = DEFAULT_FPS << 16;
	inst->prop.operating_rate = DEFAULT_FPS << 16;

	inst->capabilities = NULL;
	for (i = 0; i < core->codecs_count; i++) {
		if (core->inst_caps[i].domain == MSM_VIDC_DECODER &&
		    core->inst_caps[i].codec == get_vidc_codec_from_v4l2(
				inst->fmts[INPUT_PORT].fmt.pix.pixelformat)) {
			s_vpr_h(inst->sid, "%s: assigned capabilities with %#x caps\n",
				__func__, inst->fmts[INPUT_PORT].fmt.pix.pixelformat);
			inst->capabilities = &core->inst_caps[i];
		}
	}
	if (!inst->capabilities) {
		s_vpr_e(inst->sid, "%s: capabilities not found\n", __func__);
		return -EINVAL;
	}

	return rc;
}

int msm_vdec_ctrl_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	return rc;
}

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/msm_vidc_utils.h>
#include <media/msm_media_info.h>

#include "msm_venc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_debug.h"


int msm_venc_inst_init(struct msm_vidc_inst *inst)
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

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_PLANE;
	f->fmt.pix.width = DEFAULT_WIDTH;
	f->fmt.pix.height = DEFAULT_HEIGHT;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix.bytesperline = 0;
	f->fmt.pix.sizeimage = call_session_op(core, buffer_size,
			inst, MSM_VIDC_OUTPUT);
	inst->buffers.output.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_OUTPUT);
	inst->buffers.output.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix.sizeimage;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_OUTPUT_META);
	inst->buffers.output_meta.min_count = inst->buffers.output.min_count;
	inst->buffers.output_meta.extra_count = inst->buffers.output.extra_count;
	inst->buffers.output_meta.actual_count = inst->buffers.output.actual_count;
	inst->buffers.output_meta.size = f->fmt.meta.buffersize;			

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_PLANE;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_NV12_UBWC;
	f->fmt.pix.width = VENUS_Y_STRIDE(
		msm_vidc_convert_color_fmt(f->fmt.pix.pixelformat), DEFAULT_WIDTH);
	f->fmt.pix.height = VENUS_Y_SCANLINES(
		msm_vidc_convert_color_fmt(f->fmt.pix.pixelformat), DEFAULT_HEIGHT);
	f->fmt.pix.bytesperline = f->fmt.pix.width;
	f->fmt.pix.sizeimage = call_session_op(core, buffer_size,
			inst, MSM_VIDC_INPUT);
	inst->buffers.input.min_count =
			call_session_op(core, min_count, inst, MSM_VIDC_INPUT);
	inst->buffers.input.extra_count =
			call_session_op(core, extra_count, inst, MSM_VIDC_INPUT);
	inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	inst->buffers.input.size = f->fmt.pix.sizeimage;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_INPUT_META);
	inst->buffers.input_meta.min_count = inst->buffers.input.min_count;
	inst->buffers.input_meta.extra_count = inst->buffers.input.extra_count;
	inst->buffers.input_meta.actual_count = inst->buffers.input.actual_count;
	inst->buffers.input_meta.size = f->fmt.meta.buffersize;

	inst->prop.frame_rate = DEFAULT_FPS << 16;
	inst->prop.operating_rate = DEFAULT_FPS << 16;

	return rc;
}

int msm_venc_ctrl_init(struct msm_vidc_inst *inst)
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

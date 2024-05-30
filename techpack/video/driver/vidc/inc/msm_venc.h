/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VENC_H_
#define _MSM_VENC_H_

#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"

int msm_venc_streamoff_input(struct msm_vidc_inst *inst);
int msm_venc_streamon_input(struct msm_vidc_inst *inst);
int msm_venc_streamoff_output(struct msm_vidc_inst *inst);
int msm_venc_streamon_output(struct msm_vidc_inst *inst);
int msm_venc_qbuf(struct msm_vidc_inst *inst, struct vb2_buffer *vb2);
int msm_venc_process_cmd(struct msm_vidc_inst *inst, u32 cmd);
int msm_venc_try_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f);
int msm_venc_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f);
int msm_venc_s_fmt_output(struct msm_vidc_inst *inst, struct v4l2_format *f);
int msm_venc_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f);
int msm_venc_s_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s);
int msm_venc_g_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s);
int msm_venc_s_param(struct msm_vidc_inst *inst,
        struct v4l2_streamparm *s_parm);
int msm_venc_g_param(struct msm_vidc_inst *inst,
		struct v4l2_streamparm *s_parm);
int msm_venc_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f);
int msm_venc_inst_init(struct msm_vidc_inst *inst);
int msm_venc_inst_deinit(struct msm_vidc_inst *inst);

#endif // _MSM_VENC_H_

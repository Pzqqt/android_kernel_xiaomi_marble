/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __H_MSM_VIDC_BUFFER_AR50LT_H__
#define __H_MSM_VIDC_BUFFER_AR50LT_H__

#include "msm_vidc_inst.h"

int msm_buffer_size_ar50lt(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_buffer_min_count_ar50lt(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_buffer_extra_count_ar50lt(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
#endif // __H_MSM_VIDC_BUFFER_AR50LT_H__

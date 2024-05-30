/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __H_MSM_VIDC_FENCE_H__
#define __H_MSM_VIDC_FENCE_H__

#include "msm_vidc_inst.h"
#include "msm_vidc_buffer.h"

struct msm_vidc_fence *msm_vidc_fence_create(
		struct msm_vidc_inst *inst);
int msm_vidc_create_fence_fd(struct msm_vidc_inst *inst,
		struct msm_vidc_fence *fence);
struct msm_vidc_fence *msm_vidc_get_fence_from_id(
	struct msm_vidc_inst *inst, u32 fence_id);
int msm_vidc_fence_signal(struct msm_vidc_inst *inst,
		u32 fence_id);
void msm_vidc_fence_destroy(struct msm_vidc_inst *inst,
		u32 fence_id);
int msm_vidc_fence_init(struct msm_vidc_inst *inst);
void msm_vidc_fence_deinit(struct msm_vidc_inst *inst);

#endif // __H_MSM_VIDC_FENCE_H__

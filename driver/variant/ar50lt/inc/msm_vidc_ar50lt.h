/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _MSM_VIDC_AR50LT_H_
#define _MSM_VIDC_AR50LT_H_

#include "msm_vidc_core.h"

#if defined(CONFIG_MSM_VIDC_AR50LT)
int msm_vidc_init_ar50lt(struct msm_vidc_core *core);
int msm_vidc_deinit_ar50lt(struct msm_vidc_core *core);
#else
static inline int msm_vidc_init_ar50lt(struct msm_vidc_core *core)
{
	return -EINVAL;
}
static inline int msm_vidc_deinit_ar50lt(struct msm_vidc_core *core)
{
	return -EINVAL;
}
#endif
#endif // _MSM_VIDC_AR50LT_H_

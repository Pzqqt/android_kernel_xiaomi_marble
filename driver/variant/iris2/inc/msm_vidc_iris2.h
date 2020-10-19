/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_core.h"

//#if defined(CONFIG_MSM_VIDC_IRIS2)
int msm_vidc_init_iris2(struct msm_vidc_core *core);
//#else
//static inline int msm_vidc_init_iris2(struct msm_vidc_core *core)
//{
//	return -EINVAL;
//}
//#endif
int msm_vidc_deinit_iris2(struct msm_vidc_core *core);

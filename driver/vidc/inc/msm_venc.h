/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VENC_H_
#define _MSM_VENC_H_

#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"

int msm_venc_inst_init(struct msm_vidc_inst *inst);
int msm_venc_ctrl_init(struct msm_vidc_inst *inst);

#endif // _MSM_VENC_H_
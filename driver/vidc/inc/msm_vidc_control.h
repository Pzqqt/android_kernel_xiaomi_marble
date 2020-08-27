/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_CONTROL_H_
#define _MSM_VIDC_CONTROL_H_

#include <media/msm_vidc_utils.h>
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"

enum msm_vidc_ctrl_list_type {
	CHILD_LIST          = BIT(0),
	FW_LIST             = BIT(1),
};

int msm_vidc_ctrl_init(struct msm_vidc_inst *inst);
int msm_v4l2_op_s_ctrl(struct v4l2_ctrl *ctrl);

#endif

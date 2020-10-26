/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_CONTROL_H_
#define _MSM_VIDC_CONTROL_H_

#include <media/v4l2_vidc_extensions.h>
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"

#define MAX_GOP                 0xFFFFFFF
#define MAX_INTRA_REFRESH_MBS   ((7680 * 4320) >> 8)
#define MAX_BASE_LAYER_PRIORITY_ID 63
#define MIN_CHROMA_QP_OFFSET    -12
#define MAX_CHROMA_QP_OFFSET    12
#define MAX_LTR_FRAME_COUNT     10
#define MAX_BITRATE             220000000

enum msm_vidc_ctrl_list_type {
	CHILD_LIST          = BIT(0),
	FW_LIST             = BIT(1),
};

int msm_vidc_ctrl_init(struct msm_vidc_inst *inst);
int msm_vidc_ctrl_deinit(struct msm_vidc_inst *inst);
int msm_v4l2_op_s_ctrl(struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_bitrate(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_entropy_mode(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_ltr_count(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_properties(struct msm_vidc_inst *inst);
int msm_vidc_set_u32(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_s32(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_array(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_fw_list(struct msm_vidc_inst *inst);
int msm_vidc_v4l2_menu_to_hfi(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 *value);

#endif

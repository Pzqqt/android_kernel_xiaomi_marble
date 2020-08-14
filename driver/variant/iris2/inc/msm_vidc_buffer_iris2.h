/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __H_MSM_VIDC_BUFFER_IRIS2_H__
#define __H_MSM_VIDC_BUFFER_IRIS2_H__

#include "msm_vidc_inst.h"

u32 msm_vidc_decoder_scratch_size_iris2(struct msm_vidc_inst *inst);
u32 msm_vidc_decoder_scratch_1_size_iris2(struct msm_vidc_inst *inst);
u32 msm_vidc_decoder_persist_1_size_iris2(struct msm_vidc_inst *inst);

u32 msm_vidc_encoder_scratch_size_iris2(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_scratch_1_size_iris2(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_scratch_2_size_iris2(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_persist_size_iris2(struct msm_vidc_inst *inst);

#endif // __H_MSM_VIDC_BUFFER_IRIS2_H__

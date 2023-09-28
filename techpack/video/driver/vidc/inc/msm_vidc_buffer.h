/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef __H_MSM_VIDC_BUFFER_H__
#define __H_MSM_VIDC_BUFFER_H__

#include "msm_vidc_inst.h"

#define MIN_DEC_INPUT_BUFFERS 4
#define MIN_DEC_OUTPUT_BUFFERS 4

#define MIN_ENC_INPUT_BUFFERS 4
#define MIN_ENC_OUTPUT_BUFFERS 4

#define DCVS_ENC_EXTRA_INPUT_BUFFERS 4
#define DCVS_DEC_EXTRA_OUTPUT_BUFFERS 4

u32 msm_vidc_input_min_count(struct msm_vidc_inst *inst);
u32 msm_vidc_output_min_count(struct msm_vidc_inst *inst);
u32 msm_vidc_input_extra_count(struct msm_vidc_inst *inst);
u32 msm_vidc_output_extra_count(struct msm_vidc_inst *inst);
u32 msm_vidc_internal_buffer_count(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
u32 msm_vidc_decoder_input_size(struct msm_vidc_inst *inst);
u32 msm_vidc_decoder_output_size(struct msm_vidc_inst *inst);
u32 msm_vidc_decoder_input_meta_size(struct msm_vidc_inst *inst);
u32 msm_vidc_decoder_output_meta_size(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_input_size(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_output_size(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_input_meta_size(struct msm_vidc_inst *inst);
u32 msm_vidc_encoder_output_meta_size(struct msm_vidc_inst *inst);

#endif // __H_MSM_VIDC_BUFFER_H__

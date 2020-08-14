/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_DRIVER_H_
#define _MSM_VIDC_DRIVER_H_

#include <linux/workqueue.h>
#include <linux/iommu.h>
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"

static inline is_decode_session(struct msm_vidc_inst *inst)
{
	return inst->domain == MSM_VIDC_DECODER;
}

static inline is_encode_session(struct msm_vidc_inst *inst)
{
	return inst->domain == MSM_VIDC_ENCODER;
}

static inline is_secure_session(struct msm_vidc_inst *inst)
{
	return false;
}

static inline is_input_buffer(enum msm_vidc_buffer_type buffer_type)
{
	return buffer_type == MSM_VIDC_BUF_INPUT;
}

static inline is_output_buffer(enum msm_vidc_buffer_type buffer_type)
{
	return buffer_type == MSM_VIDC_BUF_OUTPUT;
}

static inline is_input_meta_buffer(enum msm_vidc_buffer_type buffer_type)
{
	return buffer_type == MSM_VIDC_BUF_INPUT_META;
}

static inline is_output_meta_buffer(enum msm_vidc_buffer_type buffer_type)
{
	return buffer_type == MSM_VIDC_BUF_OUTPUT_META;
}

static inline is_internal_buffer(enum msm_vidc_buffer_type buffer_type)
{
	return buffer_type == MSM_VIDC_BUF_SCRATCH ||
		buffer_type == MSM_VIDC_BUF_SCRATCH_1 ||
		buffer_type == MSM_VIDC_BUF_SCRATCH_2 ||
		buffer_type == MSM_VIDC_BUF_PERSIST ||
		buffer_type == MSM_VIDC_BUF_PERSIST_1;
}

static inline bool is_secondary_output_mode(struct msm_vidc_inst *inst)
{
	return false; // TODO: inst->stream_output_mode == HAL_VIDEO_DECODER_SECONDARY;
}

static inline bool is_thumbnail_session(struct msm_vidc_inst *inst)
{
	return false; // TODO: fix it
}

int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
		enum msm_vidc_inst_state state);
int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_add_session(struct msm_vidc_inst *inst);
int msm_vidc_session_open(struct msm_vidc_inst *inst);
int msm_vidc_core_init(struct msm_vidc_core *core);
int msm_vidc_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data);
int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
		enum msm_vidc_ssr_trigger_type type);
void msm_vidc_ssr_handler(struct work_struct *work);
void msm_vidc_pm_work_handler(struct work_struct *work);
void msm_vidc_fw_unload_handler(struct work_struct *work);
void msm_vidc_batch_handler(struct work_struct *work);
int msm_vidc_setup_event_queue(struct msm_vidc_inst *inst);
int msm_vidc_vb2_queue_init(struct msm_vidc_inst *inst);
u32 msm_vidc_convert_color_fmt(u32 v4l2_fmt);
int msm_vidc_get_control(struct msm_vidc_inst *inst, struct v4l2_ctrl *ctrl);
int msm_vidc_get_port_from_v4l2_type(u32 type);
u32 msm_vidc_get_buffer_region(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
struct msm_vidc_buffer_info *msm_vidc_get_buffer_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
struct msm_vidc_map_info *msm_vidc_get_map_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
struct msm_vidc_alloc_info *msm_vidc_get_alloc_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
#endif // _MSM_VIDC_DRIVER_H_


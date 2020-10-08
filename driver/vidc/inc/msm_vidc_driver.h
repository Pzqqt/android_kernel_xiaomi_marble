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
	return buffer_type == MSM_VIDC_BUF_BIN ||
		buffer_type == MSM_VIDC_BUF_COMV ||
		buffer_type == MSM_VIDC_BUF_NON_COMV ||
		buffer_type == MSM_VIDC_BUF_LINE ||
		buffer_type == MSM_VIDC_BUF_DPB ||
		buffer_type == MSM_VIDC_BUF_PERSIST;
}

static inline bool is_secondary_output_mode(struct msm_vidc_inst *inst)
{
	return false; // TODO: inst->stream_output_mode == HAL_VIDEO_DECODER_SECONDARY;
}

static inline bool is_thumbnail_session(struct msm_vidc_inst *inst)
{
	return false; // TODO: fix it
}

void print_vidc_buffer(u32 tag, const char *str, struct msm_vidc_inst *inst,
		struct msm_vidc_buffer *vbuf);
void print_vb2_buffer(const char *str, struct msm_vidc_inst *inst,
		struct vb2_buffer *vb2);
enum msm_vidc_codec_type v4l2_codec_to_driver(u32 v4l2_codec,
	const char *func);
u32 v4l2_codec_from_driver(enum msm_vidc_codec_type codec,
	const char *func);
u32 v4l2_colorformat_to_media(u32 v4l2_fmt,
	const char *func);
enum msm_vidc_colorformat_type v4l2_colorformat_to_driver(u32 colorformat,
	const char *func);
u32 v4l2_colorformat_from_driver(enum msm_vidc_colorformat_type colorformat,
	const char *func);
int v4l2_type_to_driver_port(struct msm_vidc_inst *inst, u32 type,
	const char *func);
int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_state state, const char *func);
int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_vb2_buffer_done(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
int msm_vidc_remove_session(struct msm_vidc_inst *inst);
int msm_vidc_add_session(struct msm_vidc_inst *inst);
int msm_vidc_session_open(struct msm_vidc_inst *inst);
int msm_vidc_session_set_codec(struct msm_vidc_inst *inst);
int msm_vidc_session_stop(struct msm_vidc_inst *inst,
		enum msm_vidc_port_type port);
int msm_vidc_session_close(struct msm_vidc_inst *inst);
int msm_vidc_get_inst_capability(struct msm_vidc_inst *inst);
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
int msm_vidc_get_control(struct msm_vidc_inst *inst, struct v4l2_ctrl *ctrl);
u32 msm_vidc_get_buffer_region(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, const char *func);
struct msm_vidc_buffers *msm_vidc_get_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, const char *func);
struct msm_vidc_mappings *msm_vidc_get_mappings(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, const char *func);
struct msm_vidc_allocations *msm_vidc_get_allocations(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func);
struct msm_vidc_buffer *msm_vidc_get_driver_buf(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2);
int msm_vidc_unmap_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
int msm_vidc_map_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
int msm_vidc_put_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
int msm_vidc_queue_buffer(struct msm_vidc_inst *inst, struct vb2_buffer *vb2);
struct msm_vidc_buffer *get_meta_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *vbuf);
struct msm_vidc_inst *get_inst(struct msm_vidc_core *core,
		u32 session_id);
void put_inst(struct msm_vidc_inst *inst);
void core_lock(struct msm_vidc_core *core, const char *function);
void core_unlock(struct msm_vidc_core *core, const char *function);
void inst_lock(struct msm_vidc_inst *inst, const char *function);
void inst_unlock(struct msm_vidc_inst *inst, const char *function);
#endif // _MSM_VIDC_DRIVER_H_


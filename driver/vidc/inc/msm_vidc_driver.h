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

#define MSM_VIDC_SESSION_INACTIVE_THRESHOLD_MS 1000

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
		buffer_type == MSM_VIDC_BUF_ARP ||
		buffer_type == MSM_VIDC_BUF_COMV ||
		buffer_type == MSM_VIDC_BUF_NON_COMV ||
		buffer_type == MSM_VIDC_BUF_LINE ||
		buffer_type == MSM_VIDC_BUF_DPB ||
		buffer_type == MSM_VIDC_BUF_PERSIST ||
		buffer_type == MSM_VIDC_BUF_VPSS;
}

static inline bool is_input_meta_enabled(struct msm_vidc_inst *inst)
{
	bool enabled = false;

	if (is_decode_session(inst)) {
		enabled = inst->capabilities->cap[META_BUF_TAG].value ?
			true : false;
	} else if (is_encode_session(inst)) {
		enabled = (inst->capabilities->cap[META_LTR_MARK_USE].value ||
			inst->capabilities->cap[META_SEQ_HDR_NAL].value ||
			inst->capabilities->cap[META_EVA_STATS].value ||
			inst->capabilities->cap[META_BUF_TAG].value);
	}
	return enabled;
}

static inline bool is_output_meta_enabled(struct msm_vidc_inst *inst)
{
	bool enabled = false;

	if (is_decode_session(inst)) {
		enabled = (inst->capabilities->cap[META_DPB_MISR].value ||
			inst->capabilities->cap[META_OPB_MISR].value ||
			inst->capabilities->cap[META_INTERLACE].value ||
			inst->capabilities->cap[META_CONCEALED_MB_CNT].value ||
			inst->capabilities->cap[META_HIST_INFO].value ||
			inst->capabilities->cap[META_SEI_MASTERING_DISP].value ||
			inst->capabilities->cap[META_SEI_CLL].value ||
			inst->capabilities->cap[META_BUF_TAG].value ||
			inst->capabilities->cap[META_SUBFRAME_OUTPUT].value);
	} else if (is_encode_session(inst)) {
		enabled = inst->capabilities->cap[META_BUF_TAG].value ?
			true : false;
	}
	return enabled;
}

static inline bool is_meta_enabled(struct msm_vidc_inst *inst, unsigned int type)
{
	bool enabled = false;

	if (type == MSM_VIDC_BUF_INPUT)
		enabled = is_input_meta_enabled(inst);
	else if (type == MSM_VIDC_BUF_OUTPUT)
		enabled = is_output_meta_enabled(inst);

	return enabled;
}

static inline bool is_linear_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_NV12 ||
		colorformat == MSM_VIDC_FMT_NV21 ||
		colorformat == MSM_VIDC_FMT_NV12_P010;
}

static inline bool is_10bit_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_NV12_P010 ||
		colorformat == MSM_VIDC_FMT_NV12_TP10_UBWC;
}

static inline bool is_secondary_output_mode(struct msm_vidc_inst *inst)
{
	return false; // TODO: inst->stream_output_mode == HAL_VIDEO_DECODER_SECONDARY;
}

static inline bool is_turbo_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_TURBO);
}

static inline bool is_thumbnail_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_THUMBNAIL);
}

static inline bool is_low_power_session(struct msm_vidc_inst *inst)
{
	return !!(inst->flags & VIDC_LOW_POWER);
}

static inline bool is_realtime_session(struct msm_vidc_inst *inst)
{
	return false; // TODO: fix it
}

static inline bool is_active_session(u64 prev, u64 curr)
{
	u64 ts_delta;

	if (!prev || !curr)
		return true;

	ts_delta = (prev < curr) ? curr - prev : prev - curr;

	return ((ts_delta / NSEC_PER_MSEC) <=
			MSM_VIDC_SESSION_INACTIVE_THRESHOLD_MS);
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
int msm_vidc_get_input_internal_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
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
int msm_vidc_session_start(struct msm_vidc_inst* inst,
		enum msm_vidc_port_type port);
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
int msm_vidc_destroy_internal_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer);
struct msm_vidc_buffer *get_meta_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *vbuf);
struct msm_vidc_inst *get_inst(struct msm_vidc_core *core,
		u32 session_id);
void put_inst(struct msm_vidc_inst *inst);
int msm_vidc_get_mbs_per_frame(struct msm_vidc_inst* inst);
int msm_vidc_get_fps(struct msm_vidc_inst* inst);
int msm_vidc_num_queued_bufs(struct msm_vidc_inst* inst, u32 type);
void core_lock(struct msm_vidc_core *core, const char *function);
void core_unlock(struct msm_vidc_core *core, const char *function);
void inst_lock(struct msm_vidc_inst *inst, const char *function);
void inst_unlock(struct msm_vidc_inst *inst, const char *function);
#endif // _MSM_VIDC_DRIVER_H_


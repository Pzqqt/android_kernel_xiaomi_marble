/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_DRIVER_H_
#define _MSM_VIDC_DRIVER_H_

#include <linux/workqueue.h>
#include <linux/iommu.h>
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_platform.h"

#define MSM_VIDC_SESSION_INACTIVE_THRESHOLD_MS 1000
#define HEIC_GRID_DIMENSION 512

enum msm_vidc_debugfs_event;

static inline is_decode_session(struct msm_vidc_inst *inst)
{
	return inst->domain == MSM_VIDC_DECODER;
}

static inline is_encode_session(struct msm_vidc_inst *inst)
{
	return inst->domain == MSM_VIDC_ENCODER;
}

static inline is_image_encode_session(struct msm_vidc_inst *inst)
{
	return inst->codec == MSM_VIDC_HEIC && inst->domain == MSM_VIDC_ENCODER;
}

static inline is_image_decode_session(struct msm_vidc_inst *inst)
{
	return inst->codec == MSM_VIDC_HEIC && inst->domain == MSM_VIDC_DECODER;
}

static inline is_image_session(struct msm_vidc_inst *inst)
{
	return inst->codec == MSM_VIDC_HEIC;
}

static inline is_secure_session(struct msm_vidc_inst *inst)
{
	return !!(inst->capabilities->cap[SECURE_MODE].value);
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

static inline is_ts_reorder_allowed(struct msm_vidc_inst *inst)
{
	return !!(inst->capabilities->cap[TS_REORDER].value &&
		is_decode_session(inst) && !is_image_session(inst));
}

static inline is_scaling_enabled(struct msm_vidc_inst *inst)
{
	return inst->crop.left != inst->compose.left ||
		inst->crop.top != inst->compose.top ||
		inst->crop.width != inst->compose.width ||
		inst->crop.height != inst->compose.height;
}

static inline is_rotation_90_or_270(struct msm_vidc_inst *inst)
{
	return inst->capabilities->cap[ROTATION].value == 90 ||
		inst->capabilities->cap[ROTATION].value == 270;
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
		enabled = (inst->capabilities->cap[META_SEQ_HDR_NAL].value ||
			inst->capabilities->cap[META_EVA_STATS].value ||
			inst->capabilities->cap[META_BUF_TAG].value ||
			inst->capabilities->cap[META_ROI_INFO].value);
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
			inst->capabilities->cap[META_DPB_TAG_LIST].value ||
			inst->capabilities->cap[META_SUBFRAME_OUTPUT].value ||
			inst->capabilities->cap[META_MAX_NUM_REORDER_FRAMES].value);
	} else if (is_encode_session(inst)) {
		enabled = (inst->capabilities->cap[META_LTR_MARK_USE].value ||
			inst->capabilities->cap[META_BUF_TAG].value);
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

static inline bool is_outbuf_fence_enabled(struct msm_vidc_inst *inst)
{
	return !!(inst->capabilities->cap[META_OUTBUF_FENCE].value);
}

static inline bool is_linear_yuv_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_NV12 ||
		colorformat == MSM_VIDC_FMT_NV21 ||
		colorformat == MSM_VIDC_FMT_P010;
}

static inline bool is_linear_rgba_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_RGBA8888;
}

static inline bool is_linear_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return is_linear_yuv_colorformat(colorformat) || is_linear_rgba_colorformat(colorformat);
}

static inline bool is_ubwc_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_NV12C ||
		colorformat == MSM_VIDC_FMT_TP10C ||
		colorformat == MSM_VIDC_FMT_RGBA8888C;
}

static inline bool is_10bit_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_P010 ||
		colorformat == MSM_VIDC_FMT_TP10C;
}

static inline bool is_8bit_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_NV12 ||
		colorformat == MSM_VIDC_FMT_NV12C ||
		colorformat == MSM_VIDC_FMT_NV21 ||
		colorformat == MSM_VIDC_FMT_RGBA8888 ||
		colorformat == MSM_VIDC_FMT_RGBA8888C;
}

static inline bool is_rgba_colorformat(enum msm_vidc_colorformat_type colorformat)
{
	return colorformat == MSM_VIDC_FMT_RGBA8888 ||
		colorformat == MSM_VIDC_FMT_RGBA8888C;
}

static inline bool is_thumbnail_session(struct msm_vidc_inst *inst)
{
	return !!(inst->capabilities->cap[THUMBNAIL_MODE].value);
}

static inline bool is_low_power_session(struct msm_vidc_inst *inst)
{
	return (inst->capabilities->cap[QUALITY_MODE].value ==
		MSM_VIDC_POWER_SAVE_MODE);
}

static inline bool is_realtime_session(struct msm_vidc_inst *inst)
{
	return inst->capabilities->cap[PRIORITY].value == 0 ? true : false;
}

static inline bool is_lowlatency_session(struct msm_vidc_inst *inst)
{
	return !!(inst->capabilities->cap[LOWLATENCY_MODE].value);
}

static inline bool is_hierb_type_requested(struct msm_vidc_inst *inst)
{
	return (inst->codec == MSM_VIDC_H264 &&
			inst->capabilities->cap[LAYER_TYPE].value ==
				V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_B) ||
			(inst->codec == MSM_VIDC_HEVC &&
			inst->capabilities->cap[LAYER_TYPE].value ==
				V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B);
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

static inline bool is_session_error(struct msm_vidc_inst *inst)
{
	return inst->state == MSM_VIDC_ERROR;
}

static inline bool is_secure_region(enum msm_vidc_buffer_region region)
{
	return !(region == MSM_VIDC_NON_SECURE ||
			region == MSM_VIDC_NON_SECURE_PIXEL);
}

const char *cap_name(enum msm_vidc_inst_capability_type cap);
const char *v4l2_pixelfmt_name(u32 pixelfmt);
const char *v4l2_type_name(u32 port);
void print_vidc_buffer(u32 tag, const char *tag_str, const char *str, struct msm_vidc_inst *inst,
		struct msm_vidc_buffer *vbuf);
void print_vb2_buffer(const char *str, struct msm_vidc_inst *inst,
		struct vb2_buffer *vb2);
enum msm_vidc_codec_type v4l2_codec_to_driver(u32 v4l2_codec,
	const char *func);
u32 v4l2_codec_from_driver(enum msm_vidc_codec_type codec,
	const char *func);
enum msm_vidc_colorformat_type v4l2_colorformat_to_driver(u32 colorformat,
	const char *func);
u32 v4l2_colorformat_from_driver(enum msm_vidc_colorformat_type colorformat,
	const char *func);
u32 v4l2_color_primaries_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_primaries, const char *func);
u32 v4l2_color_primaries_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_color_primaries, const char *func);
u32 v4l2_transfer_char_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_transfer_char, const char *func);
u32 v4l2_transfer_char_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_transfer_char, const char *func);
u32 v4l2_matrix_coeff_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_matrix_coeff, const char *func);
u32 v4l2_matrix_coeff_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_matrix_coeff, const char *func);
int v4l2_type_to_driver_port(struct msm_vidc_inst *inst, u32 type,
	const char *func);
const char *allow_name(enum msm_vidc_allow allow);
const char *state_name(enum msm_vidc_inst_state state);
int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_state request_state, const char *func);
int msm_vidc_create_internal_buffer(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, u32 index);
int msm_vidc_get_internal_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type);
int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_alloc_and_queue_session_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type);
int msm_vidc_vb2_buffer_done(struct msm_vidc_inst *inst,
		struct msm_vidc_buffer *buf);
int msm_vidc_remove_session(struct msm_vidc_inst *inst);
int msm_vidc_add_session(struct msm_vidc_inst *inst);
int msm_vidc_session_open(struct msm_vidc_inst *inst);
int msm_vidc_session_set_codec(struct msm_vidc_inst *inst);
int msm_vidc_session_set_secure_mode(struct msm_vidc_inst *inst);
int msm_vidc_session_set_default_header(struct msm_vidc_inst *inst);
int msm_vidc_session_streamon(struct msm_vidc_inst *inst,
		enum msm_vidc_port_type port);
int msm_vidc_session_streamoff(struct msm_vidc_inst *inst,
		enum msm_vidc_port_type port);
int msm_vidc_session_close(struct msm_vidc_inst *inst);
int msm_vidc_kill_session(struct msm_vidc_inst* inst);
int msm_vidc_get_inst_capability(struct msm_vidc_inst *inst);
int msm_vidc_change_core_state(struct msm_vidc_core *core,
	enum msm_vidc_core_state request_state, const char *func);
int msm_vidc_core_init(struct msm_vidc_core *core);
int msm_vidc_core_deinit(struct msm_vidc_core *core, bool force);
int msm_vidc_inst_timeout(struct msm_vidc_inst *inst);
int msm_vidc_print_buffer_info(struct msm_vidc_inst *inst);
int msm_vidc_print_inst_info(struct msm_vidc_inst *inst);
void msm_vidc_print_core_info(struct msm_vidc_core *core);
int msm_vidc_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data);
int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
		u64 trigger_ssr_val);
void msm_vidc_ssr_handler(struct work_struct *work);
int msm_vidc_trigger_stability(struct msm_vidc_core *core,
		u64 trigger_stability_val);
void msm_vidc_stability_handler(struct work_struct *work);
int cancel_stability_work_sync(struct msm_vidc_inst *inst);
void msm_vidc_fw_unload_handler(struct work_struct *work);
int msm_vidc_suspend(struct msm_vidc_core *core);
void msm_vidc_batch_handler(struct work_struct *work);
int msm_vidc_event_queue_init(struct msm_vidc_inst *inst);
int msm_vidc_event_queue_deinit(struct msm_vidc_inst *inst);
int msm_vidc_vb2_queue_init(struct msm_vidc_inst *inst);
int msm_vidc_vb2_queue_deinit(struct msm_vidc_inst *inst);
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
int msm_vidc_get_delayed_unmap(struct msm_vidc_inst *inst,
	struct msm_vidc_map *map);
int msm_vidc_put_delayed_unmap(struct msm_vidc_inst *inst,
	struct msm_vidc_map *map);
void msm_vidc_update_stats(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf, enum msm_vidc_debugfs_event etype);
void msm_vidc_stats_handler(struct work_struct *work);
int schedule_stats_work(struct msm_vidc_inst *inst);
int cancel_stats_work_sync(struct msm_vidc_inst *inst);
void msm_vidc_print_stats(struct msm_vidc_inst *inst);
enum msm_vidc_buffer_type v4l2_type_to_driver(u32 type,
	const char *func);
int msm_vidc_queue_buffer_single(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2);
int msm_vidc_queue_deferred_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buf_type);
int msm_vidc_destroy_internal_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer);
void msm_vidc_destroy_buffers(struct msm_vidc_inst *inst);
int msm_vidc_flush_buffers(struct msm_vidc_inst* inst,
	enum msm_vidc_buffer_type type);
int msm_vidc_flush_delayed_unmap_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type type);
struct msm_vidc_buffer *get_meta_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *vbuf);
struct msm_vidc_inst *get_inst_ref(struct msm_vidc_core *core,
		struct msm_vidc_inst *instance);
struct msm_vidc_inst *get_inst(struct msm_vidc_core *core,
		u32 session_id);
void put_inst(struct msm_vidc_inst *inst);
bool msm_vidc_allow_s_fmt(struct msm_vidc_inst *inst, u32 type);
bool msm_vidc_allow_s_ctrl(struct msm_vidc_inst *inst, u32 id);
bool msm_vidc_allow_metadata(struct msm_vidc_inst *inst, u32 cap_id);
bool msm_vidc_allow_property(struct msm_vidc_inst *inst, u32 hfi_id);
int msm_vidc_update_property_cap(struct msm_vidc_inst *inst, u32 hfi_id,
	bool allow);
bool msm_vidc_allow_reqbufs(struct msm_vidc_inst *inst, u32 type);
enum msm_vidc_allow msm_vidc_allow_stop(struct msm_vidc_inst *inst);
bool msm_vidc_allow_start(struct msm_vidc_inst *inst);
bool msm_vidc_allow_streamon(struct msm_vidc_inst *inst, u32 type);
enum msm_vidc_allow msm_vidc_allow_streamoff(struct msm_vidc_inst *inst, u32 type);
enum msm_vidc_allow msm_vidc_allow_qbuf(struct msm_vidc_inst *inst, u32 type);
enum msm_vidc_allow msm_vidc_allow_input_psc(struct msm_vidc_inst *inst);
bool msm_vidc_allow_last_flag(struct msm_vidc_inst *inst);
int msm_vidc_state_change_streamon(struct msm_vidc_inst *inst, u32 type);
int msm_vidc_state_change_streamoff(struct msm_vidc_inst *inst, u32 type);
int msm_vidc_state_change_stop(struct msm_vidc_inst *inst);
int msm_vidc_state_change_start(struct msm_vidc_inst *inst);
int msm_vidc_state_change_input_psc(struct msm_vidc_inst *inst);
int msm_vidc_state_change_last_flag(struct msm_vidc_inst *inst);
int msm_vidc_get_mbs_per_frame(struct msm_vidc_inst *inst);
int msm_vidc_get_fps(struct msm_vidc_inst *inst);
int msm_vidc_num_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type type, enum msm_vidc_buffer_attributes attr);
void core_lock(struct msm_vidc_core *core, const char *function);
void core_unlock(struct msm_vidc_core *core, const char *function);
bool core_lock_check(struct msm_vidc_core *core, const char *function);
void inst_lock(struct msm_vidc_inst *inst, const char *function);
void inst_unlock(struct msm_vidc_inst *inst, const char *function);
bool inst_lock_check(struct msm_vidc_inst *inst, const char *function);
int msm_vidc_update_bitstream_buffer_size(struct msm_vidc_inst *inst);
int msm_vidc_update_meta_port_settings(struct msm_vidc_inst *inst);
int msm_vidc_update_buffer_count(struct msm_vidc_inst *inst, u32 port);
int msm_vidc_schedule_core_deinit(struct msm_vidc_core *core, bool force_deinit);
bool msm_vidc_is_super_buffer(struct msm_vidc_inst *inst);
int msm_vidc_init_core_caps(struct msm_vidc_core* core);
int msm_vidc_init_instance_caps(struct msm_vidc_core* core);
int msm_vidc_deinit_core_caps(struct msm_vidc_core* core);
int msm_vidc_deinit_instance_caps(struct msm_vidc_core* core);
int msm_vidc_update_debug_str(struct msm_vidc_inst *inst);
void msm_vidc_allow_dcvs(struct msm_vidc_inst *inst);
bool msm_vidc_allow_decode_batch(struct msm_vidc_inst *inst);
int msm_vidc_check_session_supported(struct msm_vidc_inst *inst);
int msm_vidc_check_core_mbps(struct msm_vidc_inst *inst);
int msm_vidc_check_core_mbpf(struct msm_vidc_inst *inst);
int msm_vidc_check_scaling_supported(struct msm_vidc_inst *inst);
int msm_vidc_update_timestamp(struct msm_vidc_inst *inst, u64 timestamp);
int msm_vidc_set_auto_framerate(struct msm_vidc_inst *inst, u64 timestamp);
int msm_vidc_calc_window_avg_framerate(struct msm_vidc_inst *inst);
int msm_vidc_flush_ts(struct msm_vidc_inst *inst);
int msm_vidc_ts_reorder_insert_timestamp(struct msm_vidc_inst *inst, u64 timestamp);
int msm_vidc_ts_reorder_remove_timestamp(struct msm_vidc_inst *inst, u64 timestamp);
int msm_vidc_ts_reorder_get_first_timestamp(struct msm_vidc_inst *inst, u64 *timestamp);
int msm_vidc_ts_reorder_flush(struct msm_vidc_inst *inst);
const char *buf_name(enum msm_vidc_buffer_type type);
void msm_vidc_free_capabililty_list(struct msm_vidc_inst *inst,
	enum msm_vidc_ctrl_list_type list_type);
bool res_is_greater_than(u32 width, u32 height,
	u32 ref_width, u32 ref_height);
bool res_is_greater_than_or_equal_to(u32 width, u32 height,
	u32 ref_width, u32 ref_height);
bool res_is_less_than(u32 width, u32 height,
	u32 ref_width, u32 ref_height);
bool res_is_less_than_or_equal_to(u32 width, u32 height,
	u32 ref_width, u32 ref_height);
bool is_ubwc_supported_platform(struct msm_vidc_inst *inst);
int msm_vidc_get_properties(struct msm_vidc_inst *inst);
int msm_vidc_get_src_clk_scaling_ratio(struct msm_vidc_core *core);
int msm_vidc_qbuf_cache_operation(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
int msm_vidc_dqbuf_cache_operation(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf);
#endif // _MSM_VIDC_DRIVER_H_


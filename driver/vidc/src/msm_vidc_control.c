// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "hfi_packet.h"
#include "hfi_property.h"
#include "venus_hfi.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#include "msm_venc.h"
#include "msm_vidc_platform.h"

#define CAP_TO_8BIT_QP(a) {          \
	if ((a) < 0)                 \
		(a) = 0;             \
}

static bool is_priv_ctrl(u32 id)
{
	bool private = false;

	if (IS_PRIV_CTRL(id))
		return true;

	/*
	 * Treat below standard controls as private because
	 * we have added custom values to the controls
	 */
	switch (id) {
	/*
	 * TODO: V4L2_CID_MPEG_VIDEO_HEVC_PROFILE is std ctrl. But
	 * V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10_STILL_PICTURE support is not
	 * available yet. Hence, make this as private ctrl for time being
	 */
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
	/*
	 * TODO: V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE is
	 * std ctrl. But needs some fixes in v4l2-ctrls.c. Hence,
	 * make this as private ctrl for time being
	 */
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
	/*
	 * TODO: treat below std ctrls as private ctrls until
	 * all below ctrls are available in upstream
	 */
	case V4L2_CID_MPEG_VIDEO_AU_DELIMITER:
	case V4L2_CID_MPEG_VIDEO_LTR_COUNT:
	case V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX:
	case V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES:
	case V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY:
	case V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE:
		private = true;
		break;
	default:
		private = false;
		break;
	}

	return private;
}

static bool is_meta_ctrl(u32 id)
{
	return (id == V4L2_CID_MPEG_VIDC_METADATA_LTR_MARK_USE_DETAILS ||
		id == V4L2_CID_MPEG_VIDC_METADATA_SEQ_HEADER_NAL ||
		id == V4L2_CID_MPEG_VIDC_METADATA_DPB_LUMA_CHROMA_MISR ||
		id == V4L2_CID_MPEG_VIDC_METADATA_OPB_LUMA_CHROMA_MISR ||
		id == V4L2_CID_MPEG_VIDC_METADATA_INTERLACE ||
		id == V4L2_CID_MPEG_VIDC_METADATA_CONCEALED_MB_COUNT ||
		id == V4L2_CID_MPEG_VIDC_METADATA_HISTOGRAM_INFO ||
		id == V4L2_CID_MPEG_VIDC_METADATA_SEI_MASTERING_DISPLAY_COLOUR ||
		id == V4L2_CID_MPEG_VIDC_METADATA_SEI_CONTENT_LIGHT_LEVEL ||
		id == V4L2_CID_MPEG_VIDC_METADATA_HDR10PLUS ||
		id == V4L2_CID_MPEG_VIDC_METADATA_EVA_STATS ||
		id == V4L2_CID_MPEG_VIDC_METADATA_BUFFER_TAG ||
		id == V4L2_CID_MPEG_VIDC_METADATA_DPB_TAG_LIST ||
		id == V4L2_CID_MPEG_VIDC_METADATA_SUBFRAME_OUTPUT ||
		id == V4L2_CID_MPEG_VIDC_METADATA_ROI_INFO ||
		id == V4L2_CID_MPEG_VIDC_METADATA_TIMESTAMP ||
		id == V4L2_CID_MPEG_VIDC_METADATA_ENC_QP_METADATA);
}

static const char *const mpeg_video_rate_control[] = {
	"VBR",
	"CBR",
	"CBR VFR",
	"MBR",
	"MBR VFR",
	"CQ",
	NULL,
};

static const char *const mpeg_video_stream_format[] = {
	"NAL Format Start Codes",
	"NAL Format One NAL Per Buffer",
	"NAL Format One Byte Length",
	"NAL Format Two Byte Length",
	"NAL Format Four Byte Length",
	NULL,
};

static const char *const mpeg_video_blur_types[] = {
	"Blur None",
	"Blur External",
	"Blur Adaptive",
	NULL,
};

static const char *const mpeg_video_avc_coding_layer[] = {
	"B",
	"P",
	NULL,
};

static const char *const mpeg_video_hevc_profile[] = {
	"Main",
	"Main Still Picture",
	"Main 10",
	"Main 10 Still Picture",
	NULL,
};

static const char *const roi_map_type[] = {
	"None",
	"2-bit",
	"2-bit",
	NULL,
};

static u32 msm_vidc_get_port_info(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id)
{
	struct msm_vidc_inst_capability *capability = inst->capabilities;

	if (capability->cap[cap_id].flags & CAP_FLAG_INPUT_PORT &&
		capability->cap[cap_id].flags & CAP_FLAG_OUTPUT_PORT) {
		if (inst->vb2q[OUTPUT_PORT].streaming)
			return get_hfi_port(inst, INPUT_PORT);
		else
			return get_hfi_port(inst, OUTPUT_PORT);
	}

	if (capability->cap[cap_id].flags & CAP_FLAG_INPUT_PORT)
		return get_hfi_port(inst, INPUT_PORT);
	else if (capability->cap[cap_id].flags & CAP_FLAG_OUTPUT_PORT)
		return get_hfi_port(inst, OUTPUT_PORT);
	else
		return HFI_PORT_NONE;
}

static const char * const * msm_vidc_get_qmenu_type(
		struct msm_vidc_inst *inst, u32 control_id)
{
	switch (control_id) {
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		return mpeg_video_rate_control;
	case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD:
		return mpeg_video_stream_format;
	case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES:
		return mpeg_video_blur_types;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
		return mpeg_video_avc_coding_layer;
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
		return mpeg_video_hevc_profile;
	default:
		i_vpr_e(inst, "%s: No available qmenu for ctrl %#x\n",
			__func__, control_id);
		return NULL;
	}
}

static int msm_vidc_packetize_control(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 payload_type,
	void *hfi_val, u32 payload_size, const char *func)
{
	int rc = 0;
	u64 payload = 0;

	if (payload_size == sizeof(u32))
		payload = *(u32 *)hfi_val;
	else if (payload_size == sizeof(u64))
		payload = *(u64 *)hfi_val;
	else if (payload_size == sizeof(u8))
		payload = *(u8 *)hfi_val;
	else if (payload_size == sizeof(u16))
		payload = *(u16 *)hfi_val;

	if (payload_size <= sizeof(u64))
		i_vpr_h(inst,
			"set cap: name: %24s, cap value: %#10x, hfi: %#10x\n",
			cap_name(cap_id), inst->capabilities->cap[cap_id].value, payload);
	else
		i_vpr_h(inst,
			"set cap: name: %24s, hfi payload size %d\n",
			cap_name(cap_id), payload_size);

	rc = venus_hfi_session_property(inst,
		inst->capabilities->cap[cap_id].hfi_id,
		HFI_HOST_FLAGS_NONE,
		msm_vidc_get_port_info(inst, cap_id),
		payload_type,
		hfi_val,
		payload_size);
	if (rc) {
		i_vpr_e(inst, "%s: failed to set cap[%d] %s to fw\n",
			__func__, cap_id, cap_name(cap_id));
		return rc;
	}

	return 0;
}

static enum msm_vidc_inst_capability_type msm_vidc_get_cap_id(
	struct msm_vidc_inst *inst, u32 id)
{
	enum msm_vidc_inst_capability_type i = INST_CAP_NONE + 1;
	struct msm_vidc_inst_capability *capability;
	enum msm_vidc_inst_capability_type cap_id = INST_CAP_NONE;

	capability = inst->capabilities;
	do {
		if (capability->cap[i].v4l2_id == id) {
			cap_id = capability->cap[i].cap;
			break;
		}
		i++;
	} while (i < INST_CAP_MAX);

	return cap_id;
}

static int msm_vidc_add_capid_to_list(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id,
	enum msm_vidc_ctrl_list_type type)
{
	struct msm_vidc_inst_cap_entry *entry = NULL, *curr_node = NULL;

	/* skip adding if cap_id already present in list */
	if (type & FW_LIST) {
		list_for_each_entry(curr_node, &inst->firmware.list, list) {
			if (curr_node->cap_id == cap_id) {
				i_vpr_l(inst,
					"%s: cap[%d] %s already present in FW_LIST\n",
					__func__, cap_id, cap_name(cap_id));
				return 0;
			}
		}
	}

	entry = kzalloc(sizeof(*entry), GFP_ATOMIC);
	if (!entry) {
		i_vpr_e(inst, "%s: alloc failed\n", __func__);
		return -ENOMEM;
	}
	entry->cap_id = cap_id;
	if (type & CHILD_LIST)
		list_add_tail(&entry->list, &inst->children.list);
	if (type & FW_LIST)
		list_add_tail(&entry->list, &inst->firmware.list);

	return 0;
}

static int msm_vidc_add_children(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	int i = 0;
	struct msm_vidc_inst_capability *capability = inst->capabilities;

	while (i < MAX_CAP_CHILDREN &&
		capability->cap[cap_id].children[i]) {
		rc = msm_vidc_add_capid_to_list(inst,
			capability->cap[cap_id].children[i],
			CHILD_LIST);
		if (rc)
			return rc;
		i++;
	}
	return rc;
}

static bool is_parent_available(struct msm_vidc_inst* inst,
	u32 cap, u32 check_parent, const char* func)
{
	int i = 0;
	u32 cap_parent;

	while (i < MAX_CAP_PARENTS &&
		inst->capabilities->cap[cap].parents[i]) {
		cap_parent = inst->capabilities->cap[cap].parents[i];
		if (cap_parent == check_parent) {
			return true;
		}
		i++;
	}

	i_vpr_e(inst,
		"%s: missing parent %s for %s\n",
		func, cap_name(check_parent), cap_name(cap));
	return false;
}

int msm_vidc_update_cap_value(struct msm_vidc_inst *inst, u32 cap,
	s32 adjusted_val, const char *func)
{
	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->capabilities->cap[cap].value != adjusted_val)
		i_vpr_h(inst,
			"%s: updated database: name: %s, value: %#x -> %#x\n",
			func, cap_name(cap),
			inst->capabilities->cap[cap].value, adjusted_val);

	inst->capabilities->cap[cap].value = adjusted_val;

	return 0;
}

static int msm_vidc_get_parent_value(struct msm_vidc_inst* inst,
	u32 cap, u32 parent, s32 *value, const char *func)
{
	int rc = 0;

	if (is_parent_available(inst, cap, parent, __func__)) {
		switch (parent) {
		case BITRATE_MODE:
			*value = inst->hfi_rc_type;
			break;
		case LAYER_TYPE:
			*value = inst->hfi_layer_type;
			break;
		default:
			*value = inst->capabilities->cap[parent].value;
			break;
		}
	} else {
		rc = -EINVAL;
	}

	return rc;
}

static int msm_vidc_adjust_hevc_qp(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id)
{
	struct msm_vidc_inst_capability *capability;
	s32 pix_fmt = -1;

	capability = inst->capabilities;

	if (!(inst->codec == MSM_VIDC_HEVC || inst->codec == MSM_VIDC_HEIC)) {
		i_vpr_e(inst,
			"%s: incorrect cap[%d] %s entry in database, fix database\n",
			__func__, cap_id, cap_name(cap_id));
		return -EINVAL;
	}

	if (msm_vidc_get_parent_value(inst, cap_id,
		PIX_FMTS, &pix_fmt, __func__))
		return -EINVAL;

	if (pix_fmt == MSM_VIDC_FMT_P010 || pix_fmt == MSM_VIDC_FMT_TP10C)
		goto exit;

	CAP_TO_8BIT_QP(capability->cap[cap_id].value);
	if (cap_id == MIN_FRAME_QP) {
		CAP_TO_8BIT_QP(capability->cap[I_FRAME_MIN_QP].value);
		CAP_TO_8BIT_QP(capability->cap[P_FRAME_MIN_QP].value);
		CAP_TO_8BIT_QP(capability->cap[B_FRAME_MIN_QP].value);
	} else if (cap_id == MAX_FRAME_QP) {
		CAP_TO_8BIT_QP(capability->cap[I_FRAME_MAX_QP].value);
		CAP_TO_8BIT_QP(capability->cap[P_FRAME_MAX_QP].value);
		CAP_TO_8BIT_QP(capability->cap[B_FRAME_MAX_QP].value);
	}

exit:
	return 0;
}

static int msm_vidc_adjust_property(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;

	capability = inst->capabilities;

	/*
	 * skip for uninitialized cap properties.
	 * Eg: Skip Tramform 8x8 cap that is uninitialized for HEVC codec
	 */
	if (!capability->cap[cap_id].cap)
		return 0;

	if (capability->cap[cap_id].adjust) {
		rc = capability->cap[cap_id].adjust(inst, NULL);
		if (rc)
			goto exit;
	}

	/* add children cap_id's to chidren list */
	rc = msm_vidc_add_children(inst, cap_id);
	if (rc)
		goto exit;

	/* add cap_id to firmware list  */
	rc = msm_vidc_add_capid_to_list(inst, cap_id, FW_LIST);
	if (rc)
		goto exit;

	return 0;

exit:
	return rc;
}

static int msm_vidc_adjust_dynamic_property(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	s32 prev_value;

	capability = inst->capabilities;
	/*
	 * ctrl is NULL for children adjustment calls
	 * When a dynamic control having children is adjusted, check if dynamic
	 * adjustment is allowed for its children.
	 */
	if (!(capability->cap[cap_id].flags & CAP_FLAG_DYNAMIC_ALLOWED)) {
		i_vpr_h(inst,
			"%s: dynamic setting of cap[%d] %s is not allowed\n",
			__func__, cap_id, cap_name(cap_id));
		return 0;
	}

	/*
	 * if ctrl is NULL, it is children of some parent, and hence,
	 * must have an adjust function defined
	 */
	if (!ctrl && !capability->cap[cap_id].adjust) {
		i_vpr_e(inst,
			"%s: child cap[%d] %s must have ajdust function\n",
			__func__, capability->cap[cap_id].cap,
			cap_name(capability->cap[cap_id].cap));
		return -EINVAL;
	}
	prev_value = capability->cap[cap_id].value;

	if (capability->cap[cap_id].adjust) {
		rc = capability->cap[cap_id].adjust(inst, ctrl);
		if (rc)
			goto exit;
	} else if (ctrl) {
		msm_vidc_update_cap_value(inst, cap_id, ctrl->val, __func__);
	}

	/* add children if cap value modified */
	if (capability->cap[cap_id].value != prev_value) {
		rc = msm_vidc_add_children(inst, cap_id);
		if (rc)
			goto exit;
	}

	if (capability->cap[cap_id].value == prev_value && cap_id == GOP_SIZE) {
		/*
		 * Ignore setting same GOP size value to firmware to avoid
		 * unnecessary generation of IDR frame.
		 */
		goto exit;
	}

	/* add cap_id to firmware list always */
	rc = msm_vidc_add_capid_to_list(inst, cap_id, FW_LIST);
	if (rc)
		goto exit;

	return 0;

exit:
	return rc;
}

int msm_vidc_ctrl_deinit(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s(): num ctrls %d\n", __func__, inst->num_ctrls);
	v4l2_ctrl_handler_free(&inst->ctrl_handler);
	memset(&inst->ctrl_handler, 0, sizeof(struct v4l2_ctrl_handler));
	kfree(inst->ctrls);
	inst->ctrls = NULL;

	return 0;
}

int msm_vidc_ctrl_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_core *core;
	int idx = 0;
	struct v4l2_ctrl_config ctrl_cfg = {0};
	int num_ctrls = 0, ctrl_idx = 0;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	capability = inst->capabilities;

	if (!core->v4l2_ctrl_ops) {
		i_vpr_e(inst, "%s: no control ops\n", __func__);
		return -EINVAL;
	}

	for (idx = 0; idx < INST_CAP_MAX; idx++) {
		if (capability->cap[idx].v4l2_id)
			num_ctrls++;
	}
	if (!num_ctrls) {
		i_vpr_e(inst, "%s: no ctrls available in cap database\n",
			__func__);
		return -EINVAL;
	}
	inst->ctrls = kcalloc(num_ctrls,
		sizeof(struct v4l2_ctrl *), GFP_KERNEL);
	if (!inst->ctrls) {
		i_vpr_e(inst, "%s: failed to allocate ctrl\n", __func__);
		return -ENOMEM;
	}

	rc = v4l2_ctrl_handler_init(&inst->ctrl_handler, num_ctrls);
	if (rc) {
		i_vpr_e(inst, "control handler init failed, %d\n",
				inst->ctrl_handler.error);
		goto error;
	}

	for (idx = 0; idx < INST_CAP_MAX; idx++) {
		struct v4l2_ctrl *ctrl;

		if (!capability->cap[idx].v4l2_id)
			continue;

		if (ctrl_idx >= num_ctrls) {
			i_vpr_e(inst,
				"%s: invalid ctrl %#x, max allowed %d\n",
				__func__, capability->cap[idx].v4l2_id,
				num_ctrls);
			rc = -EINVAL;
			goto error;
		}
		i_vpr_l(inst,
			"%s: cap[%d] %24s, value %d min %d max %d step_or_mask %#x flags %#x v4l2_id %#x hfi_id %#x\n",
			__func__, idx, cap_name(idx),
			capability->cap[idx].value,
			capability->cap[idx].min,
			capability->cap[idx].max,
			capability->cap[idx].step_or_mask,
			capability->cap[idx].flags,
			capability->cap[idx].v4l2_id,
			capability->cap[idx].hfi_id);

		memset(&ctrl_cfg, 0, sizeof(struct v4l2_ctrl_config));

		if (is_priv_ctrl(capability->cap[idx].v4l2_id)) {
			/* add private control */
			ctrl_cfg.def = capability->cap[idx].value;
			ctrl_cfg.flags = 0;
			ctrl_cfg.id = capability->cap[idx].v4l2_id;
			ctrl_cfg.max = capability->cap[idx].max;
			ctrl_cfg.min = capability->cap[idx].min;
			ctrl_cfg.ops = core->v4l2_ctrl_ops;
			ctrl_cfg.type = (capability->cap[idx].flags &
					CAP_FLAG_MENU) ?
					V4L2_CTRL_TYPE_MENU :
					V4L2_CTRL_TYPE_INTEGER;
			if (ctrl_cfg.type == V4L2_CTRL_TYPE_MENU) {
				ctrl_cfg.menu_skip_mask =
					~(capability->cap[idx].step_or_mask);
				ctrl_cfg.qmenu = msm_vidc_get_qmenu_type(inst,
					capability->cap[idx].v4l2_id);
			} else {
				ctrl_cfg.step =
					capability->cap[idx].step_or_mask;
			}
			ctrl_cfg.name = cap_name(capability->cap[idx].cap);
			if (!ctrl_cfg.name) {
				i_vpr_e(inst, "%s: %#x ctrl name is null\n",
					__func__, ctrl_cfg.id);
				rc = -EINVAL;
				goto error;
			}
			ctrl = v4l2_ctrl_new_custom(&inst->ctrl_handler,
					&ctrl_cfg, NULL);
		} else {
			if (capability->cap[idx].flags & CAP_FLAG_MENU) {
				ctrl = v4l2_ctrl_new_std_menu(
					&inst->ctrl_handler,
					core->v4l2_ctrl_ops,
					capability->cap[idx].v4l2_id,
					capability->cap[idx].max,
					~(capability->cap[idx].step_or_mask),
					capability->cap[idx].value);
			} else {
				ctrl = v4l2_ctrl_new_std(&inst->ctrl_handler,
					core->v4l2_ctrl_ops,
					capability->cap[idx].v4l2_id,
					capability->cap[idx].min,
					capability->cap[idx].max,
					capability->cap[idx].step_or_mask,
					capability->cap[idx].value);
			}
		}
		if (!ctrl) {
			i_vpr_e(inst, "%s: invalid ctrl %#x\n", __func__,
				capability->cap[idx].v4l2_id);
			rc = -EINVAL;
			goto error;
		}

		rc = inst->ctrl_handler.error;
		if (rc) {
			i_vpr_e(inst,
				"error adding ctrl (%#x) to ctrl handle, %d\n",
				capability->cap[idx].v4l2_id,
				inst->ctrl_handler.error);
			goto error;
		}

		/*
		 * TODO(AS)
		 * ctrl->flags |= capability->cap[idx].flags;
		 */
		ctrl->flags |= V4L2_CTRL_FLAG_EXECUTE_ON_WRITE;
		inst->ctrls[ctrl_idx] = ctrl;
		ctrl_idx++;
	}
	inst->num_ctrls = num_ctrls;
	i_vpr_h(inst, "%s(): num ctrls %d\n", __func__, inst->num_ctrls);

	return 0;
error:
	msm_vidc_ctrl_deinit(inst);

	return rc;
}

static int msm_vidc_update_buffer_count_if_needed(struct msm_vidc_inst* inst,
	struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	bool update_input_port = false, update_output_port = false;

	if (!inst || !ctrl) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}

	switch (ctrl->id) {
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING:
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER:
		update_input_port = true;
		break;
	case V4L2_CID_MPEG_VIDC_THUMBNAIL_MODE:
	case V4L2_CID_MPEG_VIDC_PRIORITY:
		update_input_port = true;
		update_output_port = true;
		break;
	default:
		update_input_port = false;
		update_output_port = false;
		break;
	}

	if (update_input_port) {
		rc = msm_vidc_update_buffer_count(inst, INPUT_PORT);
		if (rc)
			return rc;
	}
	if (update_output_port) {
		rc = msm_vidc_update_buffer_count(inst, OUTPUT_PORT);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_vidc_allow_secure_session(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst *i;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	list_for_each_entry(i, &core->instances, list) {
		if (i->capabilities) {
			if (i->capabilities->cap[SECURE_MODE].value)
				count++;
		}
	}

	if (count > core->capabilities[MAX_SECURE_SESSION_COUNT].value) {
		i_vpr_e(inst,
			"%s: total secure sessions %d exceeded max limit %d\n",
			__func__, count,
			core->capabilities[MAX_SECURE_SESSION_COUNT].value);
		rc = -EINVAL;
	}
	core_unlock(core, __func__);

	return rc;
}

int msm_v4l2_op_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	enum msm_vidc_inst_capability_type cap_id;
	struct msm_vidc_inst_cap_entry *curr_node = NULL, *tmp_node = NULL;
	struct msm_vidc_inst_capability *capability;

	if (!ctrl) {
		d_vpr_e("%s: invalid ctrl parameter\n", __func__);
		return -EINVAL;
	}

	inst = container_of(ctrl->handler,
		struct msm_vidc_inst, ctrl_handler);

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid parameters for inst\n", __func__);
		return -EINVAL;
	}

	capability = inst->capabilities;

	i_vpr_h(inst, "%s: state %d, name %s, id 0x%x value %d\n",
		__func__, inst->state, ctrl->name, ctrl->id, ctrl->val);

	cap_id = msm_vidc_get_cap_id(inst, ctrl->id);
	if (cap_id == INST_CAP_NONE) {
		i_vpr_e(inst, "%s: could not find cap_id for ctrl %s\n",
			__func__, ctrl->name);
		return -EINVAL;
	}

	capability->cap[cap_id].flags |= CAP_FLAG_CLIENT_SET;
	/* Static setting */
	if (!inst->vb2q[OUTPUT_PORT].streaming) {
		msm_vidc_update_cap_value(inst, cap_id, ctrl->val, __func__);

		if (ctrl->id == V4L2_CID_MPEG_VIDC_SECURE) {
			if (ctrl->val) {
				rc = msm_vidc_allow_secure_session(inst);
				if (rc)
					return rc;
			}
		}

		if (ctrl->id == V4L2_CID_ROTATE) {
			if (ctrl->val == 90 || ctrl->val == 270) {
				struct v4l2_format *output_fmt;

				output_fmt = &inst->fmts[OUTPUT_PORT];
				rc = msm_venc_s_fmt_output(inst, output_fmt);
				if (rc)
					return rc;
			}
		}

		if (ctrl->id == V4L2_CID_MPEG_VIDC_MIN_BITSTREAM_SIZE_OVERWRITE) {
			rc = msm_vidc_update_bitstream_buffer_size(inst);
			if (rc)
				return rc;
		}
		if (ctrl->id == V4L2_CID_MPEG_VIDC_PRIORITY) {
			rc = msm_vidc_adjust_session_priority(inst, ctrl);
			if (rc)
				return rc;

			/**
			 * This is the last static s_ctrl from client(commit point). So update
			 * input & output counts to reflect final buffer counts based on dcvs
			 * & decoder_batching enable/disable. So client is expected to query
			 * for final counts after setting priority control.
			 */
			if (is_decode_session(inst))
				inst->decode_batch.enable = msm_vidc_allow_decode_batch(inst);

			msm_vidc_allow_dcvs(inst);
		}
		if (is_meta_ctrl(ctrl->id)) {
			if (cap_id == META_DPB_TAG_LIST) {
				/*
				* To subscribe HFI_PROP_DPB_TAG_LIST
				* data in FBD, HFI_PROP_BUFFER_TAG data
				* must be delivered via FTB. Hence, update
				* META_OUTPUT_BUF_TAG when META_DPB_TAG_LIST
				* is updated.
				*/
				msm_vidc_update_cap_value(inst, META_OUTPUT_BUF_TAG,
					ctrl->val, __func__);
			}

			rc = msm_vidc_update_meta_port_settings(inst);
			if (rc)
				return rc;
		}
		rc = msm_vidc_update_buffer_count_if_needed(inst, ctrl);
		if (rc)
			return rc;

		return 0;
	}

	/* check if dynamic adjustment is allowed */
	if (inst->vb2q[OUTPUT_PORT].streaming &&
		!(capability->cap[cap_id].flags & CAP_FLAG_DYNAMIC_ALLOWED)) {
		i_vpr_e(inst,
			"%s: dynamic setting of cap[%d] %s is not allowed\n",
			__func__, cap_id, cap_name(cap_id));
		return -EBUSY;
	}

	rc = msm_vidc_adjust_dynamic_property(inst, cap_id, ctrl);
	if (rc)
		goto exit;

	/* adjust all children if any */
	list_for_each_entry_safe(curr_node, tmp_node,
			&inst->children.list, list) {
		rc = msm_vidc_adjust_dynamic_property(
				inst, curr_node->cap_id, NULL);
		if (rc)
			goto exit;
		list_del(&curr_node->list);
		kfree(curr_node);
	}

	/* dynamic controls with request will be set along with qbuf */
	if (inst->request)
		return 0;

	/* Dynamic set control ASAP */
	rc = msm_vidc_set_v4l2_properties(inst);
	if (rc) {
		i_vpr_e(inst, "%s: setting %s failed\n",
			__func__, ctrl->name);
		goto exit;
	}

	if (ctrl->id == V4L2_CID_MPEG_VIDC_LOWLATENCY_REQUEST) {
		if (ctrl->val == V4L2_MPEG_MSM_VIDC_ENABLE) {
			rc = msm_vidc_set_seq_change_at_sync_frame(inst);
			if (rc)
				return rc;
		}
	}

exit:
	if (rc)
		msm_vidc_free_capabililty_list(inst, CHILD_LIST | FW_LIST);

	return rc;
}

int msm_vidc_adjust_entropy_mode(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 profile = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	/* ctrl is always NULL in streamon case */
	adjusted_value = ctrl ? ctrl->val :
		capability->cap[ENTROPY_MODE].value;

	if (inst->codec != MSM_VIDC_H264) {
		i_vpr_e(inst,
			"%s: incorrect entry in database. fix the database\n",
			__func__);
		return 0;
	}

	if (msm_vidc_get_parent_value(inst, ENTROPY_MODE,
		PROFILE, &profile, __func__))
		return -EINVAL;

	if (profile == V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE ||
		profile == V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE)
		adjusted_value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC;

	msm_vidc_update_cap_value(inst, ENTROPY_MODE,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_bitrate_mode(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	int lossless, frame_rc, bitrate_mode, frame_skip;
	u32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	bitrate_mode = capability->cap[BITRATE_MODE].value;
	lossless = capability->cap[LOSSLESS].value;
	frame_rc = capability->cap[FRAME_RC_ENABLE].value;
	frame_skip = capability->cap[FRAME_SKIP_MODE].value;

	if (lossless || (msm_vidc_lossless_encode &&
		inst->codec == MSM_VIDC_HEVC)) {
		hfi_value = HFI_RC_LOSSLESS;
		goto update;
	}

	if (!frame_rc && !is_image_session(inst)) {
		hfi_value = HFI_RC_OFF;
		goto update;
	}

	if (bitrate_mode == V4L2_MPEG_VIDEO_BITRATE_MODE_VBR) {
		hfi_value = HFI_RC_VBR_CFR;
	} else if (bitrate_mode == V4L2_MPEG_VIDEO_BITRATE_MODE_CBR) {
		if (frame_skip)
			hfi_value = HFI_RC_CBR_VFR;
		else
			hfi_value = HFI_RC_CBR_CFR;
	} else if (bitrate_mode == V4L2_MPEG_VIDEO_BITRATE_MODE_CQ) {
		hfi_value = HFI_RC_CQ;
	}

update:
	inst->hfi_rc_type = hfi_value;
	i_vpr_h(inst, "%s: hfi rc type: %#x\n",
		__func__, inst->hfi_rc_type);

	return 0;
}

int msm_vidc_adjust_profile(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 pix_fmt = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[PROFILE].value;

	/* PIX_FMTS dependency is common across all chipsets.
	 * Hence, PIX_FMTS must be specified as Parent for HEVC profile.
	 * Otherwise it would be a database error that should be fixed.
	 */
	if (msm_vidc_get_parent_value(inst, PROFILE, PIX_FMTS,
		&pix_fmt, __func__))
		return -EINVAL;

	/* 10 bit profile for 10 bit color format */
	if (pix_fmt == MSM_VIDC_FMT_TP10C || pix_fmt == MSM_VIDC_FMT_P010) {
		if (is_image_session(inst))
			adjusted_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10_STILL_PICTURE;
		else
			adjusted_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10;
	} else {
		/* 8 bit profile for 8 bit color format */
		if (is_image_session(inst))
			adjusted_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_STILL_PICTURE;
		else
			adjusted_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN;
	}

	msm_vidc_update_cap_value(inst, PROFILE,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_ltr_count(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1, all_intra = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[LTR_COUNT].value;

	if (msm_vidc_get_parent_value(inst, LTR_COUNT, BITRATE_MODE,
		&rc_type, __func__) ||
		msm_vidc_get_parent_value(inst, LTR_COUNT, ALL_INTRA,
		&all_intra, __func__))
		return -EINVAL;

	if ((rc_type != HFI_RC_OFF &&
		rc_type != HFI_RC_CBR_CFR &&
		rc_type != HFI_RC_CBR_VFR) ||
		all_intra) {
		adjusted_value = 0;
		i_vpr_h(inst,
			"%s: ltr count unsupported, rc_type: %#x, all_intra %d\n",
			__func__,rc_type, all_intra);
	}

	msm_vidc_update_cap_value(inst, LTR_COUNT,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_use_ltr(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value, ltr_count;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[USE_LTR].value;

	/*
	 * Since USE_LTR is only set dynamically, and LTR_COUNT is static
	 * control, no need to make LTR_COUNT as parent for USE_LTR as
	 * LTR_COUNT value will always be updated when dynamically USE_LTR
	 * is set
	 */
	ltr_count = capability->cap[LTR_COUNT].value;
	if (!ltr_count)
		return 0;

	if (adjusted_value <= 0 ||
		adjusted_value > ((1 << ltr_count) - 1)) {
		/*
		 * USE_LTR is bitmask value, hence should be
		 * > 0 and <= (2 ^ LTR_COUNT) - 1
		 */
		i_vpr_e(inst, "%s: invalid value %d\n",
			__func__, adjusted_value);
		return 0;
	}

	/* USE_LTR value is a bitmask value */
	msm_vidc_update_cap_value(inst, USE_LTR,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_mark_ltr(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value, ltr_count;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[MARK_LTR].value;

	/*
	 * Since MARK_LTR is only set dynamically, and LTR_COUNT is static
	 * control, no need to make LTR_COUNT as parent for MARK_LTR as
	 * LTR_COUNT value will always be updated when dynamically MARK_LTR
	 * is set
	 */
	ltr_count = capability->cap[LTR_COUNT].value;
	if (!ltr_count)
		return 0;

	if (adjusted_value < 0 ||
		adjusted_value > (ltr_count - 1)) {
		/* MARK_LTR value should be >= 0 and <= (LTR_COUNT - 1) */
		i_vpr_e(inst, "%s: invalid value %d\n",
			__func__, adjusted_value);
		return 0;
	}

	msm_vidc_update_cap_value(inst, MARK_LTR,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_ir_random(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value, all_intra = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[IR_RANDOM].value;

	if (msm_vidc_get_parent_value(inst, IR_RANDOM, ALL_INTRA,
		&all_intra, __func__))
		return -EINVAL;

	if (all_intra) {
		adjusted_value = 0;
		i_vpr_h(inst, "%s: IR unsupported, all intra: %d\n",
			__func__, all_intra);
		goto exit;
	}

	/*
	 * BITRATE_MODE dependency is NOT common across all chipsets.
	 * Hence, do not return error if not specified as one of the parent.
	 */
	if (is_parent_available(inst, IR_RANDOM, BITRATE_MODE, __func__) &&
		inst->hfi_rc_type != HFI_RC_CBR_CFR &&
		inst->hfi_rc_type != HFI_RC_CBR_VFR)
		adjusted_value = 0;

exit:
	msm_vidc_update_cap_value(inst, IR_RANDOM,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_delta_based_rc(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[TIME_DELTA_BASED_RC].value;

	if (msm_vidc_get_parent_value(inst, TIME_DELTA_BASED_RC,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type == HFI_RC_OFF ||
		rc_type == HFI_RC_CQ)
		adjusted_value = 0;

	msm_vidc_update_cap_value(inst, TIME_DELTA_BASED_RC,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_transform_8x8(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 profile = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[TRANSFORM_8X8].value;

	if (inst->codec != MSM_VIDC_H264) {
		i_vpr_e(inst,
			"%s: incorrect entry in database. fix the database\n",
			__func__);
		return 0;
	}

	if (msm_vidc_get_parent_value(inst, TRANSFORM_8X8,
		PROFILE, &profile, __func__))
		return -EINVAL;

	if (profile != V4L2_MPEG_VIDEO_H264_PROFILE_HIGH &&
		profile != V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_HIGH)
		adjusted_value = V4L2_MPEG_MSM_VIDC_DISABLE;

	msm_vidc_update_cap_value(inst, TRANSFORM_8X8,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_chroma_qp_index_offset(void *instance,
	struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[CHROMA_QP_INDEX_OFFSET].value;

	if (adjusted_value != MIN_CHROMA_QP_OFFSET)
		adjusted_value = MAX_CHROMA_QP_OFFSET;

	msm_vidc_update_cap_value(inst, CHROMA_QP_INDEX_OFFSET,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_slice_count(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	struct msm_vidc_inst_capability *capability;
	struct v4l2_format *output_fmt;
	s32 adjusted_value, rc_type = -1, slice_mode, all_intra = 0;
	u32 slice_val, mbpf = 0, mbps = 0, max_mbpf = 0, max_mbps = 0;
	u32 update_cap, max_avg_slicesize, output_width, output_height;
	u32 min_width, min_height, max_width, max_height, fps;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	slice_mode = ctrl ? ctrl->val :
		capability->cap[SLICE_MODE].value;

	if (msm_vidc_get_parent_value(inst, SLICE_MODE,
		BITRATE_MODE, &rc_type, __func__) ||
		msm_vidc_get_parent_value(inst, SLICE_MODE,
		ALL_INTRA, &all_intra, __func__))
		return -EINVAL;

	if (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE)
		return 0;

	fps = capability->cap[FRAME_RATE].value >> 16;
	if (fps > MAX_SLICES_FRAME_RATE ||
		(rc_type != HFI_RC_OFF &&
		rc_type != HFI_RC_CBR_CFR &&
		rc_type != HFI_RC_CBR_VFR) ||
		all_intra) {
		adjusted_value = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE;
		update_cap = SLICE_MODE;
		i_vpr_h(inst,
			"%s: slice unsupported, fps: %u, rc_type: %#x, all_intra %d\n",
			__func__, fps, rc_type, all_intra);
		goto exit;
	}

	output_fmt = &inst->fmts[OUTPUT_PORT];
	output_width = output_fmt->fmt.pix_mp.width;
	output_height = output_fmt->fmt.pix_mp.height;

	max_width = (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB) ?
		MAX_MB_SLICE_WIDTH : MAX_BYTES_SLICE_WIDTH;
	max_height = (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB) ?
		MAX_MB_SLICE_HEIGHT : MAX_BYTES_SLICE_HEIGHT;
	min_width = (inst->codec == MSM_VIDC_HEVC) ?
		MIN_HEVC_SLICE_WIDTH : MIN_AVC_SLICE_WIDTH;
	min_height = MIN_SLICE_HEIGHT;

	/*
	 * For V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB:
	 * 	- width >= 384 and height >= 128
	 * 	- width and height <= 4096
	 * For V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_BYTES:
	 * 	- width >= 192 and height >= 128
	 * 	- width and height <= 1920
	 */
	if (output_width < min_width || output_height < min_height ||
		output_width > max_width || output_height > max_width) {
		adjusted_value = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE;
		update_cap = SLICE_MODE;
		i_vpr_h(inst,
			"%s: slice unsupported, codec: %#x wxh: [%dx%d]\n",
			__func__, inst->codec, output_width, output_height);
		goto exit;
	}

	mbpf = NUM_MBS_PER_FRAME(output_height, output_width);
	mbps = NUM_MBS_PER_SEC(output_height, output_width, fps);
	max_mbpf = NUM_MBS_PER_FRAME(max_height, max_width);
	max_mbps = NUM_MBS_PER_SEC(max_height, max_width, fps);

	if (mbpf > max_mbpf || mbps > max_mbps) {
		adjusted_value = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE;
		update_cap = SLICE_MODE;
		i_vpr_h(inst,
			"%s: Unsupported, mbpf[%u] > max[%u], mbps[%u] > max[%u]\n",
			__func__, mbpf, max_mbpf, mbps, max_mbps);
		goto exit;
	}

	if (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB) {
		update_cap = SLICE_MAX_MB;
		slice_val = capability->cap[SLICE_MAX_MB].value;
		slice_val = max(slice_val, mbpf / MAX_SLICES_PER_FRAME);
	} else {
		slice_val = capability->cap[SLICE_MAX_BYTES].value;
		update_cap = SLICE_MAX_BYTES;
		if (rc_type != HFI_RC_OFF) {
			max_avg_slicesize = ((capability->cap[BIT_RATE].value /
				fps) / 8) /
				MAX_SLICES_PER_FRAME;
			slice_val = max(slice_val, max_avg_slicesize);
		}
	}
	adjusted_value = slice_val;

exit:
	msm_vidc_update_cap_value(inst, update_cap,
		adjusted_value, __func__);

	return 0;
}

static int msm_vidc_adjust_static_layer_count_and_type(struct msm_vidc_inst *inst,
	s32 layer_count)
{
	bool hb_requested = false;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!layer_count) {
		i_vpr_h(inst, "client not enabled layer encoding\n");
		goto exit;
	}

	if (inst->hfi_rc_type == HFI_RC_CQ) {
		i_vpr_h(inst, "rc type is CQ, disabling layer encoding\n");
		layer_count = 0;
		goto exit;
	}

	if (inst->codec == MSM_VIDC_H264) {
		if (!inst->capabilities->cap[LAYER_ENABLE].value) {
			layer_count = 0;
			goto exit;
		}

		hb_requested = (inst->capabilities->cap[LAYER_TYPE].value ==
				V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_B) ?
				true : false;
	} else if (inst->codec == MSM_VIDC_HEVC) {
		hb_requested = (inst->capabilities->cap[LAYER_TYPE].value ==
				V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B) ?
				true : false;
	}

	if (hb_requested && inst->hfi_rc_type != HFI_RC_VBR_CFR) {
		i_vpr_h(inst,
			"%s: HB layer encoding is supported for VBR rc only\n",
			__func__);
		layer_count = 0;
		goto exit;
	}

	if (!inst->capabilities->cap[META_EVA_STATS].value &&
		hb_requested && (layer_count > 1)) {
		layer_count = 1;
		i_vpr_h(inst,
			"%s: cvp disable supports only one enh layer HB\n",
			__func__);
	}

	/* decide hfi layer type */
	if (hb_requested) {
		inst->hfi_layer_type = HFI_HIER_B;
	} else {
		/* HP requested */
		inst->hfi_layer_type = HFI_HIER_P_SLIDING_WINDOW;
		if (inst->codec == MSM_VIDC_H264 &&
			inst->hfi_rc_type == HFI_RC_VBR_CFR)
			inst->hfi_layer_type = HFI_HIER_P_HYBRID_LTR;
	}

	/* sanitize layer count based on layer type and codec */
	if (inst->hfi_layer_type == HFI_HIER_B) {
		if (layer_count > MAX_ENH_LAYER_HB)
			layer_count = MAX_ENH_LAYER_HB;
	} else if (inst->hfi_layer_type == HFI_HIER_P_HYBRID_LTR) {
		if (layer_count > MAX_AVC_ENH_LAYER_HYBRID_HP)
			layer_count = MAX_AVC_ENH_LAYER_HYBRID_HP;
	} else if (inst->hfi_layer_type == HFI_HIER_P_SLIDING_WINDOW) {
		if (inst->codec == MSM_VIDC_H264) {
			if (layer_count > MAX_AVC_ENH_LAYER_SLIDING_WINDOW)
				layer_count = MAX_AVC_ENH_LAYER_SLIDING_WINDOW;
		} else {
			if (layer_count > MAX_HEVC_ENH_LAYER_SLIDING_WINDOW)
				layer_count = MAX_HEVC_ENH_LAYER_SLIDING_WINDOW;
		}
	}

exit:
	msm_vidc_update_cap_value(inst, ENH_LAYER_COUNT,
		layer_count, __func__);
	inst->capabilities->cap[ENH_LAYER_COUNT].max = layer_count;
	return 0;
}

int msm_vidc_adjust_layer_count(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	s32 client_layer_count;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	client_layer_count = ctrl ? ctrl->val :
		capability->cap[ENH_LAYER_COUNT].value;

	if (!is_parent_available(inst, ENH_LAYER_COUNT,
		BITRATE_MODE, __func__) ||
		!is_parent_available(inst, ENH_LAYER_COUNT,
		META_EVA_STATS, __func__))
		return -EINVAL;

	if (!inst->vb2q[OUTPUT_PORT].streaming) {
		rc = msm_vidc_adjust_static_layer_count_and_type(inst,
			client_layer_count);
		if (rc)
			goto exit;
	} else {
		if (inst->hfi_layer_type == HFI_HIER_P_HYBRID_LTR ||
			inst->hfi_layer_type == HFI_HIER_P_SLIDING_WINDOW) {
			/* dynamic layer count change is only supported for HP */
			if (client_layer_count >
				inst->capabilities->cap[ENH_LAYER_COUNT].max)
				client_layer_count =
					inst->capabilities->cap[ENH_LAYER_COUNT].max;

			msm_vidc_update_cap_value(inst, ENH_LAYER_COUNT,
				client_layer_count, __func__);
		}
	}

exit:
	return rc;
}

int msm_vidc_adjust_gop_size(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	s32 adjusted_value, enh_layer_count = -1;
	u32 min_gop_size, num_subgops;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[GOP_SIZE].value;

	if (msm_vidc_get_parent_value(inst, GOP_SIZE,
		ENH_LAYER_COUNT, &enh_layer_count, __func__))
		return -EINVAL;

	if (!enh_layer_count)
		goto exit;

	/*
	 * Layer encoding needs GOP size to be multiple of subgop size
	 * And subgop size is 2 ^ number of enhancement layers.
	 */

	/* v4l2 layer count is the number of enhancement layers */
	min_gop_size = 1 << enh_layer_count;
	num_subgops = (adjusted_value + (min_gop_size >> 1)) /
			min_gop_size;
	if (num_subgops)
		adjusted_value = num_subgops * min_gop_size;
	else
		adjusted_value = min_gop_size;

exit:
	msm_vidc_update_cap_value(inst, GOP_SIZE, adjusted_value, __func__);
	return 0;
}

int msm_vidc_adjust_b_frame(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	s32 adjusted_value, enh_layer_count = -1;
	const u32 max_bframe_size = 7;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[B_FRAME].value;

	if (msm_vidc_get_parent_value(inst, B_FRAME,
		ENH_LAYER_COUNT, &enh_layer_count, __func__))
		return -EINVAL;

	if (!enh_layer_count || inst->hfi_layer_type != HFI_HIER_B) {
		adjusted_value = 0;
		goto exit;
	}

	adjusted_value = (1 << enh_layer_count) - 1;
	/* Allowed Bframe values are 0, 1, 3, 7 */
	if (adjusted_value > max_bframe_size)
		adjusted_value = max_bframe_size;

exit:
	msm_vidc_update_cap_value(inst, B_FRAME, adjusted_value, __func__);
	return 0;
}

static bool msm_vidc_check_all_layer_bitrate_set(struct msm_vidc_inst *inst)
{
	bool layer_bitrate_set = true;
	u32 cap_id = 0, i, enh_layer_count;
	u32 layer_br_caps[6] = {L0_BR, L1_BR, L2_BR, L3_BR, L4_BR, L5_BR};

	enh_layer_count = inst->capabilities->cap[ENH_LAYER_COUNT].value;

	for (i = 0; i <= enh_layer_count; i++) {
		if (i >= ARRAY_SIZE(layer_br_caps))
			break;
		cap_id = layer_br_caps[i];
		if (!(inst->capabilities->cap[cap_id].flags & CAP_FLAG_CLIENT_SET)) {
			layer_bitrate_set = false;
			break;
		}
	}

	return layer_bitrate_set;
}

static u32 msm_vidc_get_cumulative_bitrate(struct msm_vidc_inst *inst)
{
	int i;
	u32 cap_id = 0;
	u32 cumulative_br = 0;
	s32 enh_layer_count;
	u32 layer_br_caps[6] = {L0_BR, L1_BR, L2_BR, L3_BR, L4_BR, L5_BR};

	enh_layer_count = inst->capabilities->cap[ENH_LAYER_COUNT].value;

	for (i = 0; i <= enh_layer_count; i++) {
		if (i >= ARRAY_SIZE(layer_br_caps))
			break;
		cap_id = layer_br_caps[i];
		cumulative_br += inst->capabilities->cap[cap_id].value;
	}

	return cumulative_br;
}

int msm_vidc_adjust_bitrate(void *instance, struct v4l2_ctrl *ctrl)
{
	int i, rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value, max_bitrate, enh_layer_count;
	u32 cumulative_bitrate = 0, cap_id = 0, cap_value = 0;
	u32 layer_br_caps[6] = {L0_BR, L1_BR, L2_BR, L3_BR, L4_BR, L5_BR};

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	/* ignore layer bitrate when total bitrate is set */
	if (capability->cap[BIT_RATE].flags & CAP_FLAG_CLIENT_SET) {
		/*
		 * For static case, ctrl is null.
		 * For dynamic case, only BIT_RATE cap uses this adjust function.
		 * Hence, no need to check for ctrl id to be BIT_RATE control, and not
		 * any of layer bitrate controls.
		 */
		adjusted_value = ctrl ? ctrl->val : capability->cap[BIT_RATE].value;
		msm_vidc_update_cap_value(inst, BIT_RATE, adjusted_value, __func__);

		return 0;
	}

	if (inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	if (msm_vidc_get_parent_value(inst, BIT_RATE,
		ENH_LAYER_COUNT, &enh_layer_count, __func__))
		return -EINVAL;

	max_bitrate = inst->capabilities->cap[BIT_RATE].max;

	/*
	 * ENH_LAYER_COUNT cap max is positive only if
	 * layer encoding is enabled during streamon.
	 */
	if (capability->cap[ENH_LAYER_COUNT].max) {
		if (!msm_vidc_check_all_layer_bitrate_set(inst)) {
			i_vpr_h(inst,
				"%s: client did not set all layer bitrates\n",
				__func__);
			return 0;
		}

		cumulative_bitrate = msm_vidc_get_cumulative_bitrate(inst);

		/* cap layer bitrates to max supported bitrate */
		if (cumulative_bitrate > max_bitrate) {
			u32 decrement_in_value = 0;
			u32 decrement_in_percent = ((cumulative_bitrate - max_bitrate) * 100) /
				max_bitrate;

			cumulative_bitrate = 0;
			for (i = 0; i <= enh_layer_count; i++) {
				if (i >= ARRAY_SIZE(layer_br_caps))
					break;
				cap_id = layer_br_caps[i];
				cap_value = inst->capabilities->cap[cap_id].value;

				decrement_in_value = (cap_value *
					decrement_in_percent) / 100;
				cumulative_bitrate += (cap_value - decrement_in_value);

				/*
				 * cap value for the L*_BR is changed. Hence, update cap,
				 * and add to FW_LIST to set new values to firmware.
				 */
				msm_vidc_update_cap_value(inst, cap_id,
					(cap_value - decrement_in_value), __func__);
			}
		}

		i_vpr_h(inst,
			"%s: update BIT_RATE with cumulative bitrate\n",
			__func__);
		msm_vidc_update_cap_value(inst, BIT_RATE,
			cumulative_bitrate, __func__);
	}

	return rc;
}

int msm_vidc_adjust_dynamic_layer_bitrate(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	struct msm_vidc_inst_capability *capability;
	u32 cumulative_bitrate = 0;
	u32 client_set_cap_id = INST_CAP_NONE;
	u32 old_br = 0, new_br = 0, exceeded_br = 0;
	s32 max_bitrate;

	if (!inst || !inst->capabilities || !ctrl) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	/* ignore layer bitrate when total bitrate is set */
	if (capability->cap[BIT_RATE].flags & CAP_FLAG_CLIENT_SET)
		return 0;

	if (!inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	/*
	 * ENH_LAYER_COUNT cap max is positive only if
	 * layer encoding is enabled during streamon.
	 */
	if (!capability->cap[ENH_LAYER_COUNT].max) {
		i_vpr_e(inst, "%s: layers not enabled\n", __func__);
		return -EINVAL;
	}

	if (!msm_vidc_check_all_layer_bitrate_set(inst)) {
		i_vpr_h(inst,
			"%s: client did not set all layer bitrates\n",
			__func__);
		return 0;
	}

	client_set_cap_id = msm_vidc_get_cap_id(inst, ctrl->id);
	if (client_set_cap_id == INST_CAP_NONE) {
		i_vpr_e(inst, "%s: could not find cap_id for ctrl %s\n",
			__func__, ctrl->name);
		return -EINVAL;
	}

	cumulative_bitrate = msm_vidc_get_cumulative_bitrate(inst);
	max_bitrate = inst->capabilities->cap[BIT_RATE].max;
	old_br = capability->cap[client_set_cap_id].value;
	new_br = ctrl->val;

	/*
	 * new bitrate is not supposed to cause cumulative bitrate to
	 * exceed max supported bitrate
	 */

	if ((cumulative_bitrate - old_br + new_br) > max_bitrate) {
		/* adjust new bitrate */
		exceeded_br = (cumulative_bitrate - old_br + new_br) - max_bitrate;
		new_br = ctrl->val - exceeded_br;
	}
	msm_vidc_update_cap_value(inst, client_set_cap_id, new_br, __func__);

	/* adjust totol bitrate cap */
	i_vpr_h(inst,
		"%s: update BIT_RATE with cumulative bitrate\n",
		__func__);
	msm_vidc_update_cap_value(inst, BIT_RATE,
		msm_vidc_get_cumulative_bitrate(inst), __func__);

	return rc;
}

int msm_vidc_adjust_peak_bitrate(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1, bitrate = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[PEAK_BITRATE].value;

	if (msm_vidc_get_parent_value(inst, PEAK_BITRATE,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_CBR_CFR &&
		rc_type != HFI_RC_CBR_VFR)
		return 0;

	if (msm_vidc_get_parent_value(inst, PEAK_BITRATE,
		BIT_RATE, &bitrate, __func__))
		return -EINVAL;

	/* Peak Bitrate should be larger than or equal to avg bitrate */
	if (capability->cap[PEAK_BITRATE].flags & CAP_FLAG_CLIENT_SET) {
		if (adjusted_value < bitrate)
			adjusted_value = bitrate;
	} else {
		adjusted_value = capability->cap[BIT_RATE].value;
	}

	msm_vidc_update_cap_value(inst, PEAK_BITRATE,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_hevc_min_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (ctrl)
		msm_vidc_update_cap_value(inst, MIN_FRAME_QP,
			ctrl->val, __func__);

	rc = msm_vidc_adjust_hevc_qp(inst, MIN_FRAME_QP);

	return rc;
}

int msm_vidc_adjust_hevc_max_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (ctrl)
		msm_vidc_update_cap_value(inst, MAX_FRAME_QP,
			ctrl->val, __func__);

	rc = msm_vidc_adjust_hevc_qp(inst, MAX_FRAME_QP);

	return rc;
}

int msm_vidc_adjust_hevc_i_frame_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (ctrl)
		msm_vidc_update_cap_value(inst, I_FRAME_QP,
			ctrl->val, __func__);

	rc = msm_vidc_adjust_hevc_qp(inst, I_FRAME_QP);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_adjust_hevc_p_frame_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (ctrl)
		msm_vidc_update_cap_value(inst, P_FRAME_QP,
			ctrl->val, __func__);

	rc = msm_vidc_adjust_hevc_qp(inst, P_FRAME_QP);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_adjust_hevc_b_frame_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (ctrl)
		msm_vidc_update_cap_value(inst, B_FRAME_QP,
			ctrl->val, __func__);

	rc = msm_vidc_adjust_hevc_qp(inst, B_FRAME_QP);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_adjust_blur_type(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1, cac = -1;
	s32 pix_fmts = -1, min_quality = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[BLUR_TYPES].value;

	if (adjusted_value == VIDC_BLUR_NONE)
		return 0;

	if (msm_vidc_get_parent_value(inst, BLUR_TYPES, BITRATE_MODE,
		&rc_type, __func__) ||
		msm_vidc_get_parent_value(inst, BLUR_TYPES,
		CONTENT_ADAPTIVE_CODING, &cac, __func__) ||
		msm_vidc_get_parent_value(inst, BLUR_TYPES, PIX_FMTS,
		&pix_fmts, __func__) ||
		msm_vidc_get_parent_value(inst, BLUR_TYPES, MIN_QUALITY,
		&min_quality, __func__))
		return -EINVAL;

	if (adjusted_value == VIDC_BLUR_EXTERNAL) {
		if (is_scaling_enabled(inst) || min_quality) {
			adjusted_value = VIDC_BLUR_NONE;
		}
	} else if (adjusted_value == VIDC_BLUR_ADAPTIVE) {
		if (is_scaling_enabled(inst) || min_quality ||
			(rc_type != HFI_RC_VBR_CFR) ||
			!cac || is_10bit_colorformat(pix_fmts)) {
			adjusted_value = VIDC_BLUR_NONE;
		}
	}

	msm_vidc_update_cap_value(inst, BLUR_TYPES,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_all_intra(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 gop_size = -1, bframe = -1;
	u32 width, height, fps, mbps, max_mbps;

	if (!inst || !inst->capabilities || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = capability->cap[ALL_INTRA].value;

	if (msm_vidc_get_parent_value(inst, ALL_INTRA, GOP_SIZE,
		&gop_size, __func__) ||
		msm_vidc_get_parent_value(inst, ALL_INTRA, B_FRAME,
		&bframe, __func__))
		return -EINVAL;

	width = inst->crop.width;
	height = inst->crop.height;
	fps =  msm_vidc_get_fps(inst);
	mbps = NUM_MBS_PER_SEC(height, width, fps);
	core = inst->core;
	max_mbps = core->capabilities[MAX_MBPS_ALL_INTRA].value;

	if (mbps > max_mbps) {
		adjusted_value = 0;
		i_vpr_h(inst, "%s: mbps %d exceeds max supported mbps %d\n",
			__func__, mbps, max_mbps);
		goto exit;
	}

	if (!gop_size && !bframe)
		adjusted_value = 1;

exit:
	msm_vidc_update_cap_value(inst, ALL_INTRA,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_blur_resolution(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 blur_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[BLUR_RESOLUTION].value;

	if (msm_vidc_get_parent_value(inst, BLUR_RESOLUTION, BLUR_TYPES,
		&blur_type, __func__))
		return -EINVAL;

	if (blur_type != VIDC_BLUR_EXTERNAL)
		return 0;

	msm_vidc_update_cap_value(inst, BLUR_RESOLUTION,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_cac(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 min_quality = -1, rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[CONTENT_ADAPTIVE_CODING].value;

	if (inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	if (msm_vidc_get_parent_value(inst, CONTENT_ADAPTIVE_CODING,
		MIN_QUALITY, &min_quality, __func__) ||
		msm_vidc_get_parent_value(inst, CONTENT_ADAPTIVE_CODING,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	/*
	 * CAC is supported only for VBR rc type.
	 * Hence, do not adjust or set to firmware for non VBR rc's
	 */
	if (rc_type != HFI_RC_VBR_CFR) {
		adjusted_value = 0;
		goto adjust;
	}

	if (min_quality) {
		adjusted_value = 1;
		goto adjust;
	}

adjust:
	msm_vidc_update_cap_value(inst, CONTENT_ADAPTIVE_CODING,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_bitrate_boost(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 min_quality = -1, rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[BITRATE_BOOST].value;

	if (inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	if (msm_vidc_get_parent_value(inst, BITRATE_BOOST,
		MIN_QUALITY, &min_quality, __func__) ||
		msm_vidc_get_parent_value(inst, BITRATE_BOOST,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	/*
	 * Bitrate Boost are supported only for VBR rc type.
	 * Hence, do not adjust or set to firmware for non VBR rc's
	 */
	if (rc_type != HFI_RC_VBR_CFR) {
		adjusted_value = 0;
		goto adjust;
	}

	if (min_quality) {
		adjusted_value = MAX_BITRATE_BOOST;
		goto adjust;
	}

adjust:
	msm_vidc_update_cap_value(inst, BITRATE_BOOST,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_min_quality(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 roi_enable = -1, rc_type = -1, enh_layer_count = -1, pix_fmts = -1;
	u32 width, height, frame_rate;
	struct v4l2_format *f;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[MIN_QUALITY].value;

	/*
	 * Although MIN_QUALITY is static, one of its parents,
	 * ENH_LAYER_COUNT is dynamic cap. Hence, dynamic call
	 * may be made for MIN_QUALITY via ENH_LAYER_COUNT.
	 * Therefore, below streaming check is required to avoid
	 * runtime modification of MIN_QUALITY.
	 */
	if (inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	if (msm_vidc_get_parent_value(inst, MIN_QUALITY,
		BITRATE_MODE, &rc_type, __func__) ||
		msm_vidc_get_parent_value(inst, MIN_QUALITY,
		META_ROI_INFO, &roi_enable, __func__) ||
		msm_vidc_get_parent_value(inst, MIN_QUALITY,
		ENH_LAYER_COUNT, &enh_layer_count, __func__))
		return -EINVAL;

	/*
	 * Min Quality is supported only for VBR rc type.
	 * Hence, do not adjust or set to firmware for non VBR rc's
	 */
	if (rc_type != HFI_RC_VBR_CFR) {
		adjusted_value = 0;
		goto update_and_exit;
	}

	frame_rate = inst->capabilities->cap[FRAME_RATE].value >> 16;
	f = &inst->fmts[OUTPUT_PORT];
	width = f->fmt.pix_mp.width;
	height = f->fmt.pix_mp.height;

	/*
	 * VBR Min Quality not supported for:
	 * - HEVC 10bit
	 * - ROI support
	 * - HP encoding
	 * - External Blur
	 * - Resolution beyond 1080P
	 * (It will fall back to CQCAC 25% or 0% (CAC) or CQCAC-OFF)
	 */
	if (inst->codec == MSM_VIDC_HEVC) {
		if (msm_vidc_get_parent_value(inst, MIN_QUALITY,
			PIX_FMTS, &pix_fmts, __func__))
			return -EINVAL;

		if (is_10bit_colorformat(pix_fmts)) {
			i_vpr_h(inst,
				"%s: min quality is supported only for 8 bit\n",
				__func__);
			adjusted_value = 0;
			goto update_and_exit;
		}
	}

	if (res_is_greater_than(width, height, 1920, 1080)) {
		i_vpr_h(inst, "%s: unsupported res, wxh %ux%u\n",
			__func__, width, height);
		adjusted_value = 0;
		goto update_and_exit;
	}

	if (frame_rate > 60) {
		i_vpr_h(inst, "%s: unsupported fps %u\n",
			__func__, frame_rate);
		adjusted_value = 0;
		goto update_and_exit;
	}

	if (roi_enable) {
		i_vpr_h(inst,
			"%s: min quality not supported with roi metadata\n",
			__func__);
		adjusted_value = 0;
		goto update_and_exit;
	}

	if (enh_layer_count && inst->hfi_layer_type != HFI_HIER_B) {
		i_vpr_h(inst,
			"%s: min quality not supported for HP encoding\n",
			__func__);
		adjusted_value = 0;
		goto update_and_exit;
	}

	/* Above conditions are met. Hence enable min quality */
	adjusted_value = MAX_SUPPORTED_MIN_QUALITY;

update_and_exit:
	msm_vidc_update_cap_value(inst, MIN_QUALITY,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_lowlatency_mode(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val :
		capability->cap[LOWLATENCY_MODE].value;

	if (msm_vidc_get_parent_value(inst, LOWLATENCY_MODE, BITRATE_MODE,
		&rc_type, __func__))
		return -EINVAL;

	if (rc_type == HFI_RC_CBR_CFR ||
		rc_type == HFI_RC_CBR_VFR)
		adjusted_value = 1;

	msm_vidc_update_cap_value(inst, LOWLATENCY_MODE,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_session_priority(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	int adjusted_value;
	bool rate_by_client;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;
	adjusted_value = ctrl ? ctrl->val :
		capability->cap[PRIORITY].value;

	if (capability->cap[FRAME_RATE].flags & CAP_FLAG_CLIENT_SET ||
		capability->cap[OPERATING_RATE].flags & CAP_FLAG_CLIENT_SET) {
		rate_by_client = true;
		inst->priority_level = MSM_VIDC_PRIORITY_HIGH;
	} else {
		rate_by_client = false;
		inst->priority_level = MSM_VIDC_PRIORITY_LOW;
	}

	/*
	 * For RT, check for resource feasability.
	 * For NRT, sessions with rate set by client takes higher order
	 * among NRT sessions. They are constraint RT or low priority RT.
	 */
	if (adjusted_value == 0) {
		rc = msm_vidc_check_core_mbps(inst);
		if (rc) {
			i_vpr_e(inst, "%s: unsupported load\n", __func__);
			goto exit;
		}
		rc = capability->cap[FRAME_RATE].value > capability->cap[FRAME_RATE].max;
		if (rc) {
			i_vpr_e(inst, "%s: unsupported FRAME_RATE %u, max %u\n", __func__,
				capability->cap[FRAME_RATE].value >> 16,
				capability->cap[FRAME_RATE].max >> 16);
			rc = -ENOMEM;
			goto exit;
		}
		rc = capability->cap[OPERATING_RATE].value > capability->cap[OPERATING_RATE].max;
		if (rc) {
			i_vpr_e(inst, "%s: unsupported OPERATING_RATE %u, max %u\n", __func__,
				capability->cap[OPERATING_RATE].value >> 16,
				capability->cap[OPERATING_RATE].max >> 16);
			rc = -ENOMEM;
			goto exit;
		}

		rc = msm_vidc_check_core_mbpf(inst);
		if (rc) {
			i_vpr_e(inst, "%s: unsupported load\n", __func__);
			goto exit;
		}
	}

	msm_vidc_update_cap_value(inst, PRIORITY, adjusted_value, __func__);

exit:
	return rc;
}

int msm_vidc_adjust_roi_info(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 rc_type = -1, pix_fmt = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[META_ROI_INFO].value;

	if (msm_vidc_get_parent_value(inst, META_ROI_INFO, BITRATE_MODE,
		&rc_type, __func__))
		return -EINVAL;

	if (msm_vidc_get_parent_value(inst, META_ROI_INFO, PIX_FMTS,
		&pix_fmt, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_VBR_CFR || !is_8bit_colorformat(pix_fmt))
		adjusted_value = 0;

	msm_vidc_update_cap_value(inst, META_ROI_INFO,
		adjusted_value, __func__);

	return 0;
}

/*
 * Loop over instance capabilities with CAP_FLAG_ROOT
 * and call adjust function, where
 * - adjust current capability value
 * - update tail of instance children list with capability children
 * - update instance firmware list with current capability id
 * Now, loop over child list and call its adjust function
 */
int msm_vidc_adjust_v4l2_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i;
	struct msm_vidc_inst_cap_entry *curr_node = NULL, *tmp_node = NULL;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	i_vpr_h(inst, "%s()\n", __func__);
	for (i = 0; i < INST_CAP_MAX; i++) {
		if (capability->cap[i].flags & CAP_FLAG_ROOT) {
			rc = msm_vidc_adjust_property(inst,
					capability->cap[i].cap);
			if (rc)
				goto exit;
		}
	}

	/*
	 * children of all root controls are already
	 * added to inst->children list at this point
	 */
	list_for_each_entry_safe(curr_node, tmp_node,
			&inst->children.list, list) {
		/*
		 * call adjust for each child. Each child adjust
		 * will also update child list at the tail with
		 * its own children list.
		 * Also, if current control id value is updated,
		 * its entry should be added to fw list.
		 */
		rc = msm_vidc_adjust_property(inst, curr_node->cap_id);
		if (rc)
			goto exit;
		list_del(&curr_node->list);
		kfree(curr_node);
	}

exit:
	if (rc)
		msm_vidc_free_capabililty_list(inst, CHILD_LIST | FW_LIST);

	return rc;
}

int msm_vidc_set_header_mode(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	int header_mode, prepend_sps_pps, hdr_metadata;
	u32 hfi_value = 0;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	header_mode = capability->cap[cap_id].value;
	prepend_sps_pps = capability->cap[PREPEND_SPSPPS_TO_IDR].value;
	hdr_metadata = capability->cap[META_SEQ_HDR_NAL].value;

	/* prioritize PREPEND_SPSPPS_TO_IDR mode over other header modes */
	if (prepend_sps_pps)
		hfi_value = HFI_SEQ_HEADER_PREFIX_WITH_SYNC_FRAME;
	else if (header_mode == V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME)
		hfi_value = HFI_SEQ_HEADER_JOINED_WITH_1ST_FRAME;
	else
		hfi_value = HFI_SEQ_HEADER_SEPERATE_FRAME;

	if (hdr_metadata)
		hfi_value |= HFI_SEQ_HEADER_METADATA;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_deblock_mode(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 alpha = 0, beta = 0;
	u32 lf_mode, hfi_value = 0, lf_offset = 6;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	rc = msm_vidc_v4l2_to_hfi_enum(inst, LF_MODE, &lf_mode);
	if (rc)
		return -EINVAL;

	beta = inst->capabilities->cap[LF_BETA].value + lf_offset;
	alpha = inst->capabilities->cap[LF_ALPHA].value + lf_offset;
	hfi_value = (alpha << 16) | (beta << 8) | lf_mode;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_constant_quality(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_vidc_get_parent_value(inst, cap_id,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_CQ)
		return 0;

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_vbr_related_properties(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_vidc_get_parent_value(inst, cap_id,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_VBR_CFR)
		return 0;

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_cbr_related_properties(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_vidc_get_parent_value(inst, cap_id,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_CBR_VFR &&
		rc_type != HFI_RC_CBR_CFR)
		return 0;

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_use_and_mark_ltr(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[LTR_COUNT].value ||
		(inst->capabilities->cap[cap_id].value ==
			INVALID_DEFAULT_MARK_OR_USE_LTR)) {
		i_vpr_h(inst,
			"%s: LTR_COUNT: %d %s: %d, cap %s is not set\n",
			__func__, inst->capabilities->cap[LTR_COUNT].value,
			cap_name(cap_id),
			inst->capabilities->cap[cap_id].value,
			cap_name(cap_id));
		return 0;
	}

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_min_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	struct msm_vidc_inst_capability *capability;
	s32 i_frame_qp = 0, p_frame_qp = 0, b_frame_qp = 0, min_qp_enable = 0;
	u32 i_qp_enable = 0, p_qp_enable = 0, b_qp_enable = 0;
	u32 client_qp_enable = 0, hfi_value = 0, offset = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (capability->cap[MIN_FRAME_QP].flags & CAP_FLAG_CLIENT_SET)
		min_qp_enable = 1;

	if (min_qp_enable ||
		(capability->cap[I_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET))
		i_qp_enable = 1;
	if (min_qp_enable ||
		(capability->cap[P_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET))
		p_qp_enable = 1;
	if (min_qp_enable ||
		(capability->cap[B_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET))
		b_qp_enable = 1;

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable) {
		i_vpr_h(inst,
			"%s: client did not set min qp, cap %s is not set\n",
			__func__, cap_name(cap_id));
		return 0;
	}

	if (is_10bit_colorformat(capability->cap[PIX_FMTS].value))
		offset = 12;

	/*
	 * I_FRAME_MIN_QP, P_FRAME_MIN_QP, B_FRAME_MIN_QP,
	 * MIN_FRAME_QP caps have default value as MIN_QP_10BIT values.
	 * Hence, if client sets either one among MIN_FRAME_QP
	 * and (I_FRAME_MIN_QP or P_FRAME_MIN_QP or B_FRAME_MIN_QP),
	 * max of both caps will result into client set value.
	 */
	i_frame_qp = max(capability->cap[I_FRAME_MIN_QP].value,
			capability->cap[MIN_FRAME_QP].value) + offset;
	p_frame_qp = max(capability->cap[P_FRAME_MIN_QP].value,
			capability->cap[MIN_FRAME_QP].value) + offset;
	b_frame_qp = max(capability->cap[B_FRAME_MIN_QP].value,
			capability->cap[MIN_FRAME_QP].value) + offset;

	hfi_value = i_frame_qp | p_frame_qp << 8 | b_frame_qp << 16 |
		client_qp_enable << 24;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_max_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	struct msm_vidc_inst_capability *capability;
	s32 i_frame_qp = 0, p_frame_qp = 0, b_frame_qp = 0, max_qp_enable = 0;
	u32 i_qp_enable = 0, p_qp_enable = 0, b_qp_enable = 0;
	u32 client_qp_enable = 0, hfi_value = 0, offset = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (capability->cap[MAX_FRAME_QP].flags & CAP_FLAG_CLIENT_SET)
		max_qp_enable = 1;

	if (max_qp_enable ||
		(capability->cap[I_FRAME_MAX_QP].flags & CAP_FLAG_CLIENT_SET))
		i_qp_enable = 1;
	if (max_qp_enable ||
		(capability->cap[P_FRAME_MAX_QP].flags & CAP_FLAG_CLIENT_SET))
		p_qp_enable = 1;
	if (max_qp_enable ||
		(capability->cap[B_FRAME_MAX_QP].flags & CAP_FLAG_CLIENT_SET))
		b_qp_enable = 1;

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable) {
		i_vpr_h(inst,
			"%s: client did not set max qp, cap %s is not set\n",
			__func__, cap_name(cap_id));
		return 0;
	}

	if (is_10bit_colorformat(capability->cap[PIX_FMTS].value))
		offset = 12;

	/*
	 * I_FRAME_MAX_QP, P_FRAME_MAX_QP, B_FRAME_MAX_QP,
	 * MAX_FRAME_QP caps have default value as MAX_QP values.
	 * Hence, if client sets either one among MAX_FRAME_QP
	 * and (I_FRAME_MAX_QP or P_FRAME_MAX_QP or B_FRAME_MAX_QP),
	 * min of both caps will result into client set value.
	 */
	i_frame_qp = min(capability->cap[I_FRAME_MAX_QP].value,
			capability->cap[MAX_FRAME_QP].value) + offset;
	p_frame_qp = min(capability->cap[P_FRAME_MAX_QP].value,
			capability->cap[MAX_FRAME_QP].value) + offset;
	b_frame_qp = min(capability->cap[B_FRAME_MAX_QP].value,
			capability->cap[MAX_FRAME_QP].value) + offset;

	hfi_value = i_frame_qp | p_frame_qp << 8 | b_frame_qp << 16 |
		client_qp_enable << 24;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_frame_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	struct msm_vidc_inst_capability *capab;
	s32 i_frame_qp = 0, p_frame_qp = 0, b_frame_qp = 0;
	u32 i_qp_enable = 0, p_qp_enable = 0, b_qp_enable = 0;
	u32 client_qp_enable = 0, hfi_value = 0, offset = 0;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capab = inst->capabilities;

	if (msm_vidc_get_parent_value(inst, cap_id,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (inst->vb2q[OUTPUT_PORT].streaming) {
		if (rc_type != HFI_RC_OFF) {
			i_vpr_h(inst,
				"%s: dynamic qp not allowed for rc type %d\n",
				__func__, rc_type);
			return 0;
		}
	}

	if (rc_type == HFI_RC_OFF) {
		/* Mandatorily set for rc off case */
		i_qp_enable = p_qp_enable = b_qp_enable = 1;
	} else {
		/* Set only if client has set for NON rc off case */
		if (capab->cap[I_FRAME_QP].flags & CAP_FLAG_CLIENT_SET)
			i_qp_enable = 1;
		if (capab->cap[P_FRAME_QP].flags & CAP_FLAG_CLIENT_SET)
			p_qp_enable = 1;
		if (capab->cap[B_FRAME_QP].flags & CAP_FLAG_CLIENT_SET)
			b_qp_enable = 1;
	}

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable) {
		i_vpr_h(inst,
			"%s: client did not set frame qp, cap %s is not set\n",
			__func__, cap_name(cap_id));
		return 0;
	}

	if (is_10bit_colorformat(capab->cap[PIX_FMTS].value))
		offset = 12;

	i_frame_qp = capab->cap[I_FRAME_QP].value + offset;
	p_frame_qp = capab->cap[P_FRAME_QP].value + offset;
	b_frame_qp = capab->cap[B_FRAME_QP].value + offset;

	hfi_value = i_frame_qp | p_frame_qp << 8 | b_frame_qp << 16 |
		client_qp_enable << 24;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_req_sync_frame(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	s32 prepend_spspps;
	u32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	prepend_spspps = inst->capabilities->cap[PREPEND_SPSPPS_TO_IDR].value;
	if (prepend_spspps)
		hfi_value = HFI_SYNC_FRAME_REQUEST_WITH_PREFIX_SEQ_HDR;
	else
		hfi_value = HFI_SYNC_FRAME_REQUEST_WITHOUT_SEQ_HDR;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_chroma_qp_index_offset(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0, chroma_qp_offset_mode = 0, chroma_qp = 0;
	u32 offset = 12;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->capabilities->cap[cap_id].flags & CAP_FLAG_CLIENT_SET)
		chroma_qp_offset_mode = HFI_FIXED_CHROMAQP_OFFSET;
	else
		chroma_qp_offset_mode = HFI_ADAPTIVE_CHROMAQP_OFFSET;

	chroma_qp = inst->capabilities->cap[cap_id].value + offset;
	hfi_value = chroma_qp_offset_mode | chroma_qp << 8 | chroma_qp << 16 ;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_slice_count(void* instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst* inst = (struct msm_vidc_inst*)instance;
	s32 slice_mode = -1;
	u32 hfi_value = 0, set_cap_id = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	slice_mode = inst->capabilities->cap[SLICE_MODE].value;

	if (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE) {
		i_vpr_h(inst, "%s: slice mode is: %u, ignore setting to fw\n",
			__func__, slice_mode);
		return 0;
	}
	if (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB) {
		hfi_value = (inst->codec == MSM_VIDC_HEVC) ?
			((inst->capabilities->cap[SLICE_MAX_MB].value + 3) / 4) :
			inst->capabilities->cap[SLICE_MAX_MB].value;
		set_cap_id = SLICE_MAX_MB;
	} else if (slice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_BYTES) {
		hfi_value = inst->capabilities->cap[SLICE_MAX_BYTES].value;
		set_cap_id = SLICE_MAX_BYTES;
	}

	rc = msm_vidc_packetize_control(inst, set_cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_nal_length(void* instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = HFI_NAL_LENGTH_STARTCODES;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!inst->capabilities->cap[WITHOUT_STARTCODE].value) {
		hfi_value = HFI_NAL_LENGTH_STARTCODES;
	} else {
		rc = msm_vidc_v4l2_to_hfi_enum(inst, NAL_LENGTH_FIELD, &hfi_value);
		if (rc)
			return -EINVAL;
	}

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_layer_count_and_type(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_layer_count, hfi_layer_type = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!inst->vb2q[OUTPUT_PORT].streaming) {
		/* set layer type */
		hfi_layer_type = inst->hfi_layer_type;
		cap_id = LAYER_TYPE;

		rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
			&hfi_layer_type, sizeof(u32), __func__);
		if (rc)
			goto exit;
	} else {
		if (inst->hfi_layer_type == HFI_HIER_B) {
			i_vpr_l(inst,
				"%s: HB dyn layers change is not supported\n",
				__func__);
			return 0;
		}
	}

	/* set layer count */
	cap_id = ENH_LAYER_COUNT;
	/* hfi baselayer starts from 1 */
	hfi_layer_count = inst->capabilities->cap[ENH_LAYER_COUNT].value + 1;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_layer_count, sizeof(u32), __func__);
	if (rc)
		goto exit;

exit:
	return rc;
}

int msm_vidc_set_gop_size(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->vb2q[OUTPUT_PORT].streaming) {
		if (inst->hfi_layer_type == HFI_HIER_B) {
			i_vpr_l(inst,
				"%s: HB dyn GOP setting is not supported\n",
				__func__);
			return 0;
		}
	}

	hfi_value = inst->capabilities->cap[GOP_SIZE].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_bitrate(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0, i;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;
	s32 rc_type = -1, enh_layer_count = -1;
	u32 layer_br_caps[6] = {L0_BR, L1_BR, L2_BR, L3_BR, L4_BR, L5_BR};

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/* set Total Bitrate */
	if (inst->capabilities->cap[BIT_RATE].flags & CAP_FLAG_CLIENT_SET)
		goto set_total_bitrate;

	/*
	 * During runtime, if BIT_RATE cap CLIENT_SET flag is not set,
	 * then this function will be called due to change in ENH_LAYER_COUNT.
	 * In this case, client did not change bitrate, hence, no need to set
	 * to fw.
	 */
	if (inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	if (msm_vidc_get_parent_value(inst, BIT_RATE,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_CBR_CFR && rc_type != HFI_RC_CBR_VFR) {
		i_vpr_h(inst, "%s: set total bitrate for non CBR rc type\n",
			__func__);
		goto set_total_bitrate;
	}

	if (msm_vidc_get_parent_value(inst, BIT_RATE,
		ENH_LAYER_COUNT, &enh_layer_count, __func__))
		return -EINVAL;

	/*
	 * ENH_LAYER_COUNT cap max is positive only if
	 *    layer encoding is enabled during streamon.
	 */
	if (inst->capabilities->cap[ENH_LAYER_COUNT].max) {
		if (!msm_vidc_check_all_layer_bitrate_set(inst))
			goto set_total_bitrate;

		/* set Layer Bitrate */
		for (i = 0; i <= enh_layer_count; i++) {
			if (i >= ARRAY_SIZE(layer_br_caps))
				break;
			cap_id = layer_br_caps[i];
			hfi_value = inst->capabilities->cap[cap_id].value;
			rc = msm_vidc_packetize_control(inst, cap_id,
				HFI_PAYLOAD_U32, &hfi_value,
				sizeof(u32), __func__);
			if (rc)
				return rc;
		}
		goto exit;
	}

set_total_bitrate:
	hfi_value = inst->capabilities->cap[BIT_RATE].value;
	rc = msm_vidc_packetize_control(inst, BIT_RATE, HFI_PAYLOAD_U32,
			&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;
exit:
	return rc;
}

int msm_vidc_set_dynamic_layer_bitrate(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!inst->vb2q[OUTPUT_PORT].streaming)
		return 0;

	/* set Total Bitrate */
	if (inst->capabilities->cap[BIT_RATE].flags & CAP_FLAG_CLIENT_SET) {
		i_vpr_h(inst,
			"%s: Total bitrate is set, ignore layer bitrate\n",
			__func__);
		return 0;
	}

	/*
	 * ENH_LAYER_COUNT cap max is positive only if
	 *    layer encoding is enabled during streamon.
	 */
	if (!inst->capabilities->cap[ENH_LAYER_COUNT].max ||
		!msm_vidc_check_all_layer_bitrate_set(inst)) {
		i_vpr_h(inst,
			"%s: invalid layer bitrate, ignore setting to fw\n",
			__func__);
		return 0;
	}

	if (inst->hfi_rc_type == HFI_RC_CBR_CFR ||
		rc_type == HFI_RC_CBR_VFR) {
		/* set layer bitrate for the client set layer */
		hfi_value = inst->capabilities->cap[cap_id].value;
		rc = msm_vidc_packetize_control(inst, cap_id,
			HFI_PAYLOAD_U32, &hfi_value,
			sizeof(u32), __func__);
		if (rc)
			return rc;
	} else {
		/*
		 * All layer bitartes set for unsupported rc type.
		 * Hence accept layer bitrates, but set total bitrate prop
		 * with cumulative bitrate.
		 */
		hfi_value = inst->capabilities->cap[BIT_RATE].value;
		rc = msm_vidc_packetize_control(inst, BIT_RATE, HFI_PAYLOAD_U32,
				&hfi_value, sizeof(u32), __func__);
		if (rc)
			return rc;
	}

	return rc;
}

int msm_vidc_set_session_priority(void *instance,
		enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	u32 hfi_value = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	hfi_value = (inst->capabilities->cap[cap_id].value * 2) + inst->priority_level;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	inst->firmware_priority = hfi_value;

	return rc;
}

int msm_vidc_set_flip(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	u32 hflip, vflip, hfi_value = HFI_DISABLE_FLIP;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	hflip = inst->capabilities->cap[HFLIP].value;
	vflip = inst->capabilities->cap[VFLIP].value;

	if (hflip)
		hfi_value |= HFI_HORIZONTAL_FLIP;

	if (vflip)
		hfi_value |= HFI_VERTICAL_FLIP;

	if (inst->vb2q[OUTPUT_PORT].streaming) {
		if (hfi_value != HFI_DISABLE_FLIP) {
			rc = msm_vidc_set_req_sync_frame(inst,
				REQUEST_I_FRAME);
			if (rc)
				return rc;
		}
	}

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_rotation(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_v4l2_to_hfi_enum(inst, cap_id, &hfi_value);
	if (rc)
		return -EINVAL;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_blur_resolution(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	s32 blur_type = -1;
	u32 hfi_value, blur_width, blur_height;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_vidc_get_parent_value(inst, cap_id,
		BLUR_TYPES, &blur_type, __func__))
		return -EINVAL;

	if (blur_type != VIDC_BLUR_EXTERNAL)
		return 0;

	hfi_value = inst->capabilities->cap[cap_id].value;

	blur_width = (hfi_value & 0xFFFF0000) >> 16;
	blur_height = hfi_value & 0xFFFF;

	if (blur_width > inst->crop.width ||
		blur_height > inst->crop.height) {
		i_vpr_e(inst,
			"%s: blur wxh: %dx%d exceeds crop wxh: %dx%d\n",
			__func__, blur_width, blur_height,
			inst->crop.width, inst->crop.height);
		hfi_value = 0;
	}

	if (blur_width == inst->crop.width &&
		blur_height == inst->crop.height) {
		i_vpr_e(inst,
			"%s: blur wxh: %dx%d is equal to crop wxh: %dx%d\n",
			__func__, blur_width, blur_height,
			inst->crop.width, inst->crop.height);
		hfi_value = 0;
	}

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

static msm_venc_set_csc_coeff(struct msm_vidc_inst *inst,
	const char *prop_name, u32 hfi_id, void *payload,
	u32 payload_size, u32 row_count, u32 column_count)
{
	int rc = 0;

	i_vpr_h(inst,
		"set cap: name: %24s, hard coded %dx%d matrix array\n",
		prop_name, row_count, column_count);
	rc = venus_hfi_session_property(inst,
		hfi_id,
		HFI_HOST_FLAGS_NONE,
		HFI_PORT_BITSTREAM,
		HFI_PAYLOAD_S32_ARRAY,
		payload,
		payload_size);
	if (rc) {
		i_vpr_e(inst,
			"%s: failed to set %s to fw\n",
			__func__, prop_name);
	}

	return rc;
}
int msm_vidc_set_csc_custom_matrix(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	int i;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	struct msm_vidc_core *core;
	struct msm_vidc_csc_coeff *csc_coeff;
	s32 matrix_payload[MAX_MATRIX_COEFFS + 2];
	s32 csc_bias_payload[MAX_BIAS_COEFFS + 2];
	s32 csc_limit_payload[MAX_LIMIT_COEFFS + 2];

	if (!inst || !inst->capabilities || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	if (!core->platform) {
		d_vpr_e("%s: invalid core platform\n", __func__);
		return -EINVAL;
	}
	csc_coeff = &core->platform->data.csc_data;

	if (!inst->capabilities->cap[cap_id].value ||
		!inst->capabilities->cap[CSC].value) {
		i_vpr_h(inst,
			"%s: ignored as custom martix %u, csc %u\n",
			__func__, inst->capabilities->cap[cap_id].value,
			inst->capabilities->cap[CSC].value);
		return 0;
	}

	/*
	 * first 2 u32's of payload in each case are for
	 * row and column count, next remaining u32's are
	 * for the actual payload values.
	 */

	/* set custom matrix */
	matrix_payload[0] = 3;
	matrix_payload[1] = 3;

	for(i = 0; i < MAX_MATRIX_COEFFS; i++) {
		if ((i + 2) >= ARRAY_SIZE(matrix_payload))
			break;
		matrix_payload[i + 2] =
			csc_coeff->vpe_csc_custom_matrix_coeff[i];
	}

	rc = msm_venc_set_csc_coeff(inst, "CSC_CUSTOM_MATRIX",
		HFI_PROP_CSC_MATRIX, &matrix_payload[0],
		ARRAY_SIZE(matrix_payload) * sizeof(s32),
		matrix_payload[0], matrix_payload[1]);
	if (rc)
		return rc;

	/* set csc bias */
	csc_bias_payload[0] = 1;
	csc_bias_payload[1] = 3;

	for(i = 0; i < MAX_BIAS_COEFFS; i++) {
		if ((i + 2) >= ARRAY_SIZE(csc_bias_payload))
			break;
		csc_bias_payload[i + 2] =
			csc_coeff->vpe_csc_custom_bias_coeff[i];
	}

	rc = msm_venc_set_csc_coeff(inst, "CSC_BIAS",
		HFI_PROP_CSC_BIAS, &csc_bias_payload[0],
		ARRAY_SIZE(csc_bias_payload) * sizeof(s32),
		csc_bias_payload[0], csc_bias_payload[1]);
	if (rc)
		return rc;

	/* set csc limit */
	csc_limit_payload[0] = 1;
	csc_limit_payload[1] = 6;

	for(i = 0; i < MAX_LIMIT_COEFFS; i++) {
		if ((i + 2) >= ARRAY_SIZE(csc_limit_payload))
			break;
		csc_limit_payload[i + 2] =
			csc_coeff->vpe_csc_custom_limit_coeff[i];
	}

	rc = msm_venc_set_csc_coeff(inst, "CSC_LIMIT",
		HFI_PROP_CSC_LIMIT, &csc_limit_payload[0],
		ARRAY_SIZE(csc_limit_payload) * sizeof(s32),
		csc_limit_payload[0], csc_limit_payload[1]);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_level(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	hfi_value = inst->capabilities->cap[cap_id].value;
	if (!(inst->capabilities->cap[cap_id].flags & CAP_FLAG_CLIENT_SET))
		hfi_value = HFI_LEVEL_NONE;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_q16(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_Q16,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_u32(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->capabilities->cap[cap_id].flags & CAP_FLAG_MENU) {
		rc = msm_vidc_v4l2_menu_to_hfi(inst, cap_id, &hfi_value);
		if (rc)
			return -EINVAL;
	} else {
		hfi_value = inst->capabilities->cap[cap_id].value;
	}

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_u32_enum(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	u32 hfi_value;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_v4l2_to_hfi_enum(inst, cap_id, &hfi_value);
	if (rc)
		return -EINVAL;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_s32(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;
	s32 hfi_value = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_S32,
		&hfi_value, sizeof(s32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_v4l2_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst_cap_entry *curr_node = NULL, *tmp_node = NULL;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);
	capability = inst->capabilities;

	list_for_each_entry_safe(curr_node, tmp_node,
			&inst->firmware.list, list) {

		/*  cap_id's like PIX_FMT etc may not have set functions */
		if (!capability->cap[curr_node->cap_id].set)
			continue;

		rc = capability->cap[curr_node->cap_id].set(inst,
			curr_node->cap_id);
		if (rc)
			goto exit;

		list_del(&curr_node->list);
		kfree(curr_node);
	}

exit:
	msm_vidc_free_capabililty_list(inst, FW_LIST);

	return rc;
}

int msm_vidc_v4l2_menu_to_hfi(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 *value)
{
	struct msm_vidc_inst_capability *capability = inst->capabilities;

	switch (capability->cap[cap_id].v4l2_id) {
	case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
		switch (capability->cap[cap_id].value) {
		case V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC:
			*value = 1;
			break;
		case V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC:
			*value = 0;
			break;
		default:
			*value = 1;
			goto set_default;
		}
		return 0;
	default:
		i_vpr_e(inst,
			"%s: mapping not specified for ctrl_id: %#x\n",
			__func__, capability->cap[cap_id].v4l2_id);
		return -EINVAL;
	}

set_default:
	i_vpr_e(inst,
		"%s: invalid value %d for ctrl id: %#x. Set default: %u\n",
		__func__, capability->cap[cap_id].value,
		capability->cap[cap_id].v4l2_id, *value);
	return 0;
}

int msm_vidc_v4l2_to_hfi_enum(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 *value)
{
	struct msm_vidc_inst_capability *capability = inst->capabilities;

	switch (capability->cap[cap_id].v4l2_id) {
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		*value = inst->hfi_rc_type;
		return 0;
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
	case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
	case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
	case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
	case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES:
		*value = capability->cap[cap_id].value;
		return 0;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
		switch (capability->cap[cap_id].value) {
		case V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B:
			*value = HFI_HIER_B;
			break;
		case V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_P:
			//TODO (AS): check if this is right mapping
			*value = HFI_HIER_P_SLIDING_WINDOW;
			break;
		default:
			*value = HFI_HIER_P_SLIDING_WINDOW;
			goto set_default;
		}
		return 0;
	case V4L2_CID_ROTATE:
		switch (capability->cap[cap_id].value) {
		case 0:
			*value = HFI_ROTATION_NONE;
			break;
		case 90:
			*value = HFI_ROTATION_90;
			break;
		case 180:
			*value = HFI_ROTATION_180;
			break;
		case 270:
			*value = HFI_ROTATION_270;
			break;
		default:
			*value = HFI_ROTATION_NONE;
			goto set_default;
		}
		return 0;
	case V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE:
		switch (capability->cap[cap_id].value) {
		case V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_ENABLED:
			*value = HFI_DEBLOCK_ALL_BOUNDARY;
			break;
		case V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED:
			*value = HFI_DEBLOCK_DISABLE;
			break;
		case DB_HEVC_DISABLE_SLICE_BOUNDARY:
			*value = HFI_DEBLOCK_DISABLE_AT_SLICE_BOUNDARY;
			break;
		default:
			*value = HFI_DEBLOCK_ALL_BOUNDARY;
			goto set_default;
		}
		return 0;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
		switch (capability->cap[cap_id].value) {
		case V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED:
			*value = HFI_DEBLOCK_ALL_BOUNDARY;
			break;
		case V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED:
			*value = HFI_DEBLOCK_DISABLE;
			break;
		case DB_H264_DISABLE_SLICE_BOUNDARY:
			*value = HFI_DEBLOCK_DISABLE_AT_SLICE_BOUNDARY;
			break;
		default:
			*value = HFI_DEBLOCK_ALL_BOUNDARY;
			goto set_default;
		}
		return 0;
	case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD:
		switch (capability->cap[cap_id].value) {
		case V4L2_MPEG_VIDEO_HEVC_SIZE_4:
			*value = HFI_NAL_LENGTH_SIZE_4;
			break;
		default:
			*value = HFI_NAL_LENGTH_STARTCODES;
			goto set_default;
		}
		return 0;
	default:
		i_vpr_e(inst,
			"%s: mapping not specified for ctrl_id: %#x\n",
			__func__, capability->cap[cap_id].v4l2_id);
		return -EINVAL;
	}

set_default:
	i_vpr_e(inst,
		"%s: invalid value %d for ctrl id: %#x. Set default: %u\n",
		__func__, capability->cap[cap_id].value,
		capability->cap[cap_id].v4l2_id, *value);
	return 0;
}

int msm_vidc_set_stage(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	u32 stage = 0;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->capabilities || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = call_session_op(core, decide_work_mode, inst);
	if (rc) {
		i_vpr_e(inst, "%s: decide_work_mode failed\n", __func__);
		return -EINVAL;
	}

	stage = inst->capabilities->cap[STAGE].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&stage, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_pipe(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	u32 pipe;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->capabilities || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = call_session_op(core, decide_work_route, inst);
	if (rc) {
		i_vpr_e(inst, "%s: decide_work_route failed\n",
			__func__);
		return -EINVAL;
	}

	pipe = inst->capabilities->cap[PIPE].value;
	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
			&pipe, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_set_seq_change_at_sync_frame(void *instance)
{
	int rc = 0;
	u32 payload;
	struct msm_vidc_inst* inst = (struct msm_vidc_inst*)instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	payload = inst->capabilities->cap[LOWLATENCY_MODE].value;
	rc = msm_vidc_packetize_control(inst, LOWLATENCY_MODE, HFI_PAYLOAD_U32,
		&payload, sizeof(u32), __func__);
	if (rc)
		return rc;

	return rc;
}

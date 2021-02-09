// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "hfi_packet.h"
#include "hfi_property.h"
#include "venus_hfi.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"

#define CAP_TO_8BIT_QP(a) {          \
	if ((a) < 0)                 \
		(a) = 0;             \
}

static bool is_priv_ctrl(u32 id)
{
	if (IS_PRIV_CTRL(id))
		return true;

	/*
	 * Treat below standard controls as private because
	 * we have added custom values to the controls
	 */

	return false;
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
		i_vpr_e(inst,
			"%s: both ports enabled. Default port set: BITSTREAM\n",
			__func__);
		return HFI_PORT_BITSTREAM;
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
	default:
		i_vpr_e(inst, "%s: No available qmenu for ctrl %#x\n",
			__func__, control_id);
		return NULL;
	}
}

static const char *msm_vidc_get_priv_ctrl_name(struct msm_vidc_inst *inst, u32 control_id)
{
	switch (control_id) {
	case V4L2_CID_MPEG_VIDC_SECURE:
		return "Secure Mode";
	case V4L2_CID_MPEG_VIDC_HEIC:
		return "HEIC";
	case V4L2_CID_MPEG_VIDC_LOWLATENCY_REQUEST:
		return "Low Latency Mode";
	case V4L2_CID_MPEG_VIDC_CODEC_CONFIG:
		return "Codec Config";
	case V4L2_CID_MPEG_VIDC_B_FRAME_MIN_QP:
		return "B frame Min QP";
	case V4L2_CID_MPEG_VIDC_B_FRAME_MAX_QP:
		return "B frame Max QP";
	case V4L2_CID_MPEG_VIDC_LTRCOUNT:
		return "LTR count";
	case V4L2_CID_MPEG_VIDC_USELTRFRAME:
		return "Use LTR Frame";
	case V4L2_CID_MPEG_VIDC_MARKLTRFRAME:
		return "Mark LTR Frame";
	case V4L2_CID_MPEG_VIDC_BASELAYER_PRIORITY:
		return "Baselayer Priority";
	case V4L2_CID_MPEG_VIDC_INTRA_REFRESH_PERIOD:
		return "Intra Refresh Period";
	case V4L2_CID_MPEG_VIDC_AU_DELIMITER:
		return "AU Delimiter";
	case V4L2_CID_MPEG_VIDC_TIME_DELTA_BASED_RC:
		return "Time Delta Based RC";
	case V4L2_CID_MPEG_VIDC_CONTENT_ADAPTIVE_CODING:
		return "Content Adaptive Coding";
	case V4L2_CID_MPEG_VIDC_QUALITY_BITRATE_BOOST:
		return "Quality Bitrate Boost";
	case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES:
		return "Blur Types";
	case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_RESOLUTION:
		return "Blur Resolution";
	case V4L2_CID_MPEG_VIDC_VIDEO_VPE_CSC_CUSTOM_MATRIX:
		return "CSC Custom Matrix";
	case V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY:
		return "H264 Display Delay";
	case V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE:
		return "H264 Display Delay Enable";
	case V4L2_CID_MPEG_VIDC_METADATA_LTR_MARK_USE_DETAILS:
		return "LTR Mark Use Details Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_SEQ_HEADER_NAL:
		return "Seq Header NAL Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_DPB_LUMA_CHROMA_MISR:
		return "DPB Luma-Chroma MISR Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_OPB_LUMA_CHROMA_MISR:
		return "OPB Luma-Chroma MISR Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_INTERLACE:
		return "Interlace Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_CONCEALED_MB_COUNT:
		return "Concealed MB Count Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_HISTOGRAM_INFO:
		return "Historgram Info Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_SEI_MASTERING_DISPLAY_COLOUR:
		return "SEI Mastering Display Color Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_SEI_CONTENT_LIGHT_LEVEL:
		return "SEI Content Lighting Level Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_HDR10PLUS:
		return "HDR10PLUS Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_EVA_STATS:
		return "EVA Stats Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_BUFFER_TAG:
		return "Buffer Tag Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_SUBFRAME_OUTPUT:
		return "Subframe Output Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_ROI_INFO:
		return "ROI Info Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_TIMESTAMP:
		return "Timestamp Metadata";
	case V4L2_CID_MPEG_VIDC_METADATA_ENC_QP_METADATA:
		return "Encoder QP Metadata";
	case V4L2_CID_MPEG_VIDC_MIN_BITSTREAM_SIZE_OVERWRITE:
		return "Bitstream Size Overwrite";
	case V4L2_CID_MPEG_VIDC_HEVC_I_FRAME_MIN_QP:
		return "HEVC I Frame Min QP";
	case V4L2_CID_MPEG_VIDC_HEVC_P_FRAME_MIN_QP:
		return "HEVC P Frame Min QP";
	case V4L2_CID_MPEG_VIDC_HEVC_I_FRAME_MAX_QP:
		return "HEVC I Frame Max QP";
	case V4L2_CID_MPEG_VIDC_HEVC_P_FRAME_MAX_QP:
		return "HEVC P Frame Max QP";
	default:
		i_vpr_e(inst, "%s: ctrl name not available for ctrl id %#x\n",
			__func__, control_id);
		return NULL;
	}
}

static int msm_vidc_packetize_control(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 payload_type,
	void *hfi_val, u32 payload_size, const char *func)
{
	int rc = 0;

	i_vpr_l(inst,
		"%s: hfi_id: %#x, value: %#x\n", func,
		inst->capabilities->cap[cap_id].hfi_id,
		*(s64 *)hfi_val);

	rc = venus_hfi_session_property(inst,
		inst->capabilities->cap[cap_id].hfi_id,
		HFI_HOST_FLAGS_NONE,
		msm_vidc_get_port_info(inst, cap_id),
		payload_type,
		hfi_val,
		sizeof(payload_size));
	if (rc)
		i_vpr_e(inst,
			"%s: failed to set cap_id: %d to fw\n",
			__func__, cap_id);

	return rc;
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
				i_vpr_e(inst,
					"%s: cap %d cannot be the child of two parents\n",
					__func__, cap_id);
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
	u32 cap, u32 check_parent)
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
	return false;
}

static int msm_vidc_update_cap_value(struct msm_vidc_inst* inst, u32 cap,
	s32 adjusted_val, const char *func)
{
	if (inst->capabilities->cap[cap].value != adjusted_val)
		i_vpr_h(inst,
			"%s: updated database value from %#x to %#x\n",
			func, inst->capabilities->cap[cap].value,
			adjusted_val);

	inst->capabilities->cap[cap].value = adjusted_val;

	return 0;
}

static int msm_vidc_get_parent_value(struct msm_vidc_inst* inst,
	u32 cap, u32 parent, s32 *value, const char *func)
{
	int rc = 0;

	if (is_parent_available(inst, cap, parent)) {
		if (parent == BITRATE_MODE)
			*value = inst->hfi_rc_type;
		else
			*value = inst->capabilities->cap[parent].value;
	} else {
		i_vpr_e(inst,
			"%s: missing parent %d for cap %d, please correct database\n",
			func, parent, cap);
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

	if (inst->codec != MSM_VIDC_HEVC) {
		i_vpr_e(inst,
			"%s: incorrect entry in database for cap %d. fix the database\n",
			__func__, cap_id);
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
	} else if (cap_id == I_FRAME_QP) {
		CAP_TO_8BIT_QP(capability->cap[P_FRAME_QP].value);
		CAP_TO_8BIT_QP(capability->cap[B_FRAME_QP].value);
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
		i_vpr_e(inst,
			"%s: dynamic setting of cap_id %d is not allowed\n",
			__func__, cap_id);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}

	/*
	 * if ctrl is NULL, it is children of some parent, and hence,
	 * must have an adjust function defined
	 */
	if (!ctrl && !capability->cap[cap_id].adjust) {
		i_vpr_e(inst,
			"%s: child cap %d must have ajdust function\n",
			__func__, capability->cap[cap_id].cap);
		return -EINVAL;
	}
	prev_value = capability->cap[cap_id].value;

	if (capability->cap[cap_id].adjust) {
		rc = capability->cap[cap_id].adjust(inst, ctrl);
		if (rc)
			goto exit;
	} else if (ctrl) {
		capability->cap[cap_id].value = ctrl->val;
	}

	/* add children if cap value modified */
	if (capability->cap[cap_id].value != prev_value) {
		rc = msm_vidc_add_children(inst, cap_id);
		if (rc)
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
	kfree(inst->ctrls);

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
		return rc;
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
			return -EINVAL;
		}
		i_vpr_h(inst,
			"%s: cap idx %d, value %d min %d max %d step_or_mask %#x flags %#x v4l2_id %#x hfi_id %#x\n",
			__func__, idx,
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
			ctrl_cfg.name = msm_vidc_get_priv_ctrl_name(inst,
					capability->cap[idx].v4l2_id);
			if (!ctrl_cfg.name) {
				i_vpr_e(inst, "%s: %#x ctrl name is null\n",
					__func__, ctrl_cfg.id);
				return -EINVAL;
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
			return -EINVAL;
		}

		rc = inst->ctrl_handler.error;
		if (rc) {
			i_vpr_e(inst,
				"error adding ctrl (%#x) to ctrl handle, %d\n",
				capability->cap[idx].v4l2_id,
				inst->ctrl_handler.error);
			return rc;
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

	if (inst->state == MSM_VIDC_ERROR) {
		i_vpr_e(inst, "%s: set ctrl not allowed in error state\n");
		/* (error name TBD); */
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
		capability->cap[cap_id].value = ctrl->val;

		if (is_meta_ctrl(ctrl->id))
			msm_vidc_update_meta_port_settings(inst);

		return 0;
	}

	/* check if dynamic adjustment is allowed */
	if (inst->vb2q[OUTPUT_PORT].streaming &&
		!(capability->cap[cap_id].flags & CAP_FLAG_DYNAMIC_ALLOWED)) {
		i_vpr_e(inst,
			"%s: dynamic setting of cap_id %d is not allowed\n",
			__func__, cap_id);
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

exit:
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

	if (lossless) {
		hfi_value = HFI_RC_LOSSLESS;
		goto update;
	}

	if (!frame_rc) {
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
	if (pix_fmt == MSM_VIDC_FMT_TP10C ||
		pix_fmt == MSM_VIDC_FMT_P010) {
		adjusted_value = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10;
	} else {
		/* 8 bit profile for 8 bit color format */
		if (adjusted_value == V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10)
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
	s32 rc_type = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[LTR_COUNT].value;

	if (msm_vidc_get_parent_value(inst, LTR_COUNT, BITRATE_MODE,
		&rc_type, __func__))
		return -EINVAL;

	if (rc_type != HFI_RC_OFF &&
		rc_type != HFI_RC_CBR_CFR &&
		rc_type != HFI_RC_CBR_VFR)
		adjusted_value = 0;

	msm_vidc_update_cap_value(inst, LTR_COUNT,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_use_ltr(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 ltr_count = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[USE_LTR].value;

	if (msm_vidc_get_parent_value(inst, USE_LTR, LTR_COUNT,
		&ltr_count, __func__))
		return -EINVAL;

	if (!ltr_count) {
		adjusted_value = 0;
	} else if (adjusted_value <= 0 ||
		adjusted_value >= (1 << ltr_count)) {
		/* USE_LTR value should be > 0 and < (2 ^ LTR_COUNT) */
		i_vpr_e(inst, "%s: invalid value %d\n",
			__func__, adjusted_value);
		return -EINVAL;
	}

	/* USE_LTR value is a bitmask value */
	msm_vidc_update_cap_value(inst, USE_LTR,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_mark_ltr(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	s32 ltr_count = -1;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[MARK_LTR].value;

	if (msm_vidc_get_parent_value(inst, MARK_LTR, LTR_COUNT,
		&ltr_count, __func__))
		return -EINVAL;

	if (!ltr_count) {
		adjusted_value = 0;
	} else if (adjusted_value < 0 ||
		adjusted_value > (ltr_count - 1)) {
		/* MARK_LTR value should be > 0 and <= (LTR_COUNT - 1) */
		i_vpr_e(inst, "%s: invalid value %d\n",
			__func__, adjusted_value);
		return -EINVAL;
	}

	msm_vidc_update_cap_value(inst, MARK_LTR,
		adjusted_value, __func__);

	return 0;
}

int msm_vidc_adjust_ir_random(void *instance, struct v4l2_ctrl *ctrl)
{
	struct msm_vidc_inst_capability *capability;
	s32 adjusted_value;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	adjusted_value = ctrl ? ctrl->val : capability->cap[IR_RANDOM].value;

	/*
	 * BITRATE_MODE dependency is NOT common across all chipsets.
	 * Hence, do not return error if not specified as one of the parent.
	 */
	if (is_parent_available(inst, IR_RANDOM, BITRATE_MODE) &&
		inst->hfi_rc_type != HFI_RC_CBR_CFR &&
		inst->hfi_rc_type != HFI_RC_CBR_VFR)
		adjusted_value = 0;

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

	rc = msm_vidc_adjust_hevc_qp(inst, MAX_FRAME_QP);

	return rc;
}

int msm_vidc_adjust_hevc_frame_qp(void *instance, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	rc = msm_vidc_adjust_hevc_qp(inst, I_FRAME_QP);

	return rc;
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

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

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

	if (header_mode == V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE)
		hfi_value |= HFI_SEQ_HEADER_SEPERATE_FRAME;
	else if (header_mode == V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME)
		hfi_value |= HFI_SEQ_HEADER_JOINED_WITH_1ST_FRAME;

	if (prepend_sps_pps) {
		hfi_value |= HFI_SEQ_HEADER_PREFIX_WITH_SYNC_FRAME;
	}

	if (hdr_metadata) {
		hfi_value |= HFI_SEQ_HEADER_METADATA;
	}

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);

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

	if (!inst->capabilities->cap[LTR_COUNT].value)
		return 0;

	hfi_value = inst->capabilities->cap[cap_id].value;

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32,
		&hfi_value, sizeof(u32), __func__);

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

	min_qp_enable =
		capability->cap[MIN_FRAME_QP].flags & CAP_FLAG_CLIENT_SET;

	i_qp_enable = min_qp_enable ||
		capability->cap[I_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;
	p_qp_enable = min_qp_enable ||
		capability->cap[P_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;
	b_qp_enable = min_qp_enable ||
		capability->cap[B_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable)
		return 0;

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

	max_qp_enable =
		capability->cap[MAX_FRAME_QP].flags & CAP_FLAG_CLIENT_SET;

	i_qp_enable = max_qp_enable ||
		capability->cap[I_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;
	p_qp_enable = max_qp_enable ||
		capability->cap[P_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;
	b_qp_enable = max_qp_enable ||
		capability->cap[B_FRAME_MIN_QP].flags & CAP_FLAG_CLIENT_SET;

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable)
		return 0;

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

	if (msm_vidc_get_parent_value(inst, I_FRAME_QP,
		BITRATE_MODE, &rc_type, __func__))
		return -EINVAL;

	if (rc_type == HFI_RC_OFF) {
		/* Mandatorily set for rc off case */
		i_qp_enable = p_qp_enable = b_qp_enable = 1;
	} else {
		/* Set only if client has set for NON rc off case */
		i_qp_enable =
			capab->cap[I_FRAME_QP].flags & CAP_FLAG_CLIENT_SET;
		p_qp_enable =
			capab->cap[P_FRAME_QP].flags & CAP_FLAG_CLIENT_SET;
		b_qp_enable =
			capab->cap[B_FRAME_QP].flags & CAP_FLAG_CLIENT_SET;
	}

	client_qp_enable = i_qp_enable | p_qp_enable << 1 | b_qp_enable << 2;
	if (!client_qp_enable)
		return 0;

	if (is_10bit_colorformat(capab->cap[PIX_FMTS].value))
		offset = 12;

	i_frame_qp = capab->cap[I_FRAME_QP].value + offset;
	p_frame_qp = capab->cap[P_FRAME_QP].value + offset;
	b_frame_qp = capab->cap[B_FRAME_QP].value + offset;

	hfi_value = i_frame_qp | p_frame_qp << 8 | b_frame_qp << 16 |
		client_qp_enable << 24;
	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_32_PACKED,
		&hfi_value, sizeof(u32), __func__);

	return rc;
}

/* TODO
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

	rc = msm_vidc_packetize_control(inst, cap_id, HFI_PAYLOAD_U32_ENUM,
		&hfi_value, sizeof(u32), __func__);

	return rc;
}
*/

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

	return rc;
}

/* Please ignore this function for now. TO DO*/
int msm_vidc_set_array(void *instance,
	enum msm_vidc_inst_capability_type cap_id)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_core *core;

	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	capability = inst->capabilities;
	core = (struct msm_vidc_core *)inst->core;

	switch (cap_id) {
	/*
	 * Needed if any control needs to be packed into a structure
	 * and sent for packetization.
	 * payload types may be:
	 * STRUCTURE, BLOB, STRING, PACKED, ARRAY,
	 *
	case BITRATE_MODE:
		i_vpr_h(inst, "%s: %d\n", __func__, hfi_value);
		hfi_create_packet(inst->packet, inst->packet_size,
			offset,
			capability->cap[cap_id].hfi_id,
			HFI_HOST_FLAGS_NONE, HFI_PAYLOAD_ENUM,
			HFI_PORT_NONE, core->packet_id++,
			&capability->cap[PROFILE].value, sizeof(u32));
		break;
	}
	*/
	default:
		i_vpr_e(inst,
			"%s: Unknown cap id %d, cannot set to fw\n",
			__func__, cap_id);
		rc = -EINVAL;
		break;
	}

	return rc;
}

int msm_vidc_set_v4l2_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_inst_cap_entry *curr_node = NULL, *tmp_node = NULL;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
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

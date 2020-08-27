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

static bool is_priv_ctrl(u32 id)
{
	if (IS_PRIV_CTRL(id))
		return true;

	/*
	 * Treat below standard controls as private because
	 * we have added custom values to the controls
	 */
	switch (id) {
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		return true;
	}

	return false;
}

static const char *const mpeg_video_rate_control[] = {
	"VBR",
	"CBR",
	"CBR VFR",
	"MBR",
	"MBR VFR",
	"CQ",
	NULL
};

static const char *const mpeg_video_stream_format[] = {
	"NAL Format Start Codes",
	"NAL Format One NAL Per Buffer",
	"NAL Format One Byte Length",
	"NAL Format Two Byte Length",
	"NAL Format Four Byte Length",
	NULL,
};

static const char *const roi_map_type[] = {
	"None",
	"2-bit",
	"2-bit",
	NULL,
};

static const char * const * msm_vidc_get_qmenu_type(
		struct msm_vidc_inst *inst, u32 control_id)
{
	switch (control_id) {
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		return mpeg_video_rate_control;
	case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD:
		return mpeg_video_stream_format;
	/*
	 * TODO(AS)
	 * case V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE:
	 *	return roi_map_type;
	 */
	default:
		s_vpr_e(inst->sid, "%s: No available qmenu for ctrl %#x",
			__func__, control_id);
		return NULL;
	}
}

static const char *msm_vidc_get_priv_ctrl_name(u32 sid, u32 control_id)
{
	switch (control_id) {
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		return "Video Bitrate Control";
	case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD:
		return "NAL Format";
	/*
	 * TODO(AS)
	 * case V4L2_CID_MPEG_VIDC_VIDEO_ROI_TYPE:
	 *	return "ROI Type";
	 */
	default:
		s_vpr_e(sid, "%s: ctrl name not available for ctrl id %#x",
			__func__, control_id);
		return NULL;
	}
}

int msm_vidc_ctrl_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability;
	struct msm_vidc_core *core;
	int idx = 0;
	struct v4l2_ctrl_config ctrl_cfg = {0};
	int num_ctrls = 0, ctrl_idx = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	capability = inst->capabilities;

	for (idx = 0; idx < INST_CAP_MAX; idx++) {
		if (capability->cap[idx].v4l2_id)
			num_ctrls++;
	}
	if (!num_ctrls) {
		s_vpr_e(inst->sid, "%s: failed to allocate ctrl\n", __func__);
		return -EINVAL;
	}
	inst->ctrls = kcalloc(num_ctrls,
		sizeof(struct v4l2_ctrl *), GFP_KERNEL);
	if (!inst->ctrls) {
		s_vpr_e(inst->sid, "%s: failed to allocate ctrl\n", __func__);
		return -ENOMEM;
	}

	rc = v4l2_ctrl_handler_init(&inst->ctrl_handler, num_ctrls);
	if (rc) {
		s_vpr_e(inst->sid, "control handler init failed, %d\n",
				inst->ctrl_handler.error);
		return rc;
	}

	for (idx = 0; idx < INST_CAP_MAX; idx++) {
		struct v4l2_ctrl *ctrl;

		if (ctrl_idx >= num_ctrls) {
			s_vpr_e(inst->sid,
				"invalid ctrl_idx, max allowed %d\n",
				num_ctrls);
			return -EINVAL;
		}
		if (!capability->cap[idx].v4l2_id)
			continue;

		if (is_priv_ctrl(capability->cap[idx].v4l2_id)) {
			/* add private control */
			ctrl_cfg.def = capability->cap[idx].value;
			ctrl_cfg.flags = 0;
			ctrl_cfg.id = capability->cap[idx].v4l2_id;
			ctrl_cfg.max = capability->cap[idx].max;
			ctrl_cfg.min = capability->cap[idx].min;
			ctrl_cfg.menu_skip_mask =
				~(capability->cap[idx].step_or_mask);
			ctrl_cfg.ops = core->v4l2_ctrl_ops;
			ctrl_cfg.step = capability->cap[idx].step_or_mask;
			ctrl_cfg.type = (capability->cap[idx].flags &
					CAP_FLAG_MENU) ?
					V4L2_CTRL_TYPE_MENU :
					V4L2_CTRL_TYPE_INTEGER;
			ctrl_cfg.qmenu = msm_vidc_get_qmenu_type(inst,
					capability->cap[idx].v4l2_id);
			ctrl_cfg.name = msm_vidc_get_priv_ctrl_name(inst->sid,
					capability->cap[idx].v4l2_id);
			if (!ctrl_cfg.name || !ctrl_cfg.ops) {
				s_vpr_e(inst->sid, "%s: invalid control, %d\n",
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
			s_vpr_e(inst->sid, "%s: invalid ctrl %s\n", __func__,
				ctrl->name);
			return -EINVAL;
		}

		rc = inst->ctrl_handler.error;
		if (rc) {
			s_vpr_e(inst->sid,
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

	return rc;
}

int msm_v4l2_op_s_ctrl(struct v4l2_ctrl *ctrl)
{
	return 0;
}


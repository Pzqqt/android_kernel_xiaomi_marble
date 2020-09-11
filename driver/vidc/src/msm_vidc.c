// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vdec.h"
#include "msm_venc.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_vb2.h"
#include "msm_vidc_v4l2.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_control.h"

#define MSM_VIDC_DRV_NAME "msm_vidc_driver"
/* kernel/msm-4.19 */
#define MSM_VIDC_VERSION     ((0 << 16) + (4 << 8) + 19)

#define MAX_EVENTS 30

bool valid_v4l2_buffer(struct v4l2_buffer *b,
		struct msm_vidc_inst *inst)
{
	return true;
}

static int get_poll_flags(struct msm_vidc_inst *inst, u32 port)
{
	int poll = 0;
	struct vb2_queue *q = NULL;
	struct vb2_buffer *vb = NULL;
	unsigned long flags = 0;

	if (!inst || port >= MAX_PORT) {
		d_vpr_e("%s: invalid params, inst %pK, port %d\n",
			__func__, inst, port);
		return poll;
	}
	q = &inst->vb2q[port];

	spin_lock_irqsave(&q->done_lock, flags);
	if (!list_empty(&q->done_list))
		vb = list_first_entry(&q->done_list, struct vb2_buffer,
							  done_entry);
	if (vb && (vb->state == VB2_BUF_STATE_DONE ||
			   vb->state == VB2_BUF_STATE_ERROR)) {
		if (port == OUTPUT_PORT || port == OUTPUT_META_PORT)
			poll |= POLLIN | POLLRDNORM;
		else if (port == INPUT_PORT || port == INPUT_META_PORT)
			poll |= POLLOUT | POLLWRNORM;
	}
	spin_unlock_irqrestore(&q->done_lock, flags);

	return poll;
}

int msm_vidc_poll(void *instance, struct file *filp,
		struct poll_table_struct *wait)
{
	int poll = 0;
	struct msm_vidc_inst *inst = instance;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	poll_wait(filp, &inst->event_handler.wait, wait);
	poll_wait(filp, &inst->vb2q[INPUT_META_PORT].done_wq, wait);
	poll_wait(filp, &inst->vb2q[OUTPUT_META_PORT].done_wq, wait);
	poll_wait(filp, &inst->vb2q[INPUT_PORT].done_wq, wait);
	poll_wait(filp, &inst->vb2q[OUTPUT_PORT].done_wq, wait);

	if (v4l2_event_pending(&inst->event_handler))
		poll |= POLLPRI;

	poll |= get_poll_flags(inst, INPUT_META_PORT);
	poll |= get_poll_flags(inst, OUTPUT_META_PORT);
	poll |= get_poll_flags(inst, INPUT_PORT);
	poll |= get_poll_flags(inst, OUTPUT_PORT);

	return poll;
}
EXPORT_SYMBOL(msm_vidc_poll);

int msm_vidc_querycap(void *instance, struct v4l2_capability *cap)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !cap) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	strlcpy(cap->driver, MSM_VIDC_DRV_NAME, sizeof(cap->driver));
	cap->bus_info[0] = 0;
	cap->version = MSM_VIDC_VERSION;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE_MPLANE |
		V4L2_CAP_VIDEO_OUTPUT_MPLANE |
		V4L2_CAP_META_CAPTURE |
		V4L2_CAP_META_OUTPUT |
		V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	memset(cap->reserved, 0, sizeof(cap->reserved));

	if (inst->domain == MSM_VIDC_DECODER)
		strlcpy(cap->card, "msm_vidc_decoder", sizeof(cap->card));
	else if (inst->domain == MSM_VIDC_ENCODER)
		strlcpy(cap->card, "msm_vidc_encoder", sizeof(cap->card));
	else
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL(msm_vidc_querycap);

int msm_vidc_enum_fmt(void *instance, struct v4l2_fmtdesc *f)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->domain == MSM_VIDC_DECODER)
		return msm_vdec_enum_fmt(inst, f);
	if (inst->domain == MSM_VIDC_ENCODER)
		return msm_venc_enum_fmt(inst, f);

	return -EINVAL;
}
EXPORT_SYMBOL(msm_vidc_enum_fmt);

int msm_vidc_query_ctrl(void *instance, struct v4l2_queryctrl *q_ctrl)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	struct v4l2_ctrl *ctrl;

	if (!inst || !q_ctrl) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, inst, q_ctrl);
		return -EINVAL;
	}

	ctrl = v4l2_ctrl_find(&inst->ctrl_handler, q_ctrl->id);
	if (!ctrl) {
		s_vpr_e(inst->sid, "%s: get_ctrl failed for id %d\n",
			__func__, q_ctrl->id);
		return -EINVAL;
	}
	q_ctrl->minimum = ctrl->minimum;
	q_ctrl->maximum = ctrl->maximum;
	q_ctrl->default_value = ctrl->default_value;
	q_ctrl->flags = 0;
	q_ctrl->step = ctrl->step;
	s_vpr_h(inst->sid,
		"query ctrl: %s: min %d, max %d, default %d step %d flags %#x\n",
		ctrl->name, q_ctrl->minimum, q_ctrl->maximum,
		q_ctrl->default_value, q_ctrl->step, q_ctrl->flags);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_query_ctrl);

int msm_vidc_query_menu(void *instance, struct v4l2_querymenu *qmenu)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	struct v4l2_ctrl *ctrl;

	if (!inst || !qmenu) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, inst, qmenu);
		return -EINVAL;
	}

	ctrl = v4l2_ctrl_find(&inst->ctrl_handler, qmenu->id);
	if (!ctrl) {
		s_vpr_e(inst->sid, "%s: get_ctrl failed for id %d\n",
			__func__, qmenu->id);
		return -EINVAL;
	}
	if (ctrl->type != V4L2_CTRL_TYPE_MENU) {
		s_vpr_e(inst->sid, "%s: ctrl: %s: type (%d) is not MENU type\n",
			__func__, ctrl->name, ctrl->type);
		return -EINVAL;
	}
	if (qmenu->index < ctrl->minimum || qmenu->index > ctrl->maximum)
		return -EINVAL;

	if (ctrl->menu_skip_mask & (1 << qmenu->index))
		rc = -EINVAL;

	s_vpr_h(inst->sid,
		"%s: ctrl: %s: min %d, max %d, menu_skip_mask %#x, qmenu: id %d, index %d, %s\n",
		__func__, ctrl->name, ctrl->minimum, ctrl->maximum,
		ctrl->menu_skip_mask, qmenu->id, qmenu->index,
		rc ? "not supported" : "supported");
	return rc;
}
EXPORT_SYMBOL(msm_vidc_query_menu);

int msm_vidc_s_fmt(void *instance, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (f->type == INPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_OUTPUT) {
			s_vpr_e(inst->sid,
				"%s: s_fmt(%d) not allowed in %d state\n",
				__func__, f->type, inst->state);
			return -EINVAL;
		}
	} else if (f->type == OUTPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_INPUT &&
		    inst->state != MSM_VIDC_DRAIN_START_INPUT) {
			s_vpr_e(inst->sid,
				"%s: s_fmt(%d) not allowed in %d state\n",
				__func__, f->type, inst->state);
			return -EINVAL;
		}
	}

	if (inst->domain == MSM_VIDC_DECODER)
		rc = msm_vdec_s_fmt(inst, f);
	if (inst->domain == MSM_VIDC_ENCODER)
		rc = msm_venc_s_fmt(inst, f);

	if (rc)
		s_vpr_e(inst->sid, "%s: s_fmt(%d) failed %d\n",
			__func__, f->type, rc);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_s_fmt);

int msm_vidc_g_fmt(void *instance, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->domain == MSM_VIDC_DECODER)
		rc = msm_vdec_g_fmt(inst, f);
	if (inst->domain == MSM_VIDC_ENCODER)
		rc = msm_venc_g_fmt(inst, f);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_g_fmt);

int msm_vidc_s_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !control)
		return -EINVAL;

	return v4l2_s_ctrl(NULL, &inst->ctrl_handler, control);
}
EXPORT_SYMBOL(msm_vidc_s_ctrl);

int msm_vidc_g_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;
	struct v4l2_ctrl *ctrl = NULL;
	int rc = 0;

	if (!inst || !control) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	ctrl = v4l2_ctrl_find(&inst->ctrl_handler, control->id);
	if (ctrl) {
		rc = msm_vidc_get_control(inst, ctrl);
		if (!rc)
			control->value = ctrl->val;
	}
	if (rc)
		s_vpr_e(inst->sid, "%s: failed for control id %#x\n",
			__func__, control->id);
	else
		s_vpr_h(inst->sid, "%s: control id %#x, value %d\n",
			__func__, control->id, control->value);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_g_ctrl);

int msm_vidc_reqbufs(void *instance, struct v4l2_requestbuffers *b)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	int port;

	if (!inst || !b) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);

	if (b->type == INPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_OUTPUT) {
			s_vpr_e(inst->sid,
				"%s: reqbufs(%d) not allowed in %d state\n",
				__func__, b->type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	} else if (b->type == OUTPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_INPUT &&
		    inst->state != MSM_VIDC_DRAIN_START_INPUT) {
			s_vpr_e(inst->sid,
				"%s: reqbufs(%d) not allowed in %d state\n",
				__func__, b->type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	}

	port = v4l2_type_to_driver_port(inst, b->type, __func__);
	if (port < 0) {
		rc = -EINVAL;
		goto unlock;
	}

	rc = vb2_reqbufs(&inst->vb2q[port], b);
	if (rc) {
		s_vpr_e(inst->sid, "%s: vb2_reqbufs(%d) failed, %d\n",
			__func__, b->type, rc);
		goto unlock;
	}

unlock:
	mutex_unlock(&inst->lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_reqbufs);

int msm_vidc_qbuf(void *instance, struct media_device *mdev,
		struct v4l2_buffer *b)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	struct vb2_queue *q;

	if (!inst || !inst->core || !b || !valid_v4l2_buffer(b, inst)) {
		d_vpr_e("%s: invalid params %pK %pK\n", __func__, inst, b);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);

	if (inst->state == MSM_VIDC_ERROR) {
		s_vpr_e(inst->sid, "%s: error state\n", __func__);
		rc = -EINVAL;
		goto unlock;
	}
	if (b->type == INPUT_MPLANE) {
		q = &inst->vb2q[INPUT_PORT];
	} else if (b->type == OUTPUT_MPLANE) {
		q = &inst->vb2q[OUTPUT_PORT];
	} else if (b->type == INPUT_META_PLANE) {
		q = &inst->vb2q[INPUT_META_PORT];
	} else if (b->type == OUTPUT_META_PLANE) {
		q = &inst->vb2q[OUTPUT_META_PORT];
	} else {
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, b->type);
		rc = -EINVAL;
		goto unlock;
	}

	rc = vb2_qbuf(q, mdev, b);
	if (rc)
		s_vpr_e(inst->sid, "%s: failed with %d\n", __func__, rc);

unlock:
	mutex_unlock(&inst->lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_qbuf);

int msm_vidc_dqbuf(void *instance, struct v4l2_buffer *b)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	struct vb2_queue *q;

	if (!inst || !b || !valid_v4l2_buffer(b, inst)) {
		d_vpr_e("%s: invalid params %pK %pK\n", __func__, inst, b);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);
	if (b->type == INPUT_MPLANE) {
		q = &inst->vb2q[INPUT_PORT];
	} else if (b->type == OUTPUT_MPLANE) {
		q = &inst->vb2q[OUTPUT_PORT];
	} else if (b->type == INPUT_META_PLANE) {
		q = &inst->vb2q[INPUT_META_PORT];
	} else if (b->type == OUTPUT_META_PLANE) {
		q = &inst->vb2q[OUTPUT_META_PORT];
	} else {
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, b->type);
		rc = -EINVAL;
		goto unlock;
	}

	rc = vb2_dqbuf(q, b, true);
	if (rc == -EAGAIN) {
		goto unlock;
	} else if (rc) {
		s_vpr_e(inst->sid, "%s: failed with %d\n", __func__, rc);
		goto unlock;
	}

unlock:
	mutex_unlock(&inst->lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_dqbuf);

int msm_vidc_streamon(void *instance, enum v4l2_buf_type type)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	enum msm_vidc_inst_state new_state = 0;
	int port;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);

	if (type == INPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_OUTPUT) {
			s_vpr_e(inst->sid,
				"%s: streamon(%d) not allowed in %d state\n",
				__func__, type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state != MSM_VIDC_OPEN &&
		    inst->state != MSM_VIDC_START_INPUT &&
		    inst->state != MSM_VIDC_DRAIN_START_INPUT) {
			s_vpr_e(inst->sid,
				"%s: streamon(%d) not allowed in %d state\n",
				__func__, type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	}

	port = v4l2_type_to_driver_port(inst, type, __func__);
	if (port < 0) {
		rc = -EINVAL;
		goto unlock;
	}

	rc = vb2_streamon(&inst->vb2q[port], type);
	if (rc) {
		s_vpr_e(inst->sid, "%s: vb2_streamon(%d) failed, %d\n",
			__func__, type, rc);
		goto unlock;
	}

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN) {
			new_state = MSM_VIDC_START_INPUT;
		} else if (inst->state == MSM_VIDC_START_OUTPUT) {
			new_state = MSM_VIDC_START;
		}
		rc = msm_vidc_change_inst_state(inst, new_state, __func__);
		if (rc)
			goto unlock;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN) {
			new_state = MSM_VIDC_START_OUTPUT;
		} else if (inst->state == MSM_VIDC_START_INPUT) {
			new_state = MSM_VIDC_START;
		} else if (inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			if (0 /* check if input port settings change pending */)
				new_state = MSM_VIDC_DRC_DRAIN;
			else
				new_state = MSM_VIDC_DRAIN;
		}
		rc = msm_vidc_change_inst_state(inst, new_state, __func__);
		if (rc)
			goto unlock;
	}

unlock:
	mutex_unlock(&inst->lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_streamon);

int msm_vidc_streamoff(void *instance, enum v4l2_buf_type type)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	enum msm_vidc_inst_state new_state = 0;
	int port;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
		    inst->state == MSM_VIDC_START_OUTPUT) {
			s_vpr_e(inst->sid,
				"%s: streamoff(%d) not allowed in %d state\n",
				__func__, type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
		    inst->state == MSM_VIDC_START_INPUT) {
			s_vpr_e(inst->sid,
				"%s: streamoff(%d) not allowed in %d state\n",
				__func__, type, inst->state);
			rc = -EINVAL;
			goto unlock;
		}
	}

	port = v4l2_type_to_driver_port(inst, type, __func__);
	if (port < 0) {
		rc = -EINVAL;
		goto unlock;
	}

	rc = vb2_streamoff(&inst->vb2q[port], type);
	if (rc) {
		s_vpr_e(inst->sid, "%s: vb2_streamoff(%d) failed, %d\n",
			__func__, type, rc);
		goto unlock;
	}

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			new_state = MSM_VIDC_OPEN;
		} else if (inst->state == MSM_VIDC_START) {
			new_state = MSM_VIDC_START_OUTPUT;
		} else if (inst->state == MSM_VIDC_DRC ||
			   inst->state == MSM_VIDC_DRC_LAST_FLAG ||
			   inst->state == MSM_VIDC_DRAIN ||
		           inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
			   inst->state == MSM_VIDC_DRC_DRAIN ||
			   inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG ||
			   inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			new_state = MSM_VIDC_START_OUTPUT;
			/* discard pending port settings change if any */
		}
		rc = msm_vidc_change_inst_state(inst, new_state, __func__);
		if (rc)
			goto unlock;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			new_state = MSM_VIDC_OPEN;
		} else if (inst->state == MSM_VIDC_START ||
			   inst->state == MSM_VIDC_DRAIN ||
			   inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
			   inst->state == MSM_VIDC_DRC ||
			   inst->state == MSM_VIDC_DRC_LAST_FLAG ||
			   inst->state == MSM_VIDC_DRC_DRAIN) {
			new_state = MSM_VIDC_START_INPUT;
		} else if (inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG) {
			new_state = MSM_VIDC_DRAIN_START_INPUT;
		}
		rc = msm_vidc_change_inst_state(inst, new_state, __func__);
		if (rc)
			goto unlock;
	}

unlock:
	mutex_unlock(&inst->lock);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_streamoff);

int msm_vidc_cmd(void *instance, union msm_v4l2_cmd *cmd)
{
	return 0;
}
EXPORT_SYMBOL(msm_vidc_cmd);

int msm_vidc_enum_framesizes(void *instance, struct v4l2_frmsizeenum *fsize)
{
	struct msm_vidc_inst *inst = instance;
	struct msm_vidc_inst_capability *capability;

	if (!inst || !fsize) {
		d_vpr_e("%s: invalid params: %pK %pK\n",
				__func__, inst, fsize);
		return -EINVAL;
	}
	if (!inst->capabilities) {
		s_vpr_e(inst->sid, "capabilities not available\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;
	fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;
	fsize->stepwise.min_width = capability->cap[FRAME_WIDTH].min;
	fsize->stepwise.max_width = capability->cap[FRAME_WIDTH].max;
	fsize->stepwise.step_width =
		capability->cap[FRAME_WIDTH].step_or_mask;
	fsize->stepwise.min_height = capability->cap[FRAME_HEIGHT].min;
	fsize->stepwise.max_height = capability->cap[FRAME_HEIGHT].max;
	fsize->stepwise.step_height =
		capability->cap[FRAME_HEIGHT].step_or_mask;

	return 0;
}
EXPORT_SYMBOL(msm_vidc_enum_framesizes);

int msm_vidc_subscribe_event(void *instance,
		const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !sub) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	s_vpr_h(inst->sid, "%s: type %d id %d\n", __func__, sub->type, sub->id);
	rc = v4l2_event_subscribe(&inst->event_handler,
		sub, MAX_EVENTS, NULL);
	if (rc)
		s_vpr_e(inst->sid, "%s: fialed, type %d id %d\n",
			__func__, sub->type, sub->id);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_subscribe_event);

int msm_vidc_unsubscribe_event(void *instance,
		const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !sub) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	s_vpr_h(inst->sid, "%s: type %d id %d\n", __func__, sub->type, sub->id);
	rc = v4l2_event_unsubscribe(&inst->event_handler, sub);
	if (rc)
		s_vpr_e(inst->sid, "%s: fialed, type %d id %d\n",
			 __func__, sub->type, sub->id);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_unsubscribe_event);

int msm_vidc_dqevent(void *instance, struct v4l2_event *event)
{
	int rc = 0;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *)instance;

	if (!inst || !event) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	rc = v4l2_event_dequeue(&inst->event_handler, event, false);
	if (rc)
		s_vpr_e(inst->sid, "%s: fialed\n", __func__);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_dqevent);

void *msm_vidc_open(void *vidc_core, u32 session_type)
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	struct msm_vidc_core *core;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	core = vidc_core;
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	if (session_type != MSM_VIDC_DECODER &&
	    session_type != MSM_VIDC_ENCODER) {
		d_vpr_e("%s: invalid session_type %d\n",
			__func__, session_type);
		return NULL;
	}

	if (core->state == MSM_VIDC_CORE_ERROR) {
		d_vpr_e("%s: core invalid state\n", __func__);
		return NULL;
	}

	if (core->state == MSM_VIDC_CORE_DEINIT) {
		rc = msm_vidc_core_init(core);
		if (rc)
			return NULL;
	}

	inst = kzalloc(sizeof(*inst), GFP_KERNEL);
	if (!inst) {
		d_vpr_e("%s: failed to allocate inst memory\n", __func__);
		return NULL;
	}
	inst->core = core;

	inst->capabilities = kzalloc(
		sizeof(struct msm_vidc_inst_capability), GFP_KERNEL);
	if (!inst->capabilities) {
		s_vpr_e(inst->sid,
			"%s: inst capability allocation failed\n", __func__);
		return NULL;
	}

	rc = msm_vidc_add_session(inst);
	if (rc) {
		d_vpr_e("%s: failed to get session id\n", __func__);
		return NULL;
	}

	s_vpr_i(inst->sid, "Opening video instance: %d\n", session_type);

	kref_init(&inst->kref);
	mutex_init(&inst->lock);
	INIT_LIST_HEAD(&inst->buffers.input.list);
	INIT_LIST_HEAD(&inst->buffers.input_meta.list);
	INIT_LIST_HEAD(&inst->buffers.output.list);
	INIT_LIST_HEAD(&inst->buffers.output_meta.list);
	INIT_LIST_HEAD(&inst->buffers.scratch.list);
	INIT_LIST_HEAD(&inst->buffers.scratch_1.list);
	INIT_LIST_HEAD(&inst->buffers.scratch_2.list);
	INIT_LIST_HEAD(&inst->buffers.persist.list);
	INIT_LIST_HEAD(&inst->buffers.persist_1.list);
	INIT_LIST_HEAD(&inst->allocations.scratch.list);
	INIT_LIST_HEAD(&inst->allocations.scratch_1.list);
	INIT_LIST_HEAD(&inst->allocations.scratch_2.list);
	INIT_LIST_HEAD(&inst->allocations.persist.list);
	INIT_LIST_HEAD(&inst->allocations.persist_1.list);
	INIT_LIST_HEAD(&inst->mappings.input.list);
	INIT_LIST_HEAD(&inst->mappings.input_meta.list);
	INIT_LIST_HEAD(&inst->mappings.output.list);
	INIT_LIST_HEAD(&inst->mappings.output_meta.list);
	INIT_LIST_HEAD(&inst->mappings.scratch.list);
	INIT_LIST_HEAD(&inst->mappings.scratch_1.list);
	INIT_LIST_HEAD(&inst->mappings.scratch_2.list);
	INIT_LIST_HEAD(&inst->mappings.persist.list);
	INIT_LIST_HEAD(&inst->mappings.persist_1.list);
	INIT_LIST_HEAD(&inst->children.list);
	INIT_LIST_HEAD(&inst->firmware.list);
	inst->domain = session_type;
	inst->state = MSM_VIDC_OPEN;
	inst->request = false;
	for (i = 0; i < MAX_SIGNAL; i++)
		init_completion(&inst->completions[i]);

	//inst->debugfs_root =
	//	msm_vidc_debugfs_init_inst(inst, core->debugfs_root);

	if (is_decode_session(inst)) {
		rc = msm_vdec_inst_init(inst);
		if (rc)
			goto error;
	} else if (is_encode_session(inst)) {
		rc = msm_venc_inst_init(inst);
		if (rc)
			goto error;
	}
	rc = msm_vidc_ctrl_init(inst);
	if (rc)
		goto error;

	rc = msm_vidc_vb2_queue_init(inst);
	if (rc)
		goto error;

	rc = msm_vidc_setup_event_queue(inst);
	if (rc)
		goto error;

	//msm_power_setup(inst);
	// send cmd to firmware here
	rc = msm_vidc_session_open(inst);
	if (rc)
		goto error;

	return inst;

error:
	msm_vidc_close(inst);
	return NULL;
}
EXPORT_SYMBOL(msm_vidc_open);

int msm_vidc_close(void *instance)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	s_vpr_h(inst->sid, "%s()\n", __func__);
	msm_vidc_session_close(inst);
	msm_vidc_remove_session(inst);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_close);

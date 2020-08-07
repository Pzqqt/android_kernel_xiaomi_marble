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

#define MSM_VIDC_DRV_NAME "msm_vidc_driver"
/* kernel/msm-4.19 */
#define MSM_VIDC_VERSION     ((0 << 16) + (4 << 8) + 19)

#define MAX_EVENTS 30

bool valid_v4l2_buffer(struct v4l2_buffer *b,
		struct msm_vidc_inst *inst)
{
	return true;
}
/*
static int get_poll_flags(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct vb2_queue *outq = &inst->bufq[PORT_INPUT].vb2_bufq;
	struct vb2_queue *capq = &inst->bufq[PORT_OUTPUT].vb2_bufq;
	struct vb2_buffer *out_vb = NULL;
	struct vb2_buffer *cap_vb = NULL;
	unsigned long flags = 0;

	if (v4l2_event_pending(&inst->event_handler))
		rc |= POLLPRI;

	spin_lock_irqsave(&capq->done_lock, flags);
	if (!list_empty(&capq->done_list))
		cap_vb = list_first_entry(&capq->done_list, struct vb2_buffer,
								done_entry);
	if (cap_vb && (cap_vb->state == VB2_BUF_STATE_DONE
				|| cap_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLIN | POLLRDNORM;
	spin_unlock_irqrestore(&capq->done_lock, flags);

	spin_lock_irqsave(&outq->done_lock, flags);
	if (!list_empty(&outq->done_list))
		out_vb = list_first_entry(&outq->done_list, struct vb2_buffer,
								done_entry);
	if (out_vb && (out_vb->state == VB2_BUF_STATE_DONE
				|| out_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLOUT | POLLWRNORM;
	spin_unlock_irqrestore(&outq->done_lock, flags);

	return rc;
}
*/

int msm_vidc_poll(void *instance, struct file *filp,
		struct poll_table_struct *wait)
{
/*
	struct msm_vidc_inst *inst = instance;
	struct vb2_queue *outq = NULL;
	struct vb2_queue *capq = NULL;

	if (!inst)
		return -EINVAL;

	outq = &inst->bufq[PORT_INPUT].vb2_bufq;
	capq = &inst->bufq[PORT_OUTPUT].vb2_bufq;

	poll_wait(filp, &inst->event_handler.wait, wait);
	poll_wait(filp, &capq->done_wq, wait);
	poll_wait(filp, &outq->done_wq, wait);
	return get_poll_flags(inst);
*/
	return 0;
}
EXPORT_SYMBOL(msm_vidc_poll);

int msm_vidc_querycap(void *instance, struct v4l2_capability *cap)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !cap)
		return -EINVAL;

	strlcpy(cap->driver, MSM_VIDC_DRV_NAME, sizeof(cap->driver));
	cap->bus_info[0] = 0;
	cap->version = MSM_VIDC_VERSION;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE_MPLANE |
		V4L2_CAP_VIDEO_OUTPUT_MPLANE |
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

	if (!inst || !f)
		return -EINVAL;

	if (inst->domain == MSM_VIDC_DECODER)
		return 0;//msm_vdec_enum_fmt(instance, f);
	else if (inst->domain == MSM_VIDC_ENCODER)
		return 0;//msm_venc_enum_fmt(instance, f);
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

	if (!inst || !f)
		return -EINVAL;

	if (inst->domain == MSM_VIDC_DECODER)
		rc = msm_vdec_s_fmt(inst, f);
	if (inst->domain == MSM_VIDC_ENCODER)
		rc = 0;//msm_venc_s_fmt(instance, f);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_s_fmt);

int msm_vidc_g_fmt(void *instance, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;

	if (!inst || !f)
		return -EINVAL;

	if (inst->domain == MSM_VIDC_DECODER)
		rc = msm_vdec_g_fmt(inst, f);
	if (inst->domain == MSM_VIDC_ENCODER)
		rc = 0;//msm_venc_g_fmt(instance, f);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_g_fmt);

int msm_vidc_s_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;

	if (!inst || !control)
		return -EINVAL;

	return 0;//msm_comm_s_ctrl(instance, control);
}
EXPORT_SYMBOL(msm_vidc_s_ctrl);

int msm_vidc_g_ctrl(void *instance, struct v4l2_control *control)
{
	struct msm_vidc_inst *inst = instance;
	struct v4l2_ctrl *ctrl = NULL;
	int rc = 0;

	if (!inst || !control)
		return -EINVAL;

	ctrl = v4l2_ctrl_find(&inst->ctrl_handler, control->id);
	if (ctrl) {
		rc = msm_vidc_get_control(inst, ctrl);
		if (!rc)
			control->value = ctrl->val;
	}

	return rc;
}
EXPORT_SYMBOL(msm_vidc_g_ctrl);

int msm_vidc_reqbufs(void *instance, struct v4l2_requestbuffers *b)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	int port;

	if (!inst || !b)
		return -EINVAL;

	port = msm_vidc_get_port_from_type(b->type);
	if (port < 0) {
		d_vpr_e("%s: invalid queue type %d\n", __func__, b->type);
		return -EINVAL;
	}
	mutex_lock(&inst->lock);
	rc = vb2_reqbufs(&inst->vb2q[port], b);
	mutex_unlock(&inst->lock);
	if (rc)
		s_vpr_e(inst->sid, "%s: vb2_reqbufs failed, %d\n", rc);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_reqbufs);

int msm_vidc_qbuf(void *instance, struct media_device *mdev,
		struct v4l2_buffer *b)
{
	return 0;
}
EXPORT_SYMBOL(msm_vidc_qbuf);

int msm_vidc_dqbuf(void *instance, struct v4l2_buffer *b)
{
	return 0;
}
EXPORT_SYMBOL(msm_vidc_dqbuf);

int msm_vidc_streamon(void *instance, enum v4l2_buf_type i)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	int port;

	if (!inst)
		return -EINVAL;

	port = msm_vidc_get_port_from_type(i);
	if (port < 0) {
		d_vpr_e("%s: invalid buf type %d\n", __func__, i);
		return -EINVAL;
	}
	mutex_lock(&inst->lock);
	rc = vb2_streamon(&inst->vb2q[port], i);
	mutex_unlock(&inst->lock);
	if (rc)
		s_vpr_e(inst->sid, "%s: vb2_streamon failed, %d\n", rc);

	return rc;
}
EXPORT_SYMBOL(msm_vidc_streamon);

int msm_vidc_streamoff(void *instance, enum v4l2_buf_type i)
{
	int rc = 0;
	struct msm_vidc_inst *inst = instance;
	int port;

	if (!inst)
		return -EINVAL;

	port = msm_vidc_get_port_from_type(i);
	if (port < 0) {
		d_vpr_e("%s: invalid buf type %d\n", __func__, i);
		return -EINVAL;
	}
	mutex_lock(&inst->lock);
	rc = vb2_streamoff(&inst->vb2q[port], i);
	mutex_unlock(&inst->lock);
	if (rc)
		s_vpr_e(inst->sid, "%s: vb2_streamoff failed, %d\n", rc);

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
	return 0;
}
EXPORT_SYMBOL(msm_vidc_enum_framesizes);

int msm_vidc_subscribe_event(void *inst,
		const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !sub)
		return -EINVAL;

	rc = v4l2_event_subscribe(&vidc_inst->event_handler,
		sub, MAX_EVENTS, NULL);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_subscribe_event);

int msm_vidc_unsubscribe_event(void *inst,
		const struct v4l2_event_subscription *sub)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !sub)
		return -EINVAL;

	rc = v4l2_event_unsubscribe(&vidc_inst->event_handler, sub);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_unsubscribe_event);

int msm_vidc_dqevent(void *inst, struct v4l2_event *event)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst = (struct msm_vidc_inst *)inst;

	if (!inst || !event)
		return -EINVAL;

	rc = v4l2_event_dequeue(&vidc_inst->event_handler, event, false);
	return rc;
}
EXPORT_SYMBOL(msm_vidc_dqevent);

void *msm_vidc_open(void *vidc_core, u32 session_type)
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	struct msm_vidc_core *core;

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

	rc = msm_vidc_add_session(inst);
	if (rc) {
		d_vpr_e("%s: failed to get session id\n", __func__);
		return NULL;
	}

	s_vpr_i(inst->sid, "Opening video instance: %d\n", session_type);

	kref_init(&inst->kref);
	INIT_LIST_HEAD(&inst->buffers.input.list);
	INIT_LIST_HEAD(&inst->buffers.input_meta.list);
	INIT_LIST_HEAD(&inst->buffers.output.list);
	INIT_LIST_HEAD(&inst->buffers.output_meta.list);
	INIT_LIST_HEAD(&inst->buffers.scratch.list);
	INIT_LIST_HEAD(&inst->buffers.scratch_1.list);
	INIT_LIST_HEAD(&inst->buffers.scratch_2.list);
	INIT_LIST_HEAD(&inst->buffers.persist.list);
	INIT_LIST_HEAD(&inst->buffers.persist_1.list);
	inst->domain = session_type;
	inst->state = MSM_VIDC_OPEN;
	//inst->debugfs_root =
	//	msm_vidc_debugfs_init_inst(inst, core->debugfs_root);

	if (is_decode_session(inst)) {
		rc = msm_vdec_inst_init(inst);
		if (rc)
			goto error;
		rc = msm_vdec_ctrl_init(inst);
		if (rc)
			goto error;
	} else if (is_encode_session(inst)) {
		rc = msm_venc_inst_init(inst);
		if (rc)
			goto error;
		rc = msm_venc_ctrl_init(inst);
		if (rc)
			goto error;
	}

	rc = msm_vidc_queue_init(inst);
	if (rc)
		goto error;

	rc = msm_vidc_setup_event_queue(inst);
	if (rc)
		goto error;

	//msm_power_setup(inst);
	// send cmd to firmware here

	return inst;

error:
	msm_vidc_close(inst);
	return NULL;
}
EXPORT_SYMBOL(msm_vidc_open);

int msm_vidc_close(void *instance)
{
	return 0;
}
EXPORT_SYMBOL(msm_vidc_close);

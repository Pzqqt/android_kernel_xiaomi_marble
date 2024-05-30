// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_v4l2.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vidc.h"
#include "msm_vidc_events.h"

extern struct msm_vidc_core *g_core;

static struct msm_vidc_inst *get_vidc_inst(struct file *filp, void *fh)
{
	if (!filp || !filp->private_data)
		return NULL;
	return container_of(filp->private_data,
					struct msm_vidc_inst, event_handler);
}

unsigned int msm_v4l2_poll(struct file *filp, struct poll_table_struct *pt)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, NULL);

	return msm_vidc_poll((void *)inst, filp, pt);
}

int msm_v4l2_open(struct file *filp)
{
	struct video_device *vdev = video_devdata(filp);
	struct msm_video_device *vid_dev =
		container_of(vdev, struct msm_video_device, vdev);
	struct msm_vidc_core *core = video_drvdata(filp);
	struct msm_vidc_inst *inst;

	trace_msm_v4l2_vidc_open("START", NULL);
	inst = msm_vidc_open(core, vid_dev->type);
	if (!inst) {
		d_vpr_e("Failed to create instance, type = %d\n",
			vid_dev->type);
		trace_msm_v4l2_vidc_open("END", NULL);
		return -ENOMEM;
	}
	clear_bit(V4L2_FL_USES_V4L2_FH, &vdev->flags);
	filp->private_data = &(inst->event_handler);
	trace_msm_v4l2_vidc_open("END", inst);
	return 0;
}

int msm_v4l2_close(struct file *filp)
{
	int rc = 0;
	struct msm_vidc_inst *inst;

	inst = get_vidc_inst(filp, NULL);
	trace_msm_v4l2_vidc_close("START", inst);

	rc = msm_vidc_close(inst);
	filp->private_data = NULL;
	trace_msm_v4l2_vidc_close("END", NULL);
	return rc;
}

int msm_v4l2_querycap(struct file *filp, void *fh,
			struct v4l2_capability *cap)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_querycap((void *)inst, cap);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_enum_fmt(struct file *filp, void *fh,
					struct v4l2_fmtdesc *f)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_enum_fmt((void *)inst, f);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_try_fmt(struct file *filp, void *fh, struct v4l2_format *f)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_try_fmt((void *)inst, f);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_s_fmt(struct file *filp, void *fh,
					struct v4l2_format *f)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_s_fmt((void *)inst, f);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_g_fmt(struct file *filp, void *fh,
					struct v4l2_format *f)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_g_fmt((void *)inst, f);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_s_selection(struct file *filp, void *fh,
					struct v4l2_selection *s)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_s_selection((void *)inst, s);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_g_selection(struct file *filp, void *fh,
					struct v4l2_selection *s)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_g_selection((void *)inst, s);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_s_parm(struct file *filp, void *fh,
					struct v4l2_streamparm *a)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_s_param((void *)inst, a);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_g_parm(struct file *filp, void *fh,
					struct v4l2_streamparm *a)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_g_param((void *)inst, a);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_s_ctrl(struct file *filp, void *fh,
					struct v4l2_control *a)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_s_ctrl((void *)inst, a);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_g_ctrl(struct file *filp, void *fh,
					struct v4l2_control *a)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_g_ctrl((void *)inst, a);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_reqbufs(struct file *filp, void *fh,
				struct v4l2_requestbuffers *b)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_reqbufs((void *)inst, b);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_qbuf(struct file *filp, void *fh,
				struct v4l2_buffer *b)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	struct video_device *vdev = video_devdata(filp);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_qbuf(inst, vdev->v4l2_dev->mdev, b);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_dqbuf(struct file *filp, void *fh,
				struct v4l2_buffer *b)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_dqbuf(inst, b);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_streamon(struct file *filp, void *fh,
				enum v4l2_buf_type i)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_streamon((void *)inst, i);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_streamoff(struct file *filp, void *fh,
				enum v4l2_buf_type i)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_streamoff((void *)inst, i);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_subscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	struct msm_vidc_inst *inst;
	int rc = 0;

	inst = container_of(fh, struct msm_vidc_inst, event_handler);
	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_subscribe_event((void *)inst, sub);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_unsubscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	struct msm_vidc_inst *inst;
	int rc = 0;

	inst = container_of(fh, struct msm_vidc_inst, event_handler);
	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_unsubscribe_event((void *)inst, sub);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_decoder_cmd(struct file *filp, void *fh,
				struct v4l2_decoder_cmd *dec)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_cmd((void *)inst, (union msm_v4l2_cmd *)dec);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_encoder_cmd(struct file *filp, void *fh,
				struct v4l2_encoder_cmd *enc)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		rc = -EBUSY;
		goto unlock;
	}
	rc = msm_vidc_cmd((void *)inst, (union msm_v4l2_cmd *)enc);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_enum_framesizes(struct file *filp, void *fh,
				struct v4l2_frmsizeenum *fsize)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_enum_framesizes((void *)inst, fsize);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_enum_frameintervals(struct file *filp, void *fh,
				struct v4l2_frmivalenum *fival)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_enum_frameintervals((void *)inst, fival);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_queryctrl(struct file *filp, void *fh,
	struct v4l2_queryctrl *ctrl)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_query_ctrl((void *)inst, ctrl);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

int msm_v4l2_querymenu(struct file *filp, void *fh,
	struct v4l2_querymenu *qmenu)
{
	struct msm_vidc_inst *inst = get_vidc_inst(filp, fh);
	int rc = 0;

	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid instance\n", __func__);
		return -EINVAL;
	}

	inst_lock(inst, __func__);
	rc = msm_vidc_query_menu((void *)inst, qmenu);
	if (rc)
		goto unlock;

unlock:
	inst_unlock(inst, __func__);
	put_inst(inst);

	return rc;
}

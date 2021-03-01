// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_v4l2.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_debug.h"
#include "msm_vidc.h"

static struct msm_vidc_inst *get_vidc_inst(struct file *filp, void *fh)
{
	if (!filp || !filp->private_data)
		return NULL;
	return container_of(filp->private_data,
					struct msm_vidc_inst, event_handler);
}

unsigned int msm_v4l2_poll(struct file *filp, struct poll_table_struct *pt)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(filp, NULL);

	return msm_vidc_poll((void *)vidc_inst, filp, pt);
}

int msm_v4l2_open(struct file *filp)
{
	struct video_device *vdev = video_devdata(filp);
	struct msm_video_device *vid_dev =
		container_of(vdev, struct msm_video_device, vdev);
	struct msm_vidc_core *core = video_drvdata(filp);
	struct msm_vidc_inst *vidc_inst;

	vidc_inst = msm_vidc_open(core, vid_dev->type);
	if (!vidc_inst) {
		d_vpr_e("Failed to create instance, type = %d\n",
			vid_dev->type);
		return -ENOMEM;
	}
	clear_bit(V4L2_FL_USES_V4L2_FH, &vdev->flags);
	filp->private_data = &(vidc_inst->event_handler);
	return 0;
}

int msm_v4l2_close(struct file *filp)
{
	int rc = 0;
	struct msm_vidc_inst *vidc_inst;

	vidc_inst = get_vidc_inst(filp, NULL);

	rc = msm_vidc_close(vidc_inst);
	filp->private_data = NULL;
	return rc;
}

int msm_v4l2_querycap(struct file *filp, void *fh,
			struct v4l2_capability *cap)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(filp, fh);

	return msm_vidc_querycap((void *)vidc_inst, cap);
}

int msm_v4l2_enum_fmt(struct file *file, void *fh,
					struct v4l2_fmtdesc *f)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_enum_fmt((void *)vidc_inst, f);
}

int msm_v4l2_s_fmt(struct file *file, void *fh,
					struct v4l2_format *f)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_s_fmt((void *)vidc_inst, f);
}

int msm_v4l2_g_fmt(struct file *file, void *fh,
					struct v4l2_format *f)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_g_fmt((void *)vidc_inst, f);
}

int msm_v4l2_s_selection(struct file* file, void* fh,
					struct v4l2_selection* s)
{
	struct msm_vidc_inst* vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_s_selection((void*)vidc_inst, s);
}

int msm_v4l2_g_selection(struct file* file, void* fh,
					struct v4l2_selection* s)
{
	struct msm_vidc_inst* vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_g_selection((void*)vidc_inst, s);
}

int msm_v4l2_s_parm(struct file *file, void *fh,
					struct v4l2_streamparm *a)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_s_param((void *)vidc_inst, a);
}

int msm_v4l2_g_parm(struct file *file, void *fh,
					struct v4l2_streamparm *a)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_g_param((void *)vidc_inst, a);
}

int msm_v4l2_s_ctrl(struct file *file, void *fh,
					struct v4l2_control *a)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_s_ctrl((void *)vidc_inst, a);
}

int msm_v4l2_g_ctrl(struct file *file, void *fh,
					struct v4l2_control *a)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_g_ctrl((void *)vidc_inst, a);
}

int msm_v4l2_reqbufs(struct file *file, void *fh,
				struct v4l2_requestbuffers *b)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_reqbufs((void *)vidc_inst, b);
}

int msm_v4l2_qbuf(struct file *file, void *fh,
				struct v4l2_buffer *b)
{
	struct video_device *vdev = video_devdata(file);
	return msm_vidc_qbuf(get_vidc_inst(file, fh), vdev->v4l2_dev->mdev, b);
}

int msm_v4l2_dqbuf(struct file *file, void *fh,
				struct v4l2_buffer *b)
{
	return msm_vidc_dqbuf(get_vidc_inst(file, fh), b);
}

int msm_v4l2_streamon(struct file *file, void *fh,
				enum v4l2_buf_type i)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_streamon((void *)vidc_inst, i);
}

int msm_v4l2_streamoff(struct file *file, void *fh,
				enum v4l2_buf_type i)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_streamoff((void *)vidc_inst, i);
}

int msm_v4l2_subscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	struct msm_vidc_inst *vidc_inst = container_of(fh,
			struct msm_vidc_inst, event_handler);

	return msm_vidc_subscribe_event((void *)vidc_inst, sub);
}

int msm_v4l2_unsubscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	struct msm_vidc_inst *vidc_inst = container_of(fh,
			struct msm_vidc_inst, event_handler);

	return msm_vidc_unsubscribe_event((void *)vidc_inst, sub);
}

int msm_v4l2_decoder_cmd(struct file *file, void *fh,
				struct v4l2_decoder_cmd *dec)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_cmd((void *)vidc_inst, (union msm_v4l2_cmd *)dec);
}

int msm_v4l2_encoder_cmd(struct file *file, void *fh,
				struct v4l2_encoder_cmd *enc)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_cmd((void *)vidc_inst, (union msm_v4l2_cmd *)enc);
}

int msm_v4l2_enum_framesizes(struct file *file, void *fh,
				struct v4l2_frmsizeenum *fsize)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_enum_framesizes((void *)vidc_inst, fsize);
}

int msm_v4l2_queryctrl(struct file *file, void *fh,
	struct v4l2_queryctrl *ctrl)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_query_ctrl((void *)vidc_inst, ctrl);
}

int msm_v4l2_querymenu(struct file *file, void *fh,
	struct v4l2_querymenu *qmenu)
{
	struct msm_vidc_inst *vidc_inst = get_vidc_inst(file, fh);

	return msm_vidc_query_menu((void *)vidc_inst, qmenu);
}

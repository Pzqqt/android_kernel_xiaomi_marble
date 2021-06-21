/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_V4L2_H_
#define _MSM_VIDC_V4L2_H_

#include <linux/poll.h>
#include <linux/fs.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>

int msm_v4l2_open(struct file *filp);
int msm_v4l2_close(struct file *filp);
int msm_v4l2_querycap(struct file *filp, void *fh,
		struct v4l2_capability *cap);
int msm_v4l2_enum_fmt(struct file *file, void *fh,
		struct v4l2_fmtdesc *f);
int msm_v4l2_try_fmt(struct file *file, void *fh,
		struct v4l2_format *f);
int msm_v4l2_s_fmt(struct file *file, void *fh,
		struct v4l2_format *f);
int msm_v4l2_g_fmt(struct file *file, void *fh,
		struct v4l2_format *f);
int msm_v4l2_s_selection(struct file* file, void* fh,
		struct v4l2_selection* s);
int msm_v4l2_g_selection(struct file* file, void* fh,
		struct v4l2_selection* s);
int msm_v4l2_s_parm(struct file *file, void *fh,
		struct v4l2_streamparm *a);
int msm_v4l2_g_parm(struct file *file, void *fh,
		struct v4l2_streamparm *a);
int msm_v4l2_s_ctrl(struct file *file, void *fh,
		struct v4l2_control *a);
int msm_v4l2_g_ctrl(struct file *file, void *fh,
		struct v4l2_control *a);
int msm_v4l2_reqbufs(struct file *file, void *fh,
		struct v4l2_requestbuffers *b);
int msm_v4l2_qbuf(struct file *file, void *fh,
		struct v4l2_buffer *b);
int msm_v4l2_dqbuf(struct file *file, void *fh,
		struct v4l2_buffer *b);
int msm_v4l2_streamon(struct file *file, void *fh,
		enum v4l2_buf_type i);
int msm_v4l2_streamoff(struct file *file, void *fh,
		enum v4l2_buf_type i);
int msm_v4l2_subscribe_event(struct v4l2_fh *fh,
		const struct v4l2_event_subscription *sub);
int msm_v4l2_unsubscribe_event(struct v4l2_fh *fh,
		const struct v4l2_event_subscription *sub);
int msm_v4l2_decoder_cmd(struct file *file, void *fh,
		struct v4l2_decoder_cmd *dec);
int msm_v4l2_encoder_cmd(struct file *file, void *fh,
		struct v4l2_encoder_cmd *enc);
int msm_v4l2_enum_framesizes(struct file *file, void *fh,
		struct v4l2_frmsizeenum *fsize);
int msm_v4l2_enum_frameintervals(struct file *file, void *fh,
		struct v4l2_frmivalenum *fival);
int msm_v4l2_queryctrl(struct file *file, void *fh,
		struct v4l2_queryctrl *ctrl);
int msm_v4l2_querymenu(struct file *file, void *fh,
		struct v4l2_querymenu *qmenu);
unsigned int msm_v4l2_poll(struct file *filp,
	struct poll_table_struct *pt);

#endif // _MSM_VIDC_V4L2_H_

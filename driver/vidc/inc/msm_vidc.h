/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_H_
#define _MSM_VIDC_H_

#include <linux/videodev2.h>
#include <media/media-device.h>

union msm_v4l2_cmd {
	struct v4l2_decoder_cmd dec;
	struct v4l2_encoder_cmd enc;
};

void *msm_vidc_open(void *core, u32 session_type);
int msm_vidc_close(void *instance);
int msm_vidc_querycap(void *instance, struct v4l2_capability *cap);
int msm_vidc_enum_fmt(void *instance, struct v4l2_fmtdesc *f);
int msm_vidc_try_fmt(void *instance, struct v4l2_format *f);
int msm_vidc_s_fmt(void *instance, struct v4l2_format *f);
int msm_vidc_g_fmt(void *instance, struct v4l2_format *f);
int msm_vidc_s_selection(void* instance, struct v4l2_selection* s);
int msm_vidc_g_selection(void* instance, struct v4l2_selection* s);
int msm_vidc_s_param(void *instance, struct v4l2_streamparm *sp);
int msm_vidc_g_param(void *instance, struct v4l2_streamparm *sp);
int msm_vidc_s_ctrl(void *instance, struct v4l2_control *a);
int msm_vidc_s_ext_ctrl(void *instance, struct v4l2_ext_controls *a);
int msm_vidc_g_ext_ctrl(void *instance, struct v4l2_ext_controls *a);
int msm_vidc_g_ctrl(void *instance, struct v4l2_control *a);
int msm_vidc_reqbufs(void *instance, struct v4l2_requestbuffers *b);
int msm_vidc_release_buffer(void *instance, int buffer_type,
		unsigned int buffer_index);
int msm_vidc_qbuf(void *instance, struct media_device *mdev,
		struct v4l2_buffer *b);
int msm_vidc_dqbuf(void *instance, struct v4l2_buffer *b);
int msm_vidc_streamon(void *instance, enum v4l2_buf_type i);
int msm_vidc_query_ctrl(void *instance, struct v4l2_queryctrl *ctrl);
int msm_vidc_query_menu(void *instance, struct v4l2_querymenu *qmenu);
int msm_vidc_streamoff(void *instance, enum v4l2_buf_type i);
int msm_vidc_cmd(void *instance, union msm_v4l2_cmd *cmd);
int msm_vidc_poll(void *instance, struct file *filp,
		struct poll_table_struct *pt);
int msm_vidc_subscribe_event(void *instance,
		const struct v4l2_event_subscription *sub);
int msm_vidc_unsubscribe_event(void *instance,
		const struct v4l2_event_subscription *sub);
int msm_vidc_dqevent(void *instance, struct v4l2_event *event);
int msm_vidc_g_crop(void *instance, struct v4l2_crop *a);
int msm_vidc_enum_framesizes(void *instance, struct v4l2_frmsizeenum *fsize);
int msm_vidc_enum_frameintervals(void *instance, struct v4l2_frmivalenum *fival);
#endif

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __V4l2_VIDC_EXTENSIONS_H__
#define __V4l2_VIDC_EXTENSIONS_H__

#include <linux/types.h>
#include <linux/v4l2-controls.h>

/* vendor color format start */
/* UBWC 8-bit Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_NV12_UBWC                  v4l2_fourcc('Q', '1', '2', '8')
/* NV12_512 8-bit Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_NV12_512                   v4l2_fourcc('Q', '5', '1', '2')
/* NV12 10-bit Y/CbCr 4:2:0 */
#define V4L2_PIX_FMT_NV12_P010                  v4l2_fourcc('Q', 'P', '1', '0')
/* UBWC 10-bit Y/CbCr 4:2:0 */
#define V4L2_PIX_FMT_NV12_TP10_UBWC             v4l2_fourcc('Q', 'T', 'P', '0')
#define V4L2_PIX_FMT_RGBA8888_UBWC              v4l2_fourcc('Q', 'R', 'G', 'B')
/* Y/CbCr 4:2:0 P10 Venus */
#define V4L2_PIX_FMT_VIDC_META                  v4l2_fourcc('Q', 'M', 'E', 'T')
/* vendor color format end */

/* vendor controls start */
#define V4L2_CID_MPEG_MSM_VIDC_BASE             (V4L2_CTRL_CLASS_MPEG | 0x2000)

#define V4L2_CID_MPEG_VIDC_VIDEO_DECODE_ORDER                                 \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x1)
#define V4L2_CID_MPEG_VIDC_VIDEO_SYNC_FRAME_DECODE                            \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x2)
#define V4L2_CID_MPEG_VIDC_VIDEO_SECURE                                       \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x3)
#define V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE                              \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x4)
#define V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_HINT                              \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x5)
#define V4L2_CID_MPEG_VIDC_VIDEO_BUFFER_SIZE_LIMIT                            \
		(V4L2_CID_MPEG_MSM_VIDC_BASE + 0x6)

/* vendor controls end */

#endif

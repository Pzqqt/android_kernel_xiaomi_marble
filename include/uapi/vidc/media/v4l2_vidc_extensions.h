/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __V4l2_VIDC_EXTENSIONS_H__
#define __V4l2_VIDC_EXTENSIONS_H__

#include <linux/types.h>
#include <linux/v4l2-controls.h>

/*
 * supported standard color formats
 * V4L2_PIX_FMT_NV12   Y/CbCr 4:2:0
 * NV21?? // TODO: jdas
 * V4L2_PIX_FMT_ARGB32  ARGB-8-8-8-8
 */
 /* Below are additional color formats */
/* 12  Y/CbCr 4:2:0  compressed */
#define V4L2_PIX_FMT_VIDC_NV12C                 v4l2_fourcc('Q', '1', '2', 'C')
/* Y/CbCr 4:2:0, 10 bits per channel compressed */
#define V4L2_PIX_FMT_VIDC_TP10C                 v4l2_fourcc('Q', '1', '0', 'C')
/* Y/CbCr 4:2:0, 10 bits per channel */
#define V4L2_PIX_FMT_VIDC_P010                  v4l2_fourcc('P', '0', '1', '0')
/* 32  ARGB-8-8-8-8 compressed */
#define V4L2_PIX_FMT_VIDC_ARGB32C               v4l2_fourcc('Q', '2', '4', 'C')
#define V4L2_META_FMT_VIDC                      v4l2_fourcc('Q', 'M', 'E', 'T')

/* vendor controls start */
#define V4L2_CID_MPEG_VIDC_BASE                 (V4L2_CTRL_CLASS_MPEG | 0x2000)

#define V4L2_MPEG_MSM_VIDC_DISABLE 0
#define V4L2_MPEG_MSM_VIDC_ENABLE 1

#define V4L2_CID_MPEG_VIDC_SECURE               (V4L2_CID_MPEG_VIDC_BASE + 0x1)
/* HEIC encoder and decoder */
#define V4L2_CID_MPEG_VIDC_HEIC                 (V4L2_CID_MPEG_VIDC_BASE + 0x2)
#define V4L2_CID_MPEG_VIDC_LOWLATENCY_REQUEST   (V4L2_CID_MPEG_VIDC_BASE + 0x3)
/* FIXme: */
#define V4L2_CID_MPEG_VIDC_CODEC_CONFIG         (V4L2_CID_MPEG_VIDC_BASE + 0x4)
/* B frame min/max qp */
#define V4L2_CID_MPEG_VIDC_B_FRAME_MIN_QP       (V4L2_CID_MPEG_VIDC_BASE + 0x5)
#define V4L2_CID_MPEG_VIDC_B_FRAME_MAX_QP       (V4L2_CID_MPEG_VIDC_BASE + 0x6)
/* LTR controls */
#define V4L2_CID_MPEG_VIDC_LTRCOUNT             (V4L2_CID_MPEG_VIDC_BASE + 0x7)
#define V4L2_CID_MPEG_VIDC_USELTRFRAME          (V4L2_CID_MPEG_VIDC_BASE + 0x8)
#define V4L2_CID_MPEG_VIDC_MARKLTRFRAME         (V4L2_CID_MPEG_VIDC_BASE + 0x9)
/* Base layer priority id value */
#define V4L2_CID_MPEG_VIDC_BASELAYER_PRIORITY   (V4L2_CID_MPEG_VIDC_BASE + 0xA)
/* Encoder Intra refresh period */
#define V4L2_CID_MPEG_VIDC_INTRA_REFRESH_PERIOD (V4L2_CID_MPEG_VIDC_BASE + 0xB)
#define V4L2_CID_MPEG_VIDC_AU_DELIMITER         (V4L2_CID_MPEG_VIDC_BASE + 0xC)
#define V4L2_CID_MPEG_VIDC_TIME_DELTA_BASED_RC  (V4L2_CID_MPEG_VIDC_BASE + 0xD)
/* Encoder quality controls */
#define V4L2_CID_MPEG_VIDC_CONTENT_ADAPTIVE_CODING                            \
	(V4L2_CID_MPEG_VIDC_BASE + 0xE)
#define V4L2_CID_MPEG_VIDC_QUALITY_BITRATE_BOOST                              \
	(V4L2_CID_MPEG_VIDC_BASE + 0xF)
#define V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES                                   \
	(V4L2_CID_MPEG_VIDC_BASE + 0x10)
enum v4l2_mpeg_vidc_blur_types {
	VIDC_BLUR_NONE               = 0x0,
	VIDC_BLUR_EXTERNAL           = 0x1,
	VIDC_BLUR_ADAPTIVE           = 0x2,
};
/* (blur width) << 16 | (blur height) */
#define V4L2_CID_MPEG_VIDC_VIDEO_BLUR_RESOLUTION                              \
	(V4L2_CID_MPEG_VIDC_BASE + 0x11)
/* TODO: jdas: compound control for matrix */
#define V4L2_CID_MPEG_VIDC_VIDEO_VPE_CSC_CUSTOM_MATRIX                        \
	(V4L2_CID_MPEG_VIDC_BASE + 0x12)

/* various Metadata - encoder & decoder */
#define V4L2_CID_MPEG_VIDC_METADATA_ENABLE                                    \
	(V4L2_CID_MPEG_VIDC_BASE + 0x13)
#define V4L2_CID_MPEG_VIDC_METADATA_DISABLE                                   \
	(V4L2_CID_MPEG_VIDC_BASE + 0x14)
enum v4l2_mpeg_vidc_metapayload_header_flags {
	METADATA_FLAGS_NONE             = 0,
	METADATA_FLAGS_TOP_FIELD        = (1 << 0),
	METADATA_FLAGS_BOTTOM_FIELD     = (1 << 1),
};
struct msm_vidc_metabuf_header {
	__u32 count;
	__u32 size;
	__u32 version;
	__u32 reserved[5];
};
struct msm_vidc_metapayload_header {
	__u32 type;
	__u32 size;
	__u32 version;
	__u32 offset;
	__u32 flags;
	__u32 reserved[3];
};
enum v4l2_mpeg_vidc_metadata {
	METADATA_LTR_MARK_USE_DETAILS         = 0x03000137,
	METADATA_METADATA_SEQ_HEADER_NAL      = 0x0300014a,
	METADATA_DPB_LUMA_CHROMA_MISR         = 0x03000153,
	METADATA_OPB_LUMA_CHROMA_MISR         = 0x03000154,
	METADATA_INTERLACE                    = 0x03000156,
	METADATA_CONEALED_MB_COUNT            = 0x0300015f,
	METADATA_HISTOGRAM_INFO               = 0x03000161,
	METADATA_SEI_MASTERING_DISPLAY_COLOUR = 0x03000163,
	METADATA_SEI_CONTENT_LIGHT_LEVEL      = 0x03000164,
	METADATA_HDR10PLUS                    = 0x03000165,
	METADATA_EVA_STATS                    = 0x03000167,
	METADATA_BUFFER_TAG                   = 0x0300016b,
	METADATA_SUBFRAME_OUTPUT              = 0x0300016d,
	METADATA_ROI_INFO                     = 0x03000173,
};

/* vendor controls end */

#endif

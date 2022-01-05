/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __V4l2_VIDC_EXTENSIONS_H__
#define __V4l2_VIDC_EXTENSIONS_H__

#include <linux/types.h>
#include <linux/v4l2-controls.h>

/*
 * supported standard color formats
 * V4L2_PIX_FMT_NV12   Y/CbCr 4:2:0
 * V4L2_PIX_FMT_RGBA32  RGBA-8-8-8-8
 */
 /* Below are additional color formats */
/* 12  Y/CbCr 4:2:0  compressed */
#define V4L2_PIX_FMT_VIDC_NV12C                 v4l2_fourcc('Q', '1', '2', 'C')
/* Y/CbCr 4:2:0, 10 bits per channel compressed */
#define V4L2_PIX_FMT_VIDC_TP10C                 v4l2_fourcc('Q', '1', '0', 'C')
/* Y/CbCr 4:2:0, 10 bits per channel */
#define V4L2_PIX_FMT_VIDC_P010                  v4l2_fourcc('P', '0', '1', '0')
/* 32  RGBA-8-8-8-8 compressed */
#define V4L2_PIX_FMT_VIDC_ARGB32C               v4l2_fourcc('Q', '2', '4', 'C')
#define V4L2_META_FMT_VIDC                      v4l2_fourcc('Q', 'M', 'E', 'T')
/* HEIC encoder and decoder */
#define V4L2_PIX_FMT_HEIC                       v4l2_fourcc('H', 'E', 'I', 'C')

/* start of vidc specific colorspace definitions */
#define V4L2_COLORSPACE_VIDC_GENERIC_FILM    101
#define V4L2_COLORSPACE_VIDC_EG431           102
#define V4L2_COLORSPACE_VIDC_EBU_TECH        103

#define V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_M   201
#define V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_BG  202
#define V4L2_XFER_FUNC_VIDC_BT601_525_OR_625 203
#define V4L2_XFER_FUNC_VIDC_LINEAR           204
#define V4L2_XFER_FUNC_VIDC_XVYCC            205
#define V4L2_XFER_FUNC_VIDC_BT1361           206
#define V4L2_XFER_FUNC_VIDC_BT2020           207
#define V4L2_XFER_FUNC_VIDC_ST428            208
#define V4L2_XFER_FUNC_VIDC_HLG              209

/* should be 255 or below due to u8 limitation */
#define V4L2_YCBCR_VIDC_SRGB_OR_SMPTE_ST428  241
#define V4L2_YCBCR_VIDC_FCC47_73_682         242

/* end of vidc specific colorspace definitions */
#ifndef V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10_STILL_PICTURE
#define V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10_STILL_PICTURE    (3)
#endif

/* vendor controls start */
#define V4L2_CID_MPEG_VIDC_BASE                 (V4L2_CTRL_CLASS_MPEG | 0x2000)

#define V4L2_MPEG_MSM_VIDC_DISABLE 0
#define V4L2_MPEG_MSM_VIDC_ENABLE 1

#define V4L2_CID_MPEG_VIDC_SECURE               (V4L2_CID_MPEG_VIDC_BASE + 0x1)
#define V4L2_CID_MPEG_VIDC_LOWLATENCY_REQUEST   (V4L2_CID_MPEG_VIDC_BASE + 0x3)
/* FIXme: */
#define V4L2_CID_MPEG_VIDC_CODEC_CONFIG         (V4L2_CID_MPEG_VIDC_BASE + 0x4)
/* Encoder Intra refresh period */
#define V4L2_CID_MPEG_VIDC_INTRA_REFRESH_PERIOD (V4L2_CID_MPEG_VIDC_BASE + 0xB)
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
#define V4L2_CID_MPEG_VIDC_METADATA_LTR_MARK_USE_DETAILS                      \
	(V4L2_CID_MPEG_VIDC_BASE + 0x13)
#define V4L2_CID_MPEG_VIDC_METADATA_SEQ_HEADER_NAL                            \
	(V4L2_CID_MPEG_VIDC_BASE + 0x14)
#define V4L2_CID_MPEG_VIDC_METADATA_DPB_LUMA_CHROMA_MISR                      \
	(V4L2_CID_MPEG_VIDC_BASE + 0x15)
#define V4L2_CID_MPEG_VIDC_METADATA_OPB_LUMA_CHROMA_MISR                      \
	(V4L2_CID_MPEG_VIDC_BASE + 0x16)
#define V4L2_CID_MPEG_VIDC_METADATA_INTERLACE                                 \
	(V4L2_CID_MPEG_VIDC_BASE + 0x17)
#define V4L2_CID_MPEG_VIDC_METADATA_CONCEALED_MB_COUNT                        \
	(V4L2_CID_MPEG_VIDC_BASE + 0x18)
#define V4L2_CID_MPEG_VIDC_METADATA_HISTOGRAM_INFO                            \
	(V4L2_CID_MPEG_VIDC_BASE + 0x19)
#define V4L2_CID_MPEG_VIDC_METADATA_SEI_MASTERING_DISPLAY_COLOUR              \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1A)
#define V4L2_CID_MPEG_VIDC_METADATA_SEI_CONTENT_LIGHT_LEVEL                   \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1B)
#define V4L2_CID_MPEG_VIDC_METADATA_HDR10PLUS                                 \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1C)
#define V4L2_CID_MPEG_VIDC_METADATA_EVA_STATS                                 \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1D)
#define V4L2_CID_MPEG_VIDC_METADATA_BUFFER_TAG                                \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1E)
#define V4L2_CID_MPEG_VIDC_METADATA_SUBFRAME_OUTPUT                           \
	(V4L2_CID_MPEG_VIDC_BASE + 0x1F)
#define V4L2_CID_MPEG_VIDC_METADATA_ROI_INFO                                  \
	(V4L2_CID_MPEG_VIDC_BASE + 0x20)
#define V4L2_CID_MPEG_VIDC_METADATA_TIMESTAMP                                 \
	(V4L2_CID_MPEG_VIDC_BASE + 0x21)
#define V4L2_CID_MPEG_VIDC_METADATA_ENC_QP_METADATA                           \
	(V4L2_CID_MPEG_VIDC_BASE + 0x22)
#define V4L2_CID_MPEG_VIDC_MIN_BITSTREAM_SIZE_OVERWRITE                       \
	(V4L2_CID_MPEG_VIDC_BASE + 0x23)

/* Encoder Super frame control */
#define V4L2_CID_MPEG_VIDC_SUPERFRAME           (V4L2_CID_MPEG_VIDC_BASE + 0x28)
/* Thumbnail Mode control */
#define V4L2_CID_MPEG_VIDC_THUMBNAIL_MODE       (V4L2_CID_MPEG_VIDC_BASE + 0x29)
/* Priority control */
#define V4L2_CID_MPEG_VIDC_PRIORITY             (V4L2_CID_MPEG_VIDC_BASE + 0x2A)
/* Metadata DPB Tag List*/
#define V4L2_CID_MPEG_VIDC_METADATA_DPB_TAG_LIST                             \
	(V4L2_CID_MPEG_VIDC_BASE + 0x2B)
/* Encoder Input Compression Ratio control */
#define V4L2_CID_MPEG_VIDC_ENC_INPUT_COMPRESSION_RATIO                       \
	(V4L2_CID_MPEG_VIDC_BASE + 0x2C)
#define V4L2_CID_MPEG_VIDC_METADATA_DEC_QP_METADATA                           \
	(V4L2_CID_MPEG_VIDC_BASE + 0x2E)
/* Encoder Complexity control */
#define V4L2_CID_MPEG_VIDC_VENC_COMPLEXITY                                   \
	(V4L2_CID_MPEG_VIDC_BASE + 0x2F)
/* Decoder Max Number of Reorder Frames */
#define V4L2_CID_MPEG_VIDC_METADATA_MAX_NUM_REORDER_FRAMES                   \
	(V4L2_CID_MPEG_VIDC_BASE + 0x30)
/* Decoder Timestamp Reorder control */
#define V4L2_CID_MPEG_VIDC_TS_REORDER           (V4L2_CID_MPEG_VIDC_BASE + 0x31)

/* Deprecate below controls once availble in gki and gsi bionic header */
#ifndef V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID
#define V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID                            \
	(V4L2_CID_MPEG_BASE + 230)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_AU_DELIMITER
#define V4L2_CID_MPEG_VIDEO_AU_DELIMITER                                     \
	(V4L2_CID_MPEG_BASE + 231)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_LTR_COUNT
#define V4L2_CID_MPEG_VIDEO_LTR_COUNT                                        \
	(V4L2_CID_MPEG_BASE + 232)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX
#define V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX                                  \
	(V4L2_CID_MPEG_BASE + 233)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES
#define V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES                                   \
	(V4L2_CID_MPEG_BASE + 234)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP                              \
	(V4L2_CID_MPEG_BASE + 389)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP                              \
	(V4L2_CID_MPEG_BASE + 390)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR                           \
	(V4L2_CID_MPEG_BASE + 391)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR                           \
	(V4L2_CID_MPEG_BASE + 392)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR                           \
	(V4L2_CID_MPEG_BASE + 393)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR                           \
	(V4L2_CID_MPEG_BASE + 394)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR                           \
	(V4L2_CID_MPEG_BASE + 395)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR                           \
	(V4L2_CID_MPEG_BASE + 396)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR
#define V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR                           \
	(V4L2_CID_MPEG_BASE + 397)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP                              \
	(V4L2_CID_MPEG_BASE + 647)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP                              \
	(V4L2_CID_MPEG_BASE + 648)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP                              \
	(V4L2_CID_MPEG_BASE + 649)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP                              \
	(V4L2_CID_MPEG_BASE + 650)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP                              \
	(V4L2_CID_MPEG_BASE + 651)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP                              \
	(V4L2_CID_MPEG_BASE + 652)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY
#define V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY                                \
	(V4L2_CID_MPEG_BASE + 653)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE
#define V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE                         \
	(V4L2_CID_MPEG_BASE + 654)
#endif

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
	METADATA_SEQ_HEADER_NAL               = 0x0300014a,
	METADATA_DPB_LUMA_CHROMA_MISR         = 0x03000153,
	METADATA_OPB_LUMA_CHROMA_MISR         = 0x03000154,
	METADATA_INTERLACE                    = 0x03000156,
	METADATA_TIMESTAMP                    = 0x0300015c,
	METADATA_CONCEALED_MB_COUNT           = 0x0300015f,
	METADATA_HISTOGRAM_INFO               = 0x03000161,
	METADATA_SEI_MASTERING_DISPLAY_COLOUR = 0x03000163,
	METADATA_SEI_CONTENT_LIGHT_LEVEL      = 0x03000164,
	METADATA_HDR10PLUS                    = 0x03000165,
	METADATA_EVA_STATS                    = 0x03000167,
	METADATA_BUFFER_TAG                   = 0x0300016b,
	METADATA_SUBFRAME_OUTPUT              = 0x0300016d,
	METADATA_ENC_QP_METADATA              = 0x0300016e,
	METADATA_DEC_QP_METADATA              = 0x0300016f,
	METADATA_ROI_INFO                     = 0x03000173,
	METADATA_DPB_TAG_LIST                 = 0x03000179,
	METADATA_MAX_NUM_REORDER_FRAMES       = 0x03000127,
};
enum meta_interlace_info {
	META_INTERLACE_INFO_NONE                            = 0x00000000,
	META_INTERLACE_FRAME_PROGRESSIVE                    = 0x00000001,
	META_INTERLACE_FRAME_MBAFF                          = 0x00000002,
	META_INTERLACE_FRAME_INTERLEAVE_TOPFIELD_FIRST      = 0x00000004,
	META_INTERLACE_FRAME_INTERLEAVE_BOTTOMFIELD_FIRST   = 0x00000008,
	META_INTERLACE_FRAME_INTERLACE_TOPFIELD_FIRST       = 0x00000010,
	META_INTERLACE_FRAME_INTERLACE_BOTTOMFIELD_FIRST    = 0x00000020,
};

/* vendor controls end */

#endif

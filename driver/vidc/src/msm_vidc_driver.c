// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/iommu.h>
#include <linux/workqueue.h>
#include <media/v4l2_vidc_extensions.h>
#include "msm_media_info.h"

#include "msm_vidc_driver.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_control.h"
#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_power.h"
#include "msm_vidc.h"
#include "msm_vdec.h"
#include "msm_venc.h"
#include "venus_hfi.h"
#include "venus_hfi_response.h"
#include "hfi_packet.h"
extern struct msm_vidc_core *g_core;

#define COUNT_BITS(a, out) {       \
	while ((a) >= 1) {          \
		(out) += (a) & (1); \
		(a) >>= (1);        \
	}                           \
}

#define SSR_TYPE 0x0000000F
#define SSR_TYPE_SHIFT 0
#define SSR_SUB_CLIENT_ID 0x000000F0
#define SSR_SUB_CLIENT_ID_SHIFT 4
#define SSR_ADDR_ID 0xFFFFFFFF00000000
#define SSR_ADDR_SHIFT 32

struct msm_vidc_buf_type_name {
	enum msm_vidc_buffer_type type;
	char *name;
};

struct msm_vidc_cap_name {
	enum msm_vidc_inst_capability_type cap;
	char *name;
};

static const struct msm_vidc_cap_name cap_name_arr[] = {
	{INST_CAP_NONE,                  "INST_CAP_NONE"              },
	{FRAME_WIDTH,                    "FRAME_WIDTH"                },
	{LOSSLESS_FRAME_WIDTH,           "LOSSLESS_FRAME_WIDTH"       },
	{SECURE_FRAME_WIDTH,             "SECURE_FRAME_WIDTH"         },
	{FRAME_HEIGHT,                   "FRAME_HEIGHT"               },
	{LOSSLESS_FRAME_HEIGHT,          "LOSSLESS_FRAME_HEIGHT"      },
	{SECURE_FRAME_HEIGHT,            "SECURE_FRAME_HEIGHT"        },
	{PIX_FMTS,                       "PIX_FMTS"                   },
	{MIN_BUFFERS_INPUT,              "MIN_BUFFERS_INPUT"          },
	{MIN_BUFFERS_OUTPUT,             "MIN_BUFFERS_OUTPUT"         },
	{MBPF,                           "MBPF"                       },
	{LOSSLESS_MBPF,                  "LOSSLESS_MBPF"              },
	{BATCH_MBPF,                     "BATCH_MBPF"                 },
	{BATCH_FPS,                      "BATCH_FPS"                  },
	{SECURE_MBPF,                    "SECURE_MBPF"                },
	{MBPS,                           "MBPS"                       },
	{POWER_SAVE_MBPS,                "POWER_SAVE_MBPS"            },
	{FRAME_RATE,                     "FRAME_RATE"                 },
	{OPERATING_RATE,                 "OPERATING_RATE"             },
	{SCALE_X,                        "SCALE_X"                    },
	{SCALE_Y,                        "SCALE_Y"                    },
	{MB_CYCLES_VSP,                  "MB_CYCLES_VSP"              },
	{MB_CYCLES_VPP,                  "MB_CYCLES_VPP"              },
	{MB_CYCLES_LP,                   "MB_CYCLES_LP"               },
	{MB_CYCLES_FW,                   "MB_CYCLES_FW"               },
	{MB_CYCLES_FW_VPP,               "MB_CYCLES_FW_VPP"           },
	{SECURE_MODE,                    "SECURE_MODE"                },
	{HFLIP,                          "HFLIP"                      },
	{VFLIP,                          "VFLIP"                      },
	{ROTATION,                       "ROTATION"                   },
	{SUPER_FRAME,                    "SUPER_FRAME"                },
	{SLICE_INTERFACE,                "SLICE_INTERFACE"            },
	{HEADER_MODE,                    "HEADER_MODE"                },
	{PREPEND_SPSPPS_TO_IDR,          "PREPEND_SPSPPS_TO_IDR"      },
	{META_SEQ_HDR_NAL,               "META_SEQ_HDR_NAL"           },
	{WITHOUT_STARTCODE,              "WITHOUT_STARTCODE"          },
	{NAL_LENGTH_FIELD,               "NAL_LENGTH_FIELD"           },
	{REQUEST_I_FRAME,                "REQUEST_I_FRAME"            },
	{BIT_RATE,                       "BIT_RATE"                   },
	{BITRATE_MODE,                   "BITRATE_MODE"               },
	{LOSSLESS,                       "LOSSLESS"                   },
	{FRAME_SKIP_MODE,                "FRAME_SKIP_MODE"            },
	{FRAME_RC_ENABLE,                "FRAME_RC_ENABLE"            },
	{CONSTANT_QUALITY,               "CONSTANT_QUALITY"           },
	{GOP_SIZE,                       "GOP_SIZE"                   },
	{GOP_CLOSURE,                    "GOP_CLOSURE"                },
	{B_FRAME,                        "B_FRAME"                    },
	{BLUR_TYPES,                     "BLUR_TYPES"                 },
	{BLUR_RESOLUTION,                "BLUR_RESOLUTION"            },
	{CSC,                            "CSC"                        },
	{CSC_CUSTOM_MATRIX,              "CSC_CUSTOM_MATRIX"          },
	{GRID,                           "GRID"                       },
	{LOWLATENCY_MODE,                "LOWLATENCY_MODE"            },
	{LTR_COUNT,                      "LTR_COUNT"                  },
	{USE_LTR,                        "USE_LTR"                    },
	{MARK_LTR,                       "MARK_LTR"                   },
	{BASELAYER_PRIORITY,             "BASELAYER_PRIORITY"         },
	{IR_RANDOM,                      "IR_RANDOM"                  },
	{AU_DELIMITER,                   "AU_DELIMITER"               },
	{TIME_DELTA_BASED_RC,            "TIME_DELTA_BASED_RC"        },
	{CONTENT_ADAPTIVE_CODING,        "CONTENT_ADAPTIVE_CODING"    },
	{BITRATE_BOOST,                  "BITRATE_BOOST"              },
	{MIN_QUALITY,                    "MIN_QUALITY"                },
	{VBV_DELAY,                      "VBV_DELAY"                  },
	{PEAK_BITRATE,                   "PEAK_BITRATE"               },
	{MIN_FRAME_QP,                   "MIN_FRAME_QP"               },
	{I_FRAME_MIN_QP,                 "I_FRAME_MIN_QP"             },
	{P_FRAME_MIN_QP,                 "P_FRAME_MIN_QP"             },
	{B_FRAME_MIN_QP,                 "B_FRAME_MIN_QP"             },
	{MAX_FRAME_QP,                   "MAX_FRAME_QP"               },
	{I_FRAME_MAX_QP,                 "I_FRAME_MAX_QP"             },
	{P_FRAME_MAX_QP,                 "P_FRAME_MAX_QP"             },
	{B_FRAME_MAX_QP,                 "B_FRAME_MAX_QP"             },
	{HEVC_HIER_QP,                   "HEVC_HIER_QP"               },
	{I_FRAME_QP,                     "I_FRAME_QP"                 },
	{P_FRAME_QP,                     "P_FRAME_QP"                 },
	{B_FRAME_QP,                     "B_FRAME_QP"                 },
	{HIER_LAYER_QP,                  "HIER_LAYER_QP"              },
	{LAYER_TYPE,                     "LAYER_TYPE"                 },
	{LAYER_ENABLE,                   "LAYER_ENABLE"               },
	{ENH_LAYER_COUNT,                "ENH_LAYER_COUNT"            },
	{L0_BR,                          "L0_BR"                      },
	{L1_BR,                          "L1_BR"                      },
	{L2_BR,                          "L2_BR"                      },
	{L3_BR,                          "L3_BR"                      },
	{L4_BR,                          "L4_BR"                      },
	{L5_BR,                          "L5_BR"                      },
	{ENTROPY_MODE,                   "ENTROPY_MODE"               },
	{PROFILE,                        "PROFILE"                    },
	{LEVEL,                          "LEVEL"                      },
	{HEVC_TIER,                      "HEVC_TIER"                  },
	{LF_MODE,                        "LF_MODE"                    },
	{LF_ALPHA,                       "LF_ALPHA"                   },
	{LF_BETA,                        "LF_BETA"                    },
	{SLICE_MODE,                     "SLICE_MODE"                 },
	{SLICE_MAX_BYTES,                "SLICE_MAX_BYTES"            },
	{SLICE_MAX_MB,                   "SLICE_MAX_MB"               },
	{MB_RC,                          "MB_RC"                      },
	{TRANSFORM_8X8,                  "TRANSFORM_8X8"              },
	{CHROMA_QP_INDEX_OFFSET,         "CHROMA_QP_INDEX_OFFSET"     },
	{DISPLAY_DELAY_ENABLE,           "DISPLAY_DELAY_ENABLE"       },
	{DISPLAY_DELAY,                  "DISPLAY_DELAY"              },
	{CONCEAL_COLOR_8BIT,             "CONCEAL_COLOR_8BIT"         },
	{CONCEAL_COLOR_10BIT,            "CONCEAL_COLOR_10BIT"        },
	{STAGE,                          "STAGE"                      },
	{PIPE,                           "PIPE"                       },
	{POC,                            "POC"                        },
	{QUALITY_MODE,                   "QUALITY_MODE"               },
	{CODED_FRAMES,                   "CODED_FRAMES"               },
	{BIT_DEPTH,                      "BIT_DEPTH"                  },
	{CODEC_CONFIG,                   "CODEC_CONFIG"               },
	{BITSTREAM_SIZE_OVERWRITE,       "BITSTREAM_SIZE_OVERWRITE"   },
	{THUMBNAIL_MODE,                 "THUMBNAIL_MODE"             },
	{DEFAULT_HEADER,                 "DEFAULT_HEADER"             },
	{RAP_FRAME,                      "RAP_FRAME"                  },
	{SEQ_CHANGE_AT_SYNC_FRAME,       "SEQ_CHANGE_AT_SYNC_FRAME"   },
	{PRIORITY,                       "PRIORITY"                   },
	{ENC_IP_CR,                      "ENC_IP_CR"                  },
	{META_LTR_MARK_USE,              "META_LTR_MARK_USE"          },
	{META_DPB_MISR,                  "META_DPB_MISR"              },
	{META_OPB_MISR,                  "META_OPB_MISR"              },
	{META_INTERLACE,                 "META_INTERLACE"             },
	{META_TIMESTAMP,                 "META_TIMESTAMP"             },
	{META_CONCEALED_MB_CNT,          "META_CONCEALED_MB_CNT"      },
	{META_HIST_INFO,                 "META_HIST_INFO"             },
	{META_SEI_MASTERING_DISP,        "META_SEI_MASTERING_DISP"    },
	{META_SEI_CLL,                   "META_SEI_CLL"               },
	{META_HDR10PLUS,                 "META_HDR10PLUS"             },
	{META_EVA_STATS,                 "META_EVA_STATS"             },
	{META_BUF_TAG,                   "META_BUF_TAG"               },
	{META_DPB_TAG_LIST,              "META_DPB_TAG_LIST"          },
	{META_OUTPUT_BUF_TAG,            "META_OUTPUT_BUF_TAG"        },
	{META_SUBFRAME_OUTPUT,           "META_SUBFRAME_OUTPUT"       },
	{META_ENC_QP_METADATA,           "META_ENC_QP_METADATA"       },
	{META_ROI_INFO,                  "META_ROI_INFO"              },
	{INST_CAP_MAX,                   "INST_CAP_MAX"               },
};

const char *cap_name(enum msm_vidc_inst_capability_type cap)
{
	const char *name = "UNKNOWN CAP";

	if (cap > ARRAY_SIZE(cap_name_arr))
		goto exit;

	if (cap_name_arr[cap].cap != cap)
		goto exit;

	name = cap_name_arr[cap].name;

exit:
	return name;
}

struct msm_vidc_inst_state_name {
	enum msm_vidc_inst_state state;
	char *name;
};

static const struct msm_vidc_inst_state_name inst_state_name_arr[] = {
	{MSM_VIDC_OPEN,                  "OPEN"                       },
	{MSM_VIDC_START_INPUT,           "START_INPUT"                },
	{MSM_VIDC_START_OUTPUT,          "START_OUTPUT"               },
	{MSM_VIDC_START,                 "START"                      },
	{MSM_VIDC_DRC,                   "DRC"                        },
	{MSM_VIDC_DRC_LAST_FLAG,         "DRC_LAST_FLAG"              },
	{MSM_VIDC_DRAIN,                 "DRAIN"                      },
	{MSM_VIDC_DRAIN_LAST_FLAG,       "DRAIN_LAST_FLAG"            },
	{MSM_VIDC_DRC_DRAIN,             "DRC_DRAIN"                  },
	{MSM_VIDC_DRC_DRAIN_LAST_FLAG,   "DRC_DRAIN_LAST_FLAG"        },
	{MSM_VIDC_DRAIN_START_INPUT,     "DRAIN_START_INPUT"          },
	{MSM_VIDC_ERROR,                 "ERROR"                      },
};

const char *state_name(enum msm_vidc_inst_state state)
{
	const char *name = "UNKNOWN STATE";

	if (!state || state > ARRAY_SIZE(inst_state_name_arr))
		goto exit;

	if (inst_state_name_arr[state - 1].state != state)
		goto exit;

	name = inst_state_name_arr[state - 1].name;

exit:
	return name;
}

struct msm_vidc_core_state_name {
	enum msm_vidc_core_state state;
	char *name;
};

static const struct msm_vidc_core_state_name core_state_name_arr[] = {
	{MSM_VIDC_CORE_DEINIT,           "CORE_DEINIT"                },
	{MSM_VIDC_CORE_INIT,             "CORE_INIT"                  },
};

const char *core_state_name(enum msm_vidc_core_state state)
{
	const char *name = "UNKNOWN STATE";

	if (state >= ARRAY_SIZE(core_state_name_arr))
		goto exit;

	if (core_state_name_arr[state].state != state)
		goto exit;

	name = core_state_name_arr[state].name;

exit:
	return name;
}

void print_vidc_buffer(u32 tag, const char *tag_str, const char *str, struct msm_vidc_inst *inst,
		struct msm_vidc_buffer *vbuf)
{
	if (!(tag & msm_vidc_debug) || !inst || !vbuf || !tag_str || !str)
		return;

	if (vbuf->type == MSM_VIDC_BUF_INPUT || vbuf->type == MSM_VIDC_BUF_OUTPUT) {
		dprintk_inst(tag, tag_str, inst,
			"%s: %s: idx %2d fd %3d off %d daddr %#llx size %d filled %d flags %#x ts %lld attr %#x\n",
			str, vbuf->type == MSM_VIDC_BUF_INPUT ? "INPUT" : "OUTPUT",
			vbuf->index, vbuf->fd, vbuf->data_offset,
			vbuf->device_addr, vbuf->buffer_size, vbuf->data_size,
			vbuf->flags, vbuf->timestamp, vbuf->attr);
	} else if (vbuf->type == MSM_VIDC_BUF_INPUT_META ||
			   vbuf->type == MSM_VIDC_BUF_OUTPUT_META) {
		dprintk_inst(tag, tag_str, inst,
			"%s: %s: idx %2d fd %3d off %d daddr %#llx size %d filled %d flags %#x ts %lld attr %#x\n",
			str, vbuf->type == MSM_VIDC_BUF_INPUT_META ? "INPUT_META" : "OUTPUT_META",
			vbuf->index, vbuf->fd, vbuf->data_offset,
			vbuf->device_addr, vbuf->buffer_size, vbuf->data_size,
			vbuf->flags, vbuf->timestamp, vbuf->attr);
	}
}

void print_vb2_buffer(const char *str, struct msm_vidc_inst *inst,
		struct vb2_buffer *vb2)
{
	if (!inst || !vb2)
		return;

	if (vb2->type == INPUT_MPLANE || vb2->type == OUTPUT_MPLANE) {
		i_vpr_e(inst,
			"%s: %s: idx %2d fd %d off %d size %d filled %d\n",
			str, vb2->type == INPUT_MPLANE ? "INPUT" : "OUTPUT",
			vb2->index, vb2->planes[0].m.fd,
			vb2->planes[0].data_offset, vb2->planes[0].length,
			vb2->planes[0].bytesused);
	} else if (vb2->type == INPUT_META_PLANE || vb2->type == OUTPUT_META_PLANE) {
		i_vpr_e(inst,
			"%s: %s: idx %2d fd %d off %d size %d filled %d\n",
			str, vb2->type == INPUT_MPLANE ? "INPUT_META" : "OUTPUT_META",
			vb2->index, vb2->planes[0].m.fd,
			vb2->planes[0].data_offset, vb2->planes[0].length,
			vb2->planes[0].bytesused);
	}
}

enum msm_vidc_buffer_type v4l2_type_to_driver(u32 type, const char *func)
{
	enum msm_vidc_buffer_type buffer_type = 0;

	switch (type) {
	case INPUT_MPLANE:
		buffer_type = MSM_VIDC_BUF_INPUT;
		break;
	case OUTPUT_MPLANE:
		buffer_type = MSM_VIDC_BUF_OUTPUT;
		break;
	case INPUT_META_PLANE:
		buffer_type = MSM_VIDC_BUF_INPUT_META;
		break;
	case OUTPUT_META_PLANE:
		buffer_type = MSM_VIDC_BUF_OUTPUT_META;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 buffer type %#x\n", func, type);
		break;
	}
	return buffer_type;
}

u32 v4l2_type_from_driver(enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	u32 type = 0;

	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		type = INPUT_MPLANE;
		break;
	case MSM_VIDC_BUF_OUTPUT:
		type = OUTPUT_MPLANE;
		break;
	case MSM_VIDC_BUF_INPUT_META:
		type = INPUT_META_PLANE;
		break;
	case MSM_VIDC_BUF_OUTPUT_META:
		type = OUTPUT_META_PLANE;
		break;
	default:
		d_vpr_e("%s: invalid driver buffer type %d\n",
			func, buffer_type);
		break;
	}
	return type;
}

enum msm_vidc_codec_type v4l2_codec_to_driver(u32 v4l2_codec, const char *func)
{
	enum msm_vidc_codec_type codec = 0;

	switch (v4l2_codec) {
	case V4L2_PIX_FMT_H264:
		codec = MSM_VIDC_H264;
		break;
	case V4L2_PIX_FMT_HEVC:
		codec = MSM_VIDC_HEVC;
		break;
	case V4L2_PIX_FMT_VP9:
		codec = MSM_VIDC_VP9;
		break;
	case V4L2_PIX_FMT_HEIC:
		codec = MSM_VIDC_HEIC;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 codec %#x\n", func, v4l2_codec);
		break;
	}
	return codec;
}

u32 v4l2_codec_from_driver(enum msm_vidc_codec_type codec, const char *func)
{
	u32 v4l2_codec = 0;

	switch (codec) {
	case MSM_VIDC_H264:
		v4l2_codec = V4L2_PIX_FMT_H264;
		break;
	case MSM_VIDC_HEVC:
		v4l2_codec = V4L2_PIX_FMT_HEVC;
		break;
	case MSM_VIDC_VP9:
		v4l2_codec = V4L2_PIX_FMT_VP9;
		break;
	case MSM_VIDC_HEIC:
		v4l2_codec = V4L2_PIX_FMT_HEIC;
		break;
	default:
		d_vpr_e("%s: invalid driver codec %#x\n", func, codec);
		break;
	}
	return v4l2_codec;
}

enum msm_vidc_colorformat_type v4l2_colorformat_to_driver(u32 v4l2_colorformat,
	const char *func)
{
	enum msm_vidc_colorformat_type colorformat = 0;

	switch (v4l2_colorformat) {
	case V4L2_PIX_FMT_NV12:
		colorformat = MSM_VIDC_FMT_NV12;
		break;
	case V4L2_PIX_FMT_NV21:
		colorformat = MSM_VIDC_FMT_NV21;
		break;
	case V4L2_PIX_FMT_VIDC_NV12C:
		colorformat = MSM_VIDC_FMT_NV12C;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		colorformat = MSM_VIDC_FMT_TP10C;
		break;
	case V4L2_PIX_FMT_RGBA32:
		colorformat = MSM_VIDC_FMT_RGBA8888;
		break;
	case V4L2_PIX_FMT_VIDC_ARGB32C:
		colorformat = MSM_VIDC_FMT_RGBA8888C;
		break;
	case V4L2_PIX_FMT_VIDC_P010:
		colorformat = MSM_VIDC_FMT_P010;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 color format %#x\n",
			func, v4l2_colorformat);
		break;
	}
	return colorformat;
}

u32 v4l2_colorformat_from_driver(enum msm_vidc_colorformat_type colorformat,
	const char *func)
{
	u32 v4l2_colorformat = 0;

	switch (colorformat) {
	case MSM_VIDC_FMT_NV12:
		v4l2_colorformat = V4L2_PIX_FMT_NV12;
		break;
	case MSM_VIDC_FMT_NV21:
		v4l2_colorformat = V4L2_PIX_FMT_NV21;
		break;
	case MSM_VIDC_FMT_NV12C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_NV12C;
		break;
	case MSM_VIDC_FMT_TP10C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_TP10C;
		break;
	case MSM_VIDC_FMT_RGBA8888:
		v4l2_colorformat = V4L2_PIX_FMT_RGBA32;
		break;
	case MSM_VIDC_FMT_RGBA8888C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_ARGB32C;
		break;
	case MSM_VIDC_FMT_P010:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_P010;
		break;
	default:
		d_vpr_e("%s: invalid driver color format %#x\n",
			func, colorformat);
		break;
	}
	return v4l2_colorformat;
}

u32 v4l2_color_primaries_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_primaries, const char *func)
{
	u32 vidc_color_primaries = MSM_VIDC_PRIMARIES_RESERVED;

	switch(v4l2_primaries) {
	case V4L2_COLORSPACE_DEFAULT:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_RESERVED;
		break;
	case V4L2_COLORSPACE_REC709:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_BT709;
		break;
	case V4L2_COLORSPACE_470_SYSTEM_M:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_BT470_SYSTEM_M;
		break;
	case V4L2_COLORSPACE_470_SYSTEM_BG:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_BT470_SYSTEM_BG;
		break;
	case V4L2_COLORSPACE_SMPTE170M:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_BT601_525;
		break;
	case V4L2_COLORSPACE_SMPTE240M:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_SMPTE_ST240M;
		break;
	case V4L2_COLORSPACE_VIDC_GENERIC_FILM:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_GENERIC_FILM;
		break;
	case V4L2_COLORSPACE_BT2020:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_BT2020;
		break;
	case V4L2_COLORSPACE_DCI_P3:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_SMPTE_RP431_2;
		break;
	case V4L2_COLORSPACE_VIDC_EG431:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_SMPTE_EG431_1;
		break;
	case V4L2_COLORSPACE_VIDC_EBU_TECH:
		vidc_color_primaries = MSM_VIDC_PRIMARIES_SMPTE_EBU_TECH;
		break;
	default:
		i_vpr_e(inst, "%s: invalid v4l2 color primaries %d\n",
			func, v4l2_primaries);
		break;
	}

	return vidc_color_primaries;
}

u32 v4l2_color_primaries_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_color_primaries, const char *func)
{
	u32 v4l2_primaries = V4L2_COLORSPACE_DEFAULT;

	switch(vidc_color_primaries) {
	case MSM_VIDC_PRIMARIES_UNSPECIFIED:
		v4l2_primaries = V4L2_COLORSPACE_DEFAULT;
		break;
	case MSM_VIDC_PRIMARIES_BT709:
		v4l2_primaries = V4L2_COLORSPACE_REC709;
		break;
	case MSM_VIDC_PRIMARIES_BT470_SYSTEM_M:
		v4l2_primaries = V4L2_COLORSPACE_470_SYSTEM_M;
		break;
	case MSM_VIDC_PRIMARIES_BT470_SYSTEM_BG:
		v4l2_primaries = V4L2_COLORSPACE_470_SYSTEM_BG;
		break;
	case MSM_VIDC_PRIMARIES_BT601_525:
		v4l2_primaries = V4L2_COLORSPACE_SMPTE170M;
		break;
	case MSM_VIDC_PRIMARIES_SMPTE_ST240M:
		v4l2_primaries = V4L2_COLORSPACE_SMPTE240M;
		break;
	case MSM_VIDC_PRIMARIES_GENERIC_FILM:
		v4l2_primaries = V4L2_COLORSPACE_VIDC_GENERIC_FILM;
		break;
	case MSM_VIDC_PRIMARIES_BT2020:
		v4l2_primaries = V4L2_COLORSPACE_BT2020;
		break;
	case MSM_VIDC_PRIMARIES_SMPTE_RP431_2:
		v4l2_primaries = V4L2_COLORSPACE_DCI_P3;
		break;
	case MSM_VIDC_PRIMARIES_SMPTE_EG431_1:
		v4l2_primaries = V4L2_COLORSPACE_VIDC_EG431;
		break;
	case MSM_VIDC_PRIMARIES_SMPTE_EBU_TECH:
		v4l2_primaries = V4L2_COLORSPACE_VIDC_EBU_TECH;
		break;
	default:
		i_vpr_e(inst, "%s: invalid hfi color primaries %d\n",
			func, vidc_color_primaries);
		break;
	}

	return v4l2_primaries;
}

u32 v4l2_transfer_char_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_transfer_char, const char *func)
{
	u32 vidc_transfer_char = MSM_VIDC_TRANSFER_RESERVED;

	switch(v4l2_transfer_char) {
	case V4L2_XFER_FUNC_DEFAULT:
		vidc_transfer_char = MSM_VIDC_TRANSFER_RESERVED;
		break;
	case V4L2_XFER_FUNC_709:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT709;
		break;
	case V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_M:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT470_SYSTEM_M;
		break;
	case V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_BG:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT470_SYSTEM_BG;
		break;
	case V4L2_XFER_FUNC_VIDC_BT601_525_OR_625:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT601_525_OR_625;
		break;
	case V4L2_XFER_FUNC_SMPTE240M:
		vidc_transfer_char = MSM_VIDC_TRANSFER_SMPTE_ST240M;
		break;
	case V4L2_XFER_FUNC_VIDC_LINEAR:
		vidc_transfer_char = MSM_VIDC_TRANSFER_LINEAR;
		break;
	case V4L2_XFER_FUNC_VIDC_XVYCC:
		vidc_transfer_char = MSM_VIDC_TRANSFER_XVYCC;
		break;
	case V4L2_XFER_FUNC_VIDC_BT1361:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT1361_0;
		break;
	case V4L2_XFER_FUNC_SRGB:
		vidc_transfer_char = MSM_VIDC_TRANSFER_SRGB_SYCC;
		break;
	case V4L2_XFER_FUNC_VIDC_BT2020:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT2020_14;
		break;
	case V4L2_XFER_FUNC_SMPTE2084:
		vidc_transfer_char = MSM_VIDC_TRANSFER_SMPTE_ST2084_PQ;
		break;
	case V4L2_XFER_FUNC_VIDC_ST428:
		vidc_transfer_char = MSM_VIDC_TRANSFER_SMPTE_ST428_1;
		break;
	case V4L2_XFER_FUNC_VIDC_HLG:
		vidc_transfer_char = MSM_VIDC_TRANSFER_BT2100_2_HLG;
		break;
	default:
		i_vpr_e(inst, "%s: invalid v4l2 transfer char %d\n",
			func, v4l2_transfer_char);
		break;
	}

	return vidc_transfer_char;
}

u32 v4l2_transfer_char_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_transfer_char, const char *func)
{
	u32  v4l2_transfer_char = V4L2_XFER_FUNC_DEFAULT;

	switch(vidc_transfer_char) {
	case MSM_VIDC_TRANSFER_UNSPECIFIED:
		v4l2_transfer_char = V4L2_XFER_FUNC_DEFAULT;
		break;
	case MSM_VIDC_TRANSFER_BT709:
		v4l2_transfer_char = V4L2_XFER_FUNC_709;
		break;
	case MSM_VIDC_TRANSFER_BT470_SYSTEM_M:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_M;
		break;
	case MSM_VIDC_TRANSFER_BT470_SYSTEM_BG:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_BT470_SYSTEM_BG;
		break;
	case MSM_VIDC_TRANSFER_BT601_525_OR_625:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_BT601_525_OR_625;
		break;
	case MSM_VIDC_TRANSFER_SMPTE_ST240M:
		v4l2_transfer_char = V4L2_XFER_FUNC_SMPTE240M;
		break;
	case MSM_VIDC_TRANSFER_LINEAR:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_LINEAR;
		break;
	case MSM_VIDC_TRANSFER_XVYCC:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_XVYCC;
		break;
	case MSM_VIDC_TRANSFER_BT1361_0:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_BT1361;
		break;
	case MSM_VIDC_TRANSFER_SRGB_SYCC:
		v4l2_transfer_char = V4L2_XFER_FUNC_SRGB;
		break;
	case MSM_VIDC_TRANSFER_BT2020_14:
	case MSM_VIDC_TRANSFER_BT2020_15:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_BT2020;
		break;
	case MSM_VIDC_TRANSFER_SMPTE_ST2084_PQ:
		v4l2_transfer_char = V4L2_XFER_FUNC_SMPTE2084;
		break;
	case MSM_VIDC_TRANSFER_SMPTE_ST428_1:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_ST428;
		break;
	case MSM_VIDC_TRANSFER_BT2100_2_HLG:
		v4l2_transfer_char = V4L2_XFER_FUNC_VIDC_HLG;
		break;
	default:
		i_vpr_e(inst, "%s: invalid hfi transfer char %d\n",
			func, vidc_transfer_char);
		break;
	}

	return v4l2_transfer_char;
}

u32 v4l2_matrix_coeff_to_driver(struct msm_vidc_inst *inst,
	u32 v4l2_matrix_coeff, const char *func)
{
	u32 vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_RESERVED;

	switch(v4l2_matrix_coeff) {
	case V4L2_YCBCR_ENC_DEFAULT:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_RESERVED;
		break;
	case V4L2_YCBCR_VIDC_SRGB_OR_SMPTE_ST428:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_SRGB_SMPTE_ST428_1;
		break;
	case V4L2_YCBCR_ENC_709:
	case V4L2_YCBCR_ENC_XV709:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT709;
		break;
	case V4L2_YCBCR_VIDC_FCC47_73_682:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_FCC_TITLE_47;
		break;
	case V4L2_YCBCR_ENC_XV601:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT470_SYS_BG_OR_BT601_625;
		break;
	case V4L2_YCBCR_ENC_601:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT601_525_BT1358_525_OR_625;
		break;
	case V4L2_YCBCR_ENC_SMPTE240M:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_SMPTE_ST240;
		break;
	case V4L2_YCBCR_ENC_BT2020:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT2020_NON_CONSTANT;
		break;
	case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
		vidc_matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT2020_CONSTANT;
		break;
	default:
		i_vpr_e(inst, "%s: invalid v4l2 matrix coeff %d\n",
			func, v4l2_matrix_coeff);
		break;
	}

	return vidc_matrix_coeff;
}

u32 v4l2_matrix_coeff_from_driver(struct msm_vidc_inst *inst,
	u32 vidc_matrix_coeff, const char *func)
{
	u32 v4l2_matrix_coeff = V4L2_YCBCR_ENC_DEFAULT;

	switch(vidc_matrix_coeff) {
	case MSM_VIDC_MATRIX_COEFF_SRGB_SMPTE_ST428_1:
		v4l2_matrix_coeff = V4L2_YCBCR_VIDC_SRGB_OR_SMPTE_ST428;
		break;
	case MSM_VIDC_MATRIX_COEFF_BT709:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_709;
		break;
	case MSM_VIDC_MATRIX_COEFF_UNSPECIFIED:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_DEFAULT;
		break;
	case MSM_VIDC_MATRIX_COEFF_FCC_TITLE_47:
		v4l2_matrix_coeff = V4L2_YCBCR_VIDC_FCC47_73_682;
		break;
	case MSM_VIDC_MATRIX_COEFF_BT470_SYS_BG_OR_BT601_625:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_XV601;
		break;
	case MSM_VIDC_MATRIX_COEFF_BT601_525_BT1358_525_OR_625:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_601;
		break;
	case MSM_VIDC_MATRIX_COEFF_SMPTE_ST240:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_SMPTE240M;
		break;
	case MSM_VIDC_MATRIX_COEFF_BT2020_NON_CONSTANT:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_BT2020;
		break;
	case MSM_VIDC_MATRIX_COEFF_BT2020_CONSTANT:
		v4l2_matrix_coeff = V4L2_YCBCR_ENC_BT2020_CONST_LUM;
		break;
	default:
		i_vpr_e(inst, "%s: invalid hfi matrix coeff %d\n",
			func, vidc_matrix_coeff);
		break;
	}

	return v4l2_matrix_coeff;
}

int v4l2_type_to_driver_port(struct msm_vidc_inst *inst, u32 type,
	const char *func)
{
	int port;

	if (type == INPUT_MPLANE) {
		port = INPUT_PORT;
	} else if (type == INPUT_META_PLANE) {
		port = INPUT_META_PORT;
	} else if (type == OUTPUT_MPLANE) {
		port = OUTPUT_PORT;
	} else if (type == OUTPUT_META_PLANE) {
		port = OUTPUT_META_PORT;
	} else {
		i_vpr_e(inst, "%s: port not found for v4l2 type %d\n",
			func, type);
		port = -EINVAL;
	}

	return port;
}

u32 msm_vidc_get_buffer_region(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, const char *func)
{
	u32 region = MSM_VIDC_NON_SECURE;

	if (!is_secure_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_ARP:
			region = MSM_VIDC_SECURE_NONPIXEL;
			break;
		case MSM_VIDC_BUF_INPUT:
			if (is_encode_session(inst))
				region = MSM_VIDC_NON_SECURE_PIXEL;
			else
				region = MSM_VIDC_NON_SECURE;
			break;
		case MSM_VIDC_BUF_OUTPUT:
			if (is_decode_session(inst))
				region = MSM_VIDC_NON_SECURE_PIXEL;
			else
				region = MSM_VIDC_NON_SECURE;
			break;
		case MSM_VIDC_BUF_DPB:
			region = MSM_VIDC_NON_SECURE_PIXEL;
			break;
		default:
			region = MSM_VIDC_NON_SECURE;
			break;
		}
	} else {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
			if (is_encode_session(inst))
				region = MSM_VIDC_SECURE_PIXEL;
			else
				region = MSM_VIDC_SECURE_BITSTREAM;
			break;
		case MSM_VIDC_BUF_OUTPUT:
			if (is_encode_session(inst))
				region = MSM_VIDC_SECURE_BITSTREAM;
			else
				region = MSM_VIDC_SECURE_PIXEL;
			break;
		case MSM_VIDC_BUF_INPUT_META:
		case MSM_VIDC_BUF_OUTPUT_META:
			region = MSM_VIDC_NON_SECURE;
			break;
		case MSM_VIDC_BUF_BIN:
			region = MSM_VIDC_SECURE_BITSTREAM;
			break;
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
			region = MSM_VIDC_SECURE_NONPIXEL;
			break;
		case MSM_VIDC_BUF_DPB:
			region = MSM_VIDC_SECURE_PIXEL;
			break;
		case MSM_VIDC_BUF_PERSIST:
		case MSM_VIDC_BUF_ARP:
			region = MSM_VIDC_SECURE_NONPIXEL;
			break;
		default:
			i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
				func, buffer_type);
		}
	}

	return region;
}

struct msm_vidc_buffers *msm_vidc_get_buffers(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		return &inst->buffers.input;
	case MSM_VIDC_BUF_INPUT_META:
		return &inst->buffers.input_meta;
	case MSM_VIDC_BUF_OUTPUT:
		return &inst->buffers.output;
	case MSM_VIDC_BUF_OUTPUT_META:
		return &inst->buffers.output_meta;
	case MSM_VIDC_BUF_BIN:
		return &inst->buffers.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->buffers.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->buffers.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->buffers.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->buffers.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->buffers.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->buffers.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->buffers.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

struct msm_vidc_mappings *msm_vidc_get_mappings(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		return &inst->mappings.input;
	case MSM_VIDC_BUF_INPUT_META:
		return &inst->mappings.input_meta;
	case MSM_VIDC_BUF_OUTPUT:
		return &inst->mappings.output;
	case MSM_VIDC_BUF_OUTPUT_META:
		return &inst->mappings.output_meta;
	case MSM_VIDC_BUF_BIN:
		return &inst->mappings.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->mappings.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->mappings.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->mappings.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->mappings.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->mappings.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->mappings.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->mappings.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

struct msm_vidc_allocations *msm_vidc_get_allocations(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_BIN:
		return &inst->allocations.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->allocations.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->allocations.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->allocations.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->allocations.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->allocations.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->allocations.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->allocations.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

int msm_vidc_change_core_state(struct msm_vidc_core *core,
	enum msm_vidc_core_state request_state, const char *func)
{
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s: core state changed from %s to %s\n",
		func, core_state_name(core->state),
		core_state_name(request_state));
	core->state = request_state;
	return 0;
}

int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_state request_state, const char *func)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!request_state) {
		i_vpr_e(inst, "%s: invalid request state\n", func);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_ERROR) {
		i_vpr_h(inst,
			"%s: inst is in bad state, can not change state to %s\n",
			func, state_name(request_state));
		return 0;
	}

	i_vpr_h(inst, "%s: state changed from %s to %s\n",
		   func, state_name(inst->state), state_name(request_state));
	inst->state = request_state;
	return 0;
}

bool msm_vidc_allow_s_fmt(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT ||
		    inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			allow = true;
			goto exit;
		}
	}
	if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			allow = true;
			goto exit;
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));
	return allow;
}

bool msm_vidc_allow_s_ctrl(struct msm_vidc_inst *inst, u32 id)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (is_decode_session(inst)) {
		if (!inst->vb2q[INPUT_PORT].streaming) {
			allow = true;
			goto exit;
		}
		if (inst->vb2q[INPUT_PORT].streaming) {
			switch (id) {
			case V4L2_CID_MPEG_VIDC_CODEC_CONFIG:
				allow = true;
				break;
			default:
				allow = false;
				break;
			}
		}
	} else if (is_encode_session(inst)) {
		if (inst->state == MSM_VIDC_START || inst->state == MSM_VIDC_START_OUTPUT) {
			switch (id) {
			case V4L2_CID_MPEG_VIDEO_BITRATE:
			case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
			case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
			case V4L2_CID_HFLIP:
			case V4L2_CID_VFLIP:
			case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
			case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR:
			case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR:
			case V4L2_CID_MPEG_VIDC_USELTRFRAME:
			case V4L2_CID_MPEG_VIDC_MARKLTRFRAME:
			case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES:
			case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_RESOLUTION:
			case V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY:
			case V4L2_CID_MPEG_VIDC_ENC_INPUT_COMPRESSION_RATIO:
			case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
				allow = true;
				break;
			default:
				allow = false;
				break;
			}
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: id %d not allowed in state %s\n",
			__func__, id, state_name(inst->state));
	return allow;
}

bool msm_vidc_allow_metadata(struct msm_vidc_inst *inst, u32 cap_id)
{
	bool is_allowed = true;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}

	switch (cap_id) {
	case META_OUTPUT_BUF_TAG:
	case META_DPB_TAG_LIST:
		if (!is_ubwc_colorformat(inst->capabilities->cap[PIX_FMTS].value)) {
			i_vpr_h(inst,
				"%s: cap: %24s not allowed for split mode\n",
				__func__, cap_name(cap_id));
			is_allowed = false;
		}
		break;
	default:
		is_allowed = true;
		break;
	}

	return is_allowed;
}

bool msm_vidc_allow_reqbufs(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT ||
		    inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			allow = true;
			goto exit;
		}
	}
	if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			allow = true;
			goto exit;
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));
	return allow;
}

enum msm_vidc_allow msm_vidc_allow_stop(struct msm_vidc_inst *inst)
{
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return allow;
	}
	if (inst->state == MSM_VIDC_START ||
		inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN) {
		allow = MSM_VIDC_ALLOW;
	} else if (inst->state == MSM_VIDC_START_INPUT) {
		allow = MSM_VIDC_IGNORE;
		i_vpr_e(inst, "%s: stop ignored in state %s\n",
			__func__, state_name(inst->state));
	} else {
		i_vpr_e(inst, "%s: stop not allowed in state %s\n",
			__func__, state_name(inst->state));
	}

	return allow;
}

bool msm_vidc_allow_start(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG)
		return true;

	i_vpr_e(inst, "%s: not allowed in state %s\n",
			__func__, state_name(inst->state));
	return false;
}

bool msm_vidc_allow_streamon(struct msm_vidc_inst *inst, u32 type)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_OUTPUT)
			return true;
	} else if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_INPUT ||
			inst->state == MSM_VIDC_DRAIN_START_INPUT)
			return true;
	}

	i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
			__func__, type, state_name(inst->state));
	return false;
}

bool msm_vidc_allow_streamoff(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = true;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_OUTPUT)
			allow = false;
	} else if (type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT)
			allow = false;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_INPUT)
			allow = false;
	} else if (type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT)
			allow = false;
	}
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));

	return allow;
}

enum msm_vidc_allow msm_vidc_allow_qbuf(struct msm_vidc_inst *inst, u32 type)
{
	int port = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return MSM_VIDC_DISALLOW;
	}
	if (inst->state == MSM_VIDC_ERROR) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		return MSM_VIDC_DISALLOW;
	}

	port = v4l2_type_to_driver_port(inst, type, __func__);
	if (port < 0)
		return MSM_VIDC_DISALLOW;

	/* defer queuing if streamon not completed */
	if (!inst->vb2q[port].streaming)
		return MSM_VIDC_DEFER;

	if (type == INPUT_META_PLANE || type == OUTPUT_META_PLANE)
		return MSM_VIDC_DEFER;

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_OUTPUT)
			return MSM_VIDC_DEFER;
		else
			return MSM_VIDC_ALLOW;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_INPUT ||
			inst->state == MSM_VIDC_DRAIN_START_INPUT)
			return MSM_VIDC_DEFER;
		else
			return MSM_VIDC_ALLOW;
	} else {
		i_vpr_e(inst, "%s: unknown buffer type %d\n", __func__, type);
		return MSM_VIDC_DISALLOW;
	}

	return MSM_VIDC_DISALLOW;
}

enum msm_vidc_allow msm_vidc_allow_input_psc(struct msm_vidc_inst *inst)
{
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return MSM_VIDC_DISALLOW;
	}
	if (inst->state == MSM_VIDC_START ||
		inst->state == MSM_VIDC_START_INPUT ||
		inst->state == MSM_VIDC_DRAIN) {
		allow = MSM_VIDC_ALLOW;
	} else if (inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN ||
		inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRAIN_START_INPUT) {
		i_vpr_h(inst, "%s: defer input psc, inst state %s\n",
				__func__, state_name(inst->state));
		allow = MSM_VIDC_DEFER;
	} else {
		i_vpr_e(inst, "%s: input psc in wrong state %s\n",
				__func__, state_name(inst->state));
		allow = MSM_VIDC_DISALLOW;
	}

	return allow;
}

bool msm_vidc_allow_last_flag(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRAIN ||
		inst->state == MSM_VIDC_DRC_DRAIN)
		return true;

	i_vpr_e(inst, "%s: not allowed in state %s\n",
			__func__, state_name(inst->state));
	return false;
}

static int msm_vidc_process_pending_ipsc(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_state *new_state)
{
	struct response_work *resp_work, *dummy = NULL;
	int rc = 0;

	if (!inst || !new_state) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (list_empty(&inst->response_works))
		return 0;

	i_vpr_h(inst, "%s: state %s, ipsc pending\n", __func__, state_name(inst->state));
	list_for_each_entry_safe(resp_work, dummy, &inst->response_works, list) {
		if (resp_work->type == RESP_WORK_INPUT_PSC) {
			rc = handle_session_response_work(inst, resp_work);
			if (rc) {
				i_vpr_e(inst, "%s: handle ipsc failed\n", __func__);
				*new_state = MSM_VIDC_ERROR;
			} else {
				if (inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG ||
					inst->state == MSM_VIDC_DRAIN_START_INPUT) {
					*new_state = MSM_VIDC_DRC_DRAIN;
				} else if (inst->state == MSM_VIDC_DRC_LAST_FLAG) {
					*new_state = MSM_VIDC_DRC;
				}
			}
			list_del(&resp_work->list);
			kfree(resp_work->data);
			kfree(resp_work);
			/* list contains max only one ipsc at anytime */
			break;
		}
	}

	return rc;
}

int msm_vidc_state_change_streamon(struct msm_vidc_inst *inst, u32 type)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == INPUT_META_PLANE || type == OUTPUT_META_PLANE)
		return 0;

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN)
			new_state = MSM_VIDC_START_INPUT;
		else if (inst->state == MSM_VIDC_START_OUTPUT)
			new_state = MSM_VIDC_START;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN) {
			new_state = MSM_VIDC_START_OUTPUT;
		} else if (inst->state == MSM_VIDC_START_INPUT) {
			new_state = MSM_VIDC_START;
		} else if (inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			i_vpr_h(inst, "%s: streamon(output) in %s state\n",
				__func__, state_name(inst->state));
			new_state = MSM_VIDC_DRAIN;
			rc = msm_vidc_process_pending_ipsc(inst, &new_state);
			if (rc) {
				i_vpr_e(inst, "%s: process pending ipsc failed\n", __func__);
				goto state_change;
			}
		}
	}

state_change:
	msm_vidc_change_inst_state(inst, new_state, __func__);

	return rc;
}

int msm_vidc_state_change_streamoff(struct msm_vidc_inst *inst, u32 type)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;
	struct response_work *resp_work, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == INPUT_META_PLANE || type == OUTPUT_META_PLANE)
		return 0;

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
			list_for_each_entry_safe(resp_work, dummy,
						&inst->response_works, list) {
				if (resp_work->type == RESP_WORK_INPUT_PSC) {
					i_vpr_h(inst,
						"%s: discard pending input psc\n", __func__);
					list_del(&resp_work->list);
					kfree(resp_work->data);
					kfree(resp_work);
				}
			}
		}
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
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		goto exit;

exit:
	return rc;
}

int msm_vidc_state_change_stop(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_START) {
		new_state = MSM_VIDC_DRAIN;
	} else if (inst->state == MSM_VIDC_DRC) {
		new_state = MSM_VIDC_DRC_DRAIN;
	} else if (inst->state == MSM_VIDC_DRC_DRAIN ||
			   inst->state == MSM_VIDC_DRC_LAST_FLAG) {
		new_state = MSM_VIDC_DRC_DRAIN_LAST_FLAG;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
			__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_start(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG) {
		new_state = MSM_VIDC_START;
		rc = msm_vidc_process_pending_ipsc(inst, &new_state);
		if (rc) {
			i_vpr_e(inst, "%s: process pending ipsc failed\n", __func__);
			goto state_change;
		}
	} else if (inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG) {
		new_state = MSM_VIDC_DRAIN;
		rc = msm_vidc_process_pending_ipsc(inst, &new_state);
		if (rc) {
			i_vpr_e(inst, "%s: process pending ipsc failed\n", __func__);
			goto state_change;
		}
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n", __func__, state_name(inst->state));
		new_state = MSM_VIDC_ERROR;
		rc = -EINVAL;
		goto state_change;
	}

state_change:
	msm_vidc_change_inst_state(inst, new_state, __func__);

	return rc;
}

int msm_vidc_state_change_input_psc(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	/* don't change state as output port is not started yet */
	if (inst->state == MSM_VIDC_START_INPUT)
		return 0;

	if (inst->state == MSM_VIDC_START) {
		new_state = MSM_VIDC_DRC;
	} else if (inst->state == MSM_VIDC_DRAIN) {
		new_state = MSM_VIDC_DRC_DRAIN;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
				__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_last_flag(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (inst->state == MSM_VIDC_DRC) {
		new_state = MSM_VIDC_DRC_LAST_FLAG;
	} else if (inst->state == MSM_VIDC_DRAIN) {
		new_state = MSM_VIDC_DRAIN_LAST_FLAG;
	} else if (inst->state == MSM_VIDC_DRC_DRAIN) {
		new_state = MSM_VIDC_DRC_DRAIN_LAST_FLAG;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
				__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_get_control(struct msm_vidc_inst *inst, struct v4l2_ctrl *ctrl)
{
	int rc = 0;

	if (!inst || !ctrl) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
		i_vpr_h(inst, "g_min: output buffers %d\n", ctrl->val);
		break;
	case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
		ctrl->val = inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
		i_vpr_h(inst, "g_min: input buffers %d\n", ctrl->val);
		break;
	default:
		break;
	}

	return rc;
}

int msm_vidc_get_mbs_per_frame(struct msm_vidc_inst *inst)
{
	int height, width;
	struct v4l2_format *out_f;
	struct v4l2_format *inp_f;

	out_f = &inst->fmts[OUTPUT_PORT];
	inp_f = &inst->fmts[INPUT_PORT];
	height = max(out_f->fmt.pix_mp.height,
			inp_f->fmt.pix_mp.height);
	width = max(out_f->fmt.pix_mp.width,
			inp_f->fmt.pix_mp.width);

	return NUM_MBS_PER_FRAME(height, width);
}

int msm_vidc_get_fps(struct msm_vidc_inst *inst)
{
	int fps;
	u32 frame_rate, operating_rate;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	frame_rate = inst->capabilities->cap[FRAME_RATE].value;
	operating_rate = inst->capabilities->cap[OPERATING_RATE].value;

	if (operating_rate > frame_rate)
		fps = (operating_rate >> 16) ?
			(operating_rate >> 16) : 1;
	else
		fps = frame_rate >> 16;

	return fps;
}

int msm_vidc_num_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type type, enum msm_vidc_buffer_attributes attr)
{
	int count = 0;
	struct msm_vidc_buffer *vbuf;
	struct msm_vidc_buffers *buffers;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return count;
	}
	if (type == MSM_VIDC_BUF_OUTPUT) {
		buffers = &inst->buffers.output;
	} else if (type == MSM_VIDC_BUF_INPUT) {
		buffers = &inst->buffers.input;
	} else {
		i_vpr_e(inst, "%s: invalid buffer type %#x\n",
				__func__, type);
		return count;
	}

	list_for_each_entry(vbuf, &buffers->list, list) {
		if (vbuf->type != type)
			continue;
		if (!(vbuf->attr & attr))
			continue;
		count++;
	}

	return count;
}

static int vb2_buffer_to_driver(struct vb2_buffer *vb2,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;

	if (!vb2 || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buf->valid = true;
	buf->type = v4l2_type_to_driver(vb2->type, __func__);
	if (!buf->type)
		return -EINVAL;
	buf->index = vb2->index;
	buf->fd = vb2->planes[0].m.fd;
	buf->data_offset = vb2->planes[0].data_offset;
	buf->data_size = vb2->planes[0].bytesused;
	buf->buffer_size = vb2->planes[0].length;
	buf->timestamp = vb2->timestamp;

	return rc;
}

int msm_vidc_unmap_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_map *map = NULL;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mappings = msm_vidc_get_mappings(inst, buf->type, __func__);
	if (!mappings)
		return -EINVAL;

	/* sanity check to see if it was not removed */
	list_for_each_entry(map, &mappings->list, list) {
		if (map->dmabuf == buf->dmabuf) {
			found = true;
			break;
		}
	}
	if (!found) {
		print_vidc_buffer(VIDC_ERR, "err ", "no buf in mappings", inst, buf);
		return -EINVAL;
	}

	rc = msm_vidc_memory_unmap(inst->core, map);
	if (rc) {
		print_vidc_buffer(VIDC_ERR, "err ", "unmap failed", inst, buf);
		return -EINVAL;
	}

	/* finally delete if refcount is zero */
	if (!map->refcount) {
		list_del(&map->list);
		kfree(map);
	}

	return 0;
}

int msm_vidc_put_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_unmap_driver_buf(inst, buf);
	if (rc)
		return rc;

	msm_vidc_memory_put_dmabuf(buf->dmabuf);

	/* delete the buffer from buffers->list */
	list_del(&buf->list);
	kfree(buf);

	return 0;
}

int msm_vidc_map_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_map *map = NULL;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mappings = msm_vidc_get_mappings(inst, buf->type, __func__);
	if (!mappings)
		return -EINVAL;

	/* check if it is an existing one */
	list_for_each_entry(map, &mappings->list, list) {
		if (map->dmabuf == buf->dmabuf) {
			found = true;
			break;
		}
	}
	if (found) {
		/* skip mapping for RO buffer */
		if (!(buf->attr & MSM_VIDC_ATTR_READ_ONLY)) {
			rc = msm_vidc_memory_map(inst->core, map);
			if (rc)
				return -ENOMEM;
			buf->device_addr = map->device_addr;
		}
		return 0;
	}
	map = kzalloc(sizeof(struct msm_vidc_map), GFP_KERNEL);
	if (!map) {
		i_vpr_e(inst, "%s: alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&map->list);
	map->type = buf->type;
	map->dmabuf = buf->dmabuf;
	map->region = msm_vidc_get_buffer_region(inst, buf->type, __func__);
	rc = msm_vidc_memory_map(inst->core, map);
	if (rc) {
		kfree(map);
		return -ENOMEM;
	}
	buf->device_addr = map->device_addr;
	list_add_tail(&map->list, &mappings->list);

	return 0;
}

struct msm_vidc_buffer *msm_vidc_get_driver_buf(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2)
{
	int rc = 0;
	struct msm_vidc_buffer *buf = NULL;
	struct msm_vidc_buffers *buffers;
	struct dma_buf *dmabuf;
	enum msm_vidc_buffer_type buf_type;
	bool found = false;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	buf_type = v4l2_type_to_driver(vb2->type, __func__);
	if (!buf_type)
		return NULL;

	buffers = msm_vidc_get_buffers(inst, buf_type, __func__);
	if (!buffers)
		return NULL;

	dmabuf = msm_vidc_memory_get_dmabuf(vb2->planes[0].m.fd);
	if (!dmabuf)
		return NULL;

	/* check if it is an existing buffer */
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->dmabuf == dmabuf &&
		    buf->data_offset == vb2->planes[0].data_offset) {
			found = true;
			break;
		}
	}
	if (found) {
		/* only YUV buffers are allowed to repeat */
		if ((is_decode_session(inst) && vb2->type != OUTPUT_MPLANE) ||
		    (is_encode_session(inst) && vb2->type != INPUT_MPLANE)) {
			print_vidc_buffer(VIDC_ERR, "err ",
				"existing buffer", inst, buf);
			goto error;
		}
		/* for decoder, YUV with RO flag are allowed to repeat */
		if (is_decode_session(inst) &&
		    !(buf->attr & MSM_VIDC_ATTR_READ_ONLY)) {
			print_vidc_buffer(VIDC_ERR, "err ",
				"existing buffer without RO flag", inst, buf);
			goto error;
		}
		/* for encoder, treat the repeated buffer as new buffer */
		if (is_encode_session(inst) && vb2->type == INPUT_MPLANE)
			found = false;
	}
	if (!found) {
		buf = kzalloc(sizeof(struct msm_vidc_buffer), GFP_KERNEL);
		if (!buf) {
			i_vpr_e(inst, "%s: alloc failed\n", __func__);
			goto error;
		}
		buf->dmabuf = dmabuf;
		INIT_LIST_HEAD(&buf->list);
		list_add_tail(&buf->list, &buffers->list);
	} else {
		/* reset all attributes except read only */
		buf->attr &= MSM_VIDC_ATTR_READ_ONLY;
	}

	/* treat every buffer as deferred buffer initially */
	buf->attr |= MSM_VIDC_ATTR_DEFERRED;

	rc = vb2_buffer_to_driver(vb2, buf);
	if (rc)
		goto error;

	rc = msm_vidc_map_driver_buf(inst, buf);
	if (rc)
		goto error;

	return buf;

error:
	msm_vidc_memory_put_dmabuf(dmabuf);
	if (!found)
		kfree(buf);
	return NULL;
}

struct msm_vidc_buffer *get_meta_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	struct msm_vidc_buffer *mbuf;
	struct msm_vidc_buffers *buffers;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT) {
		buffers = &inst->buffers.input_meta;
	} else if (buf->type == MSM_VIDC_BUF_OUTPUT) {
		buffers = &inst->buffers.output_meta;
	} else {
		i_vpr_e(inst, "%s: invalid buffer type %d\n",
			__func__, buf->type);
		return NULL;
	}
	list_for_each_entry(mbuf, &buffers->list, list) {
		if (!mbuf->valid)
			continue;
		if (mbuf->index == buf->index) {
			found = true;
			break;
		}
	}
	if (!found)
		return NULL;

	return mbuf;
}

bool msm_vidc_is_super_buffer(struct msm_vidc_inst *inst)
{
	struct msm_vidc_inst_capability *capability = NULL;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return false;
	}

	capability = inst->capabilities;

	return !!capability->cap[SUPER_FRAME].value;
}

static bool is_single_session(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return false;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry(inst, &core->instances, list)
		count++;
	core_unlock(core, __func__);

	return count == 1;
}

void msm_vidc_allow_dcvs(struct msm_vidc_inst *inst)
{
	bool allow = false;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: Invalid args: %pK\n", __func__, inst);
		return;
	}
	core = inst->core;

	allow = !msm_vidc_clock_voting;
	if (!allow) {
		i_vpr_h(inst, "%s: core_clock_voting is set\n", __func__);
		goto exit;
	}

	allow = core->capabilities[DCVS].value;
	if (!allow) {
		i_vpr_h(inst, "%s: core doesn't support dcvs\n", __func__);
		goto exit;
	}

	allow = !inst->decode_batch.enable;
	if (!allow) {
		i_vpr_h(inst, "%s: decode_batching enabled\n", __func__);
		goto exit;
	}

	allow = !msm_vidc_is_super_buffer(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: encode_batching(super_buffer) enabled\n", __func__);
		goto exit;
	}

	allow = !is_thumbnail_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: thumbnail session\n", __func__);
		goto exit;
	}

	allow = !is_image_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: image session\n", __func__);
		goto exit;
	}

exit:
	i_vpr_h(inst, "%s: dcvs: %s\n", __func__, allow ? "enabled" : "disabled");

	inst->power.dcvs_flags = 0;
	inst->power.dcvs_mode = allow;
}

bool msm_vidc_allow_decode_batch(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	bool allow = false;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	core = inst->core;

	allow = inst->decode_batch.enable;
	if (!allow) {
		i_vpr_h(inst, "%s: batching already disabled\n", __func__);
		goto exit;
	}

	allow = core->capabilities[DECODE_BATCH].value;
	if (!allow) {
		i_vpr_h(inst, "%s: core doesn't support batching\n", __func__);
		goto exit;
	}

	allow = is_single_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: multiple sessions running\n", __func__);
		goto exit;
	}

	allow = is_decode_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: not a decoder session\n", __func__);
		goto exit;
	}

	allow = !is_thumbnail_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: thumbnail session\n", __func__);
		goto exit;
	}

	allow = !is_image_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: image session\n", __func__);
		goto exit;
	}

	allow = is_realtime_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: non-realtime session\n", __func__);
		goto exit;
	}

	allow = !is_lowlatency_session(inst);
	if (!allow) {
		i_vpr_h(inst, "%s: lowlatency session\n", __func__);
		goto exit;
	}

exit:
	i_vpr_h(inst, "%s: batching: %s\n", __func__, allow ? "enabled" : "disabled");

	return allow;
}

static int msm_vidc_queue_buffer(struct msm_vidc_inst *inst, struct msm_vidc_buffer *buf)
{
	struct msm_vidc_buffer *meta;
	int rc = 0;

	if (!inst || !buf || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_decode_session(inst) && is_input_buffer(buf->type) &&
		inst->capabilities->cap[CODEC_CONFIG].value) {
		buf->flags |= MSM_VIDC_BUF_FLAG_CODECCONFIG;
		msm_vidc_update_cap_value(inst, CODEC_CONFIG, 0, __func__);
	}

	print_vidc_buffer(VIDC_HIGH, "high", "qbuf", inst, buf);
	meta = get_meta_buffer(inst, buf);
	if (meta)
		print_vidc_buffer(VIDC_HIGH, "high", "qbuf", inst, meta);

	if (!meta && is_meta_enabled(inst, buf->type)) {
		print_vidc_buffer(VIDC_ERR, "err ", "missing meta for", inst, buf);
		return -EINVAL;
	}

	if (msm_vidc_is_super_buffer(inst) && is_input_buffer(buf->type))
		rc = venus_hfi_queue_super_buffer(inst, buf, meta);
	else
		rc = venus_hfi_queue_buffer(inst, buf, meta);
	if (rc)
		return rc;

	buf->attr &= ~MSM_VIDC_ATTR_DEFERRED;
	buf->attr |= MSM_VIDC_ATTR_QUEUED;
	if (meta) {
		meta->attr &= ~MSM_VIDC_ATTR_DEFERRED;
		meta->attr |= MSM_VIDC_ATTR_QUEUED;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT)
		msm_vidc_debugfs_update(inst, MSM_VIDC_DEBUGFS_EVENT_ETB);
	else if (buf->type == MSM_VIDC_BUF_OUTPUT)
		msm_vidc_debugfs_update(inst, MSM_VIDC_DEBUGFS_EVENT_FTB);

	return 0;
}

int msm_vidc_queue_buffer_batch(struct msm_vidc_inst *inst)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_OUTPUT, __func__);
	if (!buffers)
		return -EINVAL;

	msm_vidc_scale_power(inst, true);

	list_for_each_entry(buf, &buffers->list, list) {
		if (!(buf->attr & MSM_VIDC_ATTR_DEFERRED))
			continue;
		rc = msm_vidc_queue_buffer(inst, buf);
		if (rc)
			return rc;
	}

	return 0;
}

void msm_vidc_update_input_cr(struct msm_vidc_inst *inst, u32 idx, u32 cr)
{
	struct msm_vidc_input_cr_data *temp, *next;
	bool found = false;

	list_for_each_entry_safe(temp, next, &inst->enc_input_crs, list) {
		if (temp->index == idx) {
			temp->input_cr = cr;
			found = true;
			break;
		}
	}
	if (!found) {
		temp = kzalloc(sizeof(*temp), GFP_KERNEL);
		if (!temp) {
			i_vpr_e(inst, "%s: malloc failure.\n", __func__);
			return;
		}
		temp->index = idx;
		temp->input_cr = cr;
		list_add_tail(&temp->list, &inst->enc_input_crs);
	}
}

void msm_vidc_free_input_cr_list(struct msm_vidc_inst *inst)
{
	struct msm_vidc_input_cr_data *temp, *next;

	list_for_each_entry_safe(temp, next, &inst->enc_input_crs, list) {
		list_del(&temp->list);
		kfree(temp);
	}
	INIT_LIST_HEAD(&inst->enc_input_crs);
}

int msm_vidc_queue_buffer_single(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;
	struct msm_vidc_buffer *buf;
	enum msm_vidc_allow allow;
	u32 cr = 0;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buf = msm_vidc_get_driver_buf(inst, vb2);
	if (!buf)
		return -EINVAL;

	allow = msm_vidc_allow_qbuf(inst, vb2->type);
	if (allow == MSM_VIDC_DISALLOW) {
		i_vpr_e(inst, "%s: qbuf not allowed\n", __func__);
		return -EINVAL;
	} else if (allow == MSM_VIDC_DEFER) {
		print_vidc_buffer(VIDC_LOW, "high", "qbuf deferred", inst, buf);
		return 0;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT) {
		if (is_encode_session(inst)) {
			cr = inst->capabilities->cap[ENC_IP_CR].value;
			msm_vidc_update_input_cr(inst, vb2->index, cr);
			inst->capabilities->cap[ENC_IP_CR].value = 0;
		}
		inst->power.buffer_counter++;
	}

	msm_vidc_scale_power(inst, is_input_buffer(buf->type));

	rc = msm_vidc_queue_buffer(inst, buf);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_destroy_internal_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_allocations *allocations;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_alloc *alloc, *alloc_dummy;
	struct msm_vidc_map  *map, *map_dummy;
	struct msm_vidc_buffer *buf, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!is_internal_buffer(buffer->type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer->type);
		return 0;
	}

	i_vpr_h(inst,
		"%s: destroy buffer_type %#x, size %d device_addr %#x\n",
		__func__, buffer->type, buffer->buffer_size,
		buffer->device_addr);

	buffers = msm_vidc_get_buffers(inst, buffer->type, __func__);
	if (!buffers)
		return -EINVAL;
	allocations = msm_vidc_get_allocations(inst, buffer->type, __func__);
	if (!allocations)
		return -EINVAL;
	mappings = msm_vidc_get_mappings(inst, buffer->type, __func__);
	if (!mappings)
		return -EINVAL;

	list_for_each_entry_safe(map, map_dummy, &mappings->list, list) {
		if (map->dmabuf == buffer->dmabuf) {
			msm_vidc_memory_unmap(inst->core, map);
			list_del(&map->list);
			kfree(map);
			break;
		}
	}

	list_for_each_entry_safe(alloc, alloc_dummy, &allocations->list, list) {
		if (alloc->dmabuf == buffer->dmabuf) {
			msm_vidc_memory_free(inst->core, alloc);
			list_del(&alloc->list);
			kfree(alloc);
			break;
		}
	}

	list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
		if (buf->dmabuf == buffer->dmabuf) {
			list_del(&buf->list);
			kfree(buf);
			break;
		}
	}

	buffers->size = 0;
	buffers->min_count = buffers->extra_count = buffers->actual_count = 0;

	return 0;
}

int msm_vidc_get_internal_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	u32 buf_size;
	u32 buf_count;
	struct msm_vidc_core *core;
	struct msm_vidc_buffers *buffers;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	buf_size = call_session_op(core, buffer_size,
		inst, buffer_type);

	buf_count = call_session_op(core, min_count,
		inst, buffer_type);

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buf_size <= buffers->size &&
		buf_count <= buffers->min_count) {
		buffers->reuse = true;
	} else {
		buffers->reuse = false;
		buffers->size = buf_size;
		buffers->min_count = buf_count;
	}
	return 0;
}

int msm_vidc_create_internal_buffer(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, u32 index)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_allocations *allocations;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_buffer *buffer;
	struct msm_vidc_alloc *alloc;
	struct msm_vidc_map *map;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;
	allocations = msm_vidc_get_allocations(inst, buffer_type, __func__);
	if (!allocations)
		return -EINVAL;
	mappings = msm_vidc_get_mappings(inst, buffer_type, __func__);
	if (!mappings)
		return -EINVAL;

	if (!buffers->size)
		return 0;

	buffer = kzalloc(sizeof(struct msm_vidc_buffer), GFP_KERNEL);
	if (!buffer) {
		i_vpr_e(inst, "%s: buf alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&buffer->list);
	buffer->valid = true;
	buffer->type = buffer_type;
	buffer->index = index;
	buffer->buffer_size = buffers->size;
	list_add_tail(&buffer->list, &buffers->list);

	alloc = kzalloc(sizeof(struct msm_vidc_alloc), GFP_KERNEL);
	if (!alloc) {
		i_vpr_e(inst, "%s: alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&alloc->list);
	alloc->type = buffer_type;
	alloc->region = msm_vidc_get_buffer_region(inst,
		buffer_type, __func__);
	alloc->size = buffer->buffer_size;
	alloc->secure = is_secure_region(alloc->region);
	rc = msm_vidc_memory_alloc(inst->core, alloc);
	if (rc)
		return -ENOMEM;
	list_add_tail(&alloc->list, &allocations->list);

	map = kzalloc(sizeof(struct msm_vidc_map), GFP_KERNEL);
	if (!map) {
		i_vpr_e(inst, "%s: map alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&map->list);
	map->type = alloc->type;
	map->region = alloc->region;
	map->dmabuf = alloc->dmabuf;
	rc = msm_vidc_memory_map(inst->core, map);
	if (rc)
		return -ENOMEM;
	list_add_tail(&map->list, &mappings->list);

	buffer->dmabuf = alloc->dmabuf;
	buffer->device_addr = map->device_addr;
	i_vpr_h(inst,
		"%s: created buffer_type %#x, size %d device_addr %#x\n",
		__func__, buffer_type, buffers->size,
		buffer->device_addr);

	return 0;
}

int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	int i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	for (i = 0; i < buffers->min_count; i++) {
		rc = msm_vidc_create_internal_buffer(inst, buffer_type, i);
		if (rc)
			return rc;
	}

	return rc;
}

int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buffer, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	list_for_each_entry_safe(buffer, dummy, &buffers->list, list) {
		/* do not queue pending release buffers */
		if (buffer->flags & MSM_VIDC_ATTR_PENDING_RELEASE)
			continue;
		/* do not queue already queued buffers */
		if (buffer->attr & MSM_VIDC_ATTR_QUEUED)
			continue;
		rc = venus_hfi_queue_buffer(inst, buffer, NULL);
		if (rc)
			return rc;
		/* mark queued */
		buffer->attr |= MSM_VIDC_ATTR_QUEUED;

		i_vpr_h(inst, "%s: queued buffer_type %#x, size %d\n",
			__func__, buffer_type, buffers->size);
	}

	return 0;
}

int msm_vidc_alloc_and_queue_session_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (buffer_type != MSM_VIDC_BUF_ARP &&
		buffer_type != MSM_VIDC_BUF_PERSIST) {
		i_vpr_e(inst, "%s: invalid buffer type: %d\n",
			__func__, buffer_type);
		rc = -EINVAL;
		goto exit;
	}

	rc = msm_vidc_get_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

	rc = msm_vidc_create_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

	rc = msm_vidc_queue_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

exit:
	return rc;
}

int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buffer, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	list_for_each_entry_safe(buffer, dummy, &buffers->list, list) {
		/* do not release already pending release buffers */
		if (buffer->attr & MSM_VIDC_ATTR_PENDING_RELEASE)
			continue;
		/* release only queued buffers */
		if (!(buffer->attr & MSM_VIDC_ATTR_QUEUED))
			continue;
		rc = venus_hfi_release_buffer(inst, buffer);
		if (rc)
			return rc;
		/* mark pending release */
		buffer->attr |= MSM_VIDC_ATTR_PENDING_RELEASE;

		i_vpr_h(inst, "%s: released buffer_type %#x, size %d\n",
			__func__, buffer_type, buffers->size);
	}

	return 0;
}

int msm_vidc_vb2_buffer_done(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int type, port;
	struct vb2_queue *q;
	struct vb2_buffer *vb2;
	struct vb2_v4l2_buffer *vbuf;
	bool found;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	type = v4l2_type_from_driver(buf->type, __func__);
	if (!type)
		return -EINVAL;
	port = v4l2_type_to_driver_port(inst, type, __func__);
	if (port < 0)
		return -EINVAL;

	q = &inst->vb2q[port];
	if (!q->streaming) {
		i_vpr_e(inst, "%s: port %d is not streaming\n",
			__func__, port);
		return -EINVAL;
	}

	found = false;
	list_for_each_entry(vb2, &q->queued_list, queued_entry) {
		if (vb2->state != VB2_BUF_STATE_ACTIVE)
			continue;
		if (vb2->index == buf->index) {
			found = true;
			break;
		}
	}
	if (!found) {
		print_vidc_buffer(VIDC_ERR, "err ", "vb2 not found for", inst, buf);
		return -EINVAL;
	}
	vbuf = to_vb2_v4l2_buffer(vb2);
	vbuf->flags = buf->flags;
	vb2->timestamp = buf->timestamp;
	vb2->planes[0].bytesused = buf->data_size;
	vb2_buffer_done(vb2, VB2_BUF_STATE_DONE);

	return 0;
}

int msm_vidc_event_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int index;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (is_decode_session(inst))
		index = 0;
	else if (is_encode_session(inst))
		index = 1;
	else
		return -EINVAL;

	v4l2_fh_init(&inst->event_handler, &core->vdev[index].vdev);
	v4l2_fh_add(&inst->event_handler);

	return rc;
}

int msm_vidc_event_queue_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	v4l2_fh_del(&inst->event_handler);
	v4l2_fh_exit(&inst->event_handler);

	return rc;
}

static int vb2q_init(struct msm_vidc_inst *inst,
	struct vb2_queue *q, enum v4l2_buf_type type)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !q || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	q->type = type;
	q->io_modes = VB2_DMABUF;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->ops = core->vb2_ops;
	q->mem_ops = core->vb2_mem_ops;
	q->drv_priv = inst;
	q->allow_zero_bytesused = 1;
	q->copy_timestamp = 1;
	rc = vb2_queue_init(q);
	if (rc)
		i_vpr_e(inst, "%s: vb2_queue_init failed for type %d\n",
				__func__, type);
	return rc;
}

int msm_vidc_vb2_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->vb2q_init) {
		i_vpr_h(inst, "%s: vb2q already inited\n", __func__);
		return 0;
	}

	rc = vb2q_init(inst, &inst->vb2q[INPUT_PORT], INPUT_MPLANE);
	if (rc)
		goto exit;

	rc = vb2q_init(inst, &inst->vb2q[OUTPUT_PORT], OUTPUT_MPLANE);
	if (rc)
		goto fail_out_vb2q_init;

	rc = vb2q_init(inst, &inst->vb2q[INPUT_META_PORT], INPUT_META_PLANE);
	if (rc)
		goto fail_in_meta_vb2q_init;

	rc = vb2q_init(inst, &inst->vb2q[OUTPUT_META_PORT], OUTPUT_META_PLANE);
	if (rc)
		goto fail_out_meta_vb2q_init;
	inst->vb2q_init = true;

	return 0;
fail_out_meta_vb2q_init:
	vb2_queue_release(&inst->vb2q[INPUT_META_PORT]);
fail_in_meta_vb2q_init:
	vb2_queue_release(&inst->vb2q[OUTPUT_PORT]);
fail_out_vb2q_init:
	vb2_queue_release(&inst->vb2q[INPUT_PORT]);
exit:
	return rc;
}

int msm_vidc_vb2_queue_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!inst->vb2q_init) {
		i_vpr_h(inst, "%s: vb2q already deinited\n", __func__);
		return 0;
	}

	vb2_queue_release(&inst->vb2q[OUTPUT_META_PORT]);
	vb2_queue_release(&inst->vb2q[INPUT_META_PORT]);
	vb2_queue_release(&inst->vb2q[OUTPUT_PORT]);
	vb2_queue_release(&inst->vb2q[INPUT_PORT]);
	inst->vb2q_init = false;

	return rc;
}

int msm_vidc_add_session(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst *i;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (!core->capabilities) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	list_for_each_entry(i, &core->instances, list)
		count++;

	if (count < core->capabilities[MAX_SESSION_COUNT].value) {
		list_add_tail(&inst->list, &core->instances);
	} else {
		i_vpr_e(inst, "%s: total sessions %d exceeded max limit %d\n",
			__func__, count, core->capabilities[MAX_SESSION_COUNT].value);
		rc = -EINVAL;
	}
	core_unlock(core, __func__);

	return rc;
}

int msm_vidc_remove_session(struct msm_vidc_inst *inst)
{
	struct msm_vidc_inst *i, *temp;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry_safe(i, temp, &core->instances, list) {
		if (i->session_id == inst->session_id) {
			list_del_init(&i->list);
			i_vpr_h(inst, "%s: removed session %#x\n",
				__func__, i->session_id);
		}
	}
	list_for_each_entry(i, &core->instances, list)
		count++;
	i_vpr_h(inst, "%s: remaining sessions %d\n", __func__, count);
	core_unlock(core, __func__);

	return 0;
}

int msm_vidc_session_open(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	inst->packet_size = 4096;
	inst->packet = kzalloc(inst->packet_size, GFP_KERNEL);
	if (!inst->packet) {
		i_vpr_e(inst, "%s(): inst packet allocation failed\n", __func__);
		return -ENOMEM;
	}

	rc = venus_hfi_session_open(inst);
	if (rc)
		goto error;

	return 0;
error:
	i_vpr_e(inst, "%s(): session open failed\n", __func__);
	kfree(inst->packet);
	inst->packet = NULL;
	return rc;
}

int msm_vidc_session_set_codec(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = venus_hfi_session_set_codec(inst);
	if (rc)
		return rc;

	return 0;
}

int msm_vidc_session_set_default_header(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 default_header = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	default_header = inst->capabilities->cap[DEFAULT_HEADER].value;
	i_vpr_h(inst, "%s: default header: %d", __func__, default_header);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_DEC_DEFAULT_HEADER,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, INPUT_PORT),
			HFI_PAYLOAD_U32,
			&default_header,
			sizeof(u32));
	if (rc)
		i_vpr_e(inst, "%s: set property failed\n", __func__);
	return rc;
}

int msm_vidc_session_streamon(struct msm_vidc_inst *inst,
		enum msm_vidc_port_type port)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	msm_vidc_scale_power(inst, true);

	rc = venus_hfi_start(inst, port);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_session_streamoff(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	int count = 0;
	struct msm_vidc_core *core;
	enum signal_session_response signal_type;
	enum msm_vidc_buffer_type buffer_type;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (port == INPUT_PORT) {
		signal_type = SIGNAL_CMD_STOP_INPUT;
		buffer_type = MSM_VIDC_BUF_INPUT;
	} else if (port == OUTPUT_PORT) {
		signal_type = SIGNAL_CMD_STOP_OUTPUT;
		buffer_type = MSM_VIDC_BUF_OUTPUT;
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_stop(inst, port);
	if (rc)
		goto error;

	core = inst->core;
	i_vpr_h(inst, "%s: wait on port: %d for time: %d ms\n",
		__func__, port, core->capabilities[HW_RESPONSE_TIMEOUT].value);
	mutex_unlock(&inst->lock);
	rc = wait_for_completion_timeout(
			&inst->completions[signal_type],
			msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	if (!rc) {
		i_vpr_e(inst, "%s: session stop timed out for port: %d\n",
				__func__, port);
		rc = -ETIMEDOUT;
		msm_vidc_core_timeout(inst->core);
	} else {
		rc = 0;
	}
	mutex_lock(&inst->lock);

	if(rc)
		goto error;

	/* no more queued buffers after streamoff */
	count = msm_vidc_num_buffers(inst, buffer_type, MSM_VIDC_ATTR_QUEUED);
	if (!count) {
		i_vpr_h(inst, "%s: stop successful on port: %d\n",
			__func__, port);
	} else {
		i_vpr_e(inst,
			"%s: %d buffers pending with firmware on port: %d\n",
			__func__, count, port);
		rc = -EINVAL;
		goto error;
	}

	/* flush deferred buffers */
	msm_vidc_flush_buffers(inst, buffer_type);
	return 0;

error:
	msm_vidc_kill_session(inst);
	msm_vidc_flush_buffers(inst, buffer_type);
	return rc;
}

int msm_vidc_session_close(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = venus_hfi_session_close(inst);
	if (rc)
		return rc;

	core = inst->core;
	i_vpr_h(inst, "%s: wait on close for time: %d ms\n",
		__func__, core->capabilities[HW_RESPONSE_TIMEOUT].value);
	mutex_unlock(&inst->lock);
	rc = wait_for_completion_timeout(
			&inst->completions[SIGNAL_CMD_CLOSE],
			msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	if (!rc) {
		i_vpr_e(inst, "%s: session close timed out\n", __func__);
		rc = -ETIMEDOUT;
		msm_vidc_core_timeout(inst->core);
	} else {
		rc = 0;
		i_vpr_h(inst, "%s: close successful\n", __func__);
	}
	mutex_lock(&inst->lock);

	msm_vidc_remove_session(inst);

	i_vpr_h(inst, "%s: free session packet data\n", __func__);
	kfree(inst->packet);
	inst->packet = NULL;

	return rc;
}

int msm_vidc_kill_session(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!inst->session_id) {
		i_vpr_e(inst, "%s: already killed\n", __func__);
		return 0;
	}

	i_vpr_e(inst, "%s: killing session\n", __func__);
	msm_vidc_session_close(inst);
	msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);

	return 0;
}

int msm_vidc_get_inst_capability(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	for (i = 0; i < core->codecs_count; i++) {
		if (core->inst_caps[i].domain == inst->domain &&
			core->inst_caps[i].codec == inst->codec) {
			i_vpr_h(inst,
				"%s: copied capabilities with %#x codec, %#x domain\n",
				__func__, inst->codec, inst->domain);
			memcpy(inst->capabilities, &core->inst_caps[i],
				sizeof(struct msm_vidc_inst_capability));
		}
	}

	return rc;
}

int msm_vidc_deinit_core_caps(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	kfree(core->capabilities);
	core->capabilities = NULL;
	d_vpr_h("%s: Core capabilities freed\n", __func__);

	return rc;
}

int msm_vidc_init_core_caps(struct msm_vidc_core *core)
{
	int rc = 0;
	int i, num_platform_caps;
	struct msm_platform_core_capability *platform_data;

	if (!core || !core->platform) {
		d_vpr_e("%s: invalid params\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	platform_data = core->platform->data.core_data;
	if (!platform_data) {
		d_vpr_e("%s: platform core data is NULL\n",
				__func__);
			rc = -EINVAL;
			goto exit;
	}

	core->capabilities = kcalloc(1,
		(sizeof(struct msm_vidc_core_capability) *
		(CORE_CAP_MAX + 1)), GFP_KERNEL);
	if (!core->capabilities) {
		d_vpr_e("%s: failed to allocate core capabilities\n",
			__func__);
		rc = -ENOMEM;
		goto exit;
	}

	num_platform_caps = core->platform->data.core_data_size;

	/* loop over platform caps */
	for (i = 0; i < num_platform_caps && i < CORE_CAP_MAX; i++) {
		core->capabilities[platform_data[i].type].type = platform_data[i].type;
		core->capabilities[platform_data[i].type].value = platform_data[i].value;
	}

exit:
	return rc;
}

static void update_inst_capability(struct msm_platform_inst_capability *in,
		struct msm_vidc_inst_capability *capability)
{
	if (!in || !capability) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, in, capability);
		return;
	}
	if (in->cap < INST_CAP_MAX) {
		capability->cap[in->cap].cap = in->cap;
		capability->cap[in->cap].min = in->min;
		capability->cap[in->cap].max = in->max;
		capability->cap[in->cap].step_or_mask = in->step_or_mask;
		capability->cap[in->cap].value = in->value;
		capability->cap[in->cap].flags = in->flags;
		capability->cap[in->cap].v4l2_id = in->v4l2_id;
		capability->cap[in->cap].hfi_id = in->hfi_id;
		memcpy(capability->cap[in->cap].parents, in->parents,
			sizeof(capability->cap[in->cap].parents));
		memcpy(capability->cap[in->cap].children, in->children,
			sizeof(capability->cap[in->cap].children));
		capability->cap[in->cap].adjust = in->adjust;
		capability->cap[in->cap].set = in->set;
	} else {
		d_vpr_e("%s: invalid cap %d\n",
			__func__, in->cap);
	}
}

int msm_vidc_deinit_instance_caps(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	kfree(core->inst_caps);
	core->inst_caps = NULL;
	d_vpr_h("%s: core->inst_caps freed\n", __func__);

	return rc;
}

int msm_vidc_init_instance_caps(struct msm_vidc_core *core)
{
	int rc = 0;
	u8 enc_valid_codecs, dec_valid_codecs;
	u8 count_bits, enc_codec_count;
	u8 codecs_count = 0;
	int i, j, check_bit, num_platform_caps;
	struct msm_platform_inst_capability *platform_data = NULL;

	if (!core || !core->platform || !core->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		rc = -EINVAL;
		goto error;
	}

	platform_data = core->platform->data.instance_data;
	if (!platform_data) {
		d_vpr_e("%s: platform instance data is NULL\n",
				__func__);
			rc = -EINVAL;
		goto error;
	}

	enc_valid_codecs = core->capabilities[ENC_CODECS].value;
	count_bits = enc_valid_codecs;
	COUNT_BITS(count_bits, codecs_count);
	enc_codec_count = codecs_count;

	dec_valid_codecs = core->capabilities[DEC_CODECS].value;
	count_bits = dec_valid_codecs;
	COUNT_BITS(count_bits, codecs_count);

	core->codecs_count = codecs_count;
	core->inst_caps = kcalloc(codecs_count,
		sizeof(struct msm_vidc_inst_capability),
		GFP_KERNEL);
	if (!core->inst_caps) {
		d_vpr_e("%s: failed to allocate core capabilities\n",
			__func__);
		rc = -ENOMEM;
		goto error;
	}

	check_bit = 0;
	/* determine codecs for enc domain */
	for (i = 0; i < enc_codec_count; i++) {
		while (check_bit < (sizeof(enc_valid_codecs) * 8)) {
			if (enc_valid_codecs & BIT(check_bit)) {
				core->inst_caps[i].domain = MSM_VIDC_ENCODER;
				core->inst_caps[i].codec = enc_valid_codecs &
						BIT(check_bit);
				check_bit++;
				break;
			}
			check_bit++;
		}
	}

	/* reset checkbit to check from 0th bit of decoder codecs set bits*/
	check_bit = 0;
	/* determine codecs for dec domain */
	for (; i < codecs_count; i++) {
		while (check_bit < (sizeof(dec_valid_codecs) * 8)) {
			if (dec_valid_codecs & BIT(check_bit)) {
				core->inst_caps[i].domain = MSM_VIDC_DECODER;
				core->inst_caps[i].codec = dec_valid_codecs &
						BIT(check_bit);
				check_bit++;
				break;
			}
			check_bit++;
		}
	}

	num_platform_caps = core->platform->data.instance_data_size;

	d_vpr_h("%s: num caps %d\n", __func__, num_platform_caps);
	/* loop over each platform capability */
	for (i = 0; i < num_platform_caps; i++) {
		/* select matching core codec and update it */
		for (j = 0; j < codecs_count; j++) {
			if ((platform_data[i].domain &
				core->inst_caps[j].domain) &&
				(platform_data[i].codec &
				core->inst_caps[j].codec)) {
				/* update core capability */
				update_inst_capability(&platform_data[i],
					&core->inst_caps[j]);
			}
		}
	}

error:
	return rc;
}

int msm_vidc_core_deinit(struct msm_vidc_core *core, bool force)
{
	int rc = 0;
	struct msm_vidc_inst *inst, *dummy;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	d_vpr_h("%s(): force %u\n", __func__, force);
	if (core->state == MSM_VIDC_CORE_DEINIT)
		goto unlock;

	if (!force)
		if (!list_empty(&core->instances))
			goto unlock;

	venus_hfi_core_deinit(core);

	/* unlink all sessions from core, if any */
	list_for_each_entry_safe(inst, dummy, &core->instances, list) {
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		list_del(&inst->list);
	}
	msm_vidc_change_core_state(core, MSM_VIDC_CORE_DEINIT, __func__);

unlock:
	core_unlock(core, __func__);
	return rc;
}

int msm_vidc_core_init(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core || !core->platform) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	if (core->state == MSM_VIDC_CORE_INIT) {
		rc = 0;
		goto unlock;
	}

	msm_vidc_change_core_state(core, MSM_VIDC_CORE_INIT, __func__);
	init_completion(&core->init_done);
	core->smmu_fault_handled = false;
	core->ssr.trigger = false;

	rc = venus_hfi_core_init(core);
	if (rc) {
		d_vpr_e("%s: core init failed\n", __func__);
		goto unlock;
	}

	d_vpr_h("%s(): waiting for sys_init_done, %d ms\n", __func__,
		core->capabilities[HW_RESPONSE_TIMEOUT].value);
	core_unlock(core, __func__);
	rc = wait_for_completion_timeout(&core->init_done, msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	core_lock(core, __func__);
	if (!rc) {
		d_vpr_e("%s: core init timed out\n", __func__);
		rc = -ETIMEDOUT;
	} else {
		d_vpr_h("%s: system init wait completed\n", __func__);
		rc = 0;
	}

unlock:
	core_unlock(core, __func__);
	if (rc)
		msm_vidc_core_deinit(core, true);
	return rc;
}

int msm_vidc_core_timeout(struct msm_vidc_core *core)
{
	return msm_vidc_core_deinit(core, true);
}

int msm_vidc_print_inst_info(struct msm_vidc_inst *inst)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	enum msm_vidc_port_type port;
	bool is_secure, is_decode;
	u32 bit_depth, bit_rate, frame_rate, width, height;
	struct dma_buf *dbuf;
	int i = 0;

	static const struct msm_vidc_buf_type_name buf_type_name[] = {
		{MSM_VIDC_BUF_INPUT,             "INPUT"      },
		{MSM_VIDC_BUF_OUTPUT,            "OUTPUT"     },
		{MSM_VIDC_BUF_INPUT_META,        "IN_META"    },
		{MSM_VIDC_BUF_OUTPUT_META,       "OUT_META"   },
		{MSM_VIDC_BUF_BIN,               "BIN"        },
		{MSM_VIDC_BUF_ARP,               "ARP"        },
		{MSM_VIDC_BUF_COMV,              "COMV"       },
		{MSM_VIDC_BUF_NON_COMV,          "NON_COMV"   },
		{MSM_VIDC_BUF_LINE,              "LINE"       },
		{MSM_VIDC_BUF_PERSIST,           "PERSIST"    },
		{MSM_VIDC_BUF_VPSS,              "VPSS"       },
	};

	if (!inst || !inst->capabilities) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	is_secure = is_secure_session(inst);
	is_decode = inst->domain == MSM_VIDC_DECODER;
	port = is_decode ? INPUT_PORT : OUTPUT_PORT;
	width = inst->fmts[port].fmt.pix_mp.width;
	height = inst->fmts[port].fmt.pix_mp.height;
	bit_depth = inst->capabilities->cap[BIT_DEPTH].value & 0xFFFF;
	bit_rate = inst->capabilities->cap[BIT_RATE].value;
	frame_rate = inst->capabilities->cap[FRAME_RATE].value >> 16;

	i_vpr_e(inst, "%s %s session, HxW: %d x %d, fps: %d, bitrate: %d, bit-depth: %d\n",
		is_secure ? "Secure" : "Non-Secure",
		is_decode ? "Decode" : "Encode",
		height, width,
		frame_rate, bit_rate, bit_depth);

	/* Print buffer details */
	for (i = 0; i < ARRAY_SIZE(buf_type_name); i++) {
		buffers = msm_vidc_get_buffers(inst, buf_type_name[i].type, __func__);
		if (!buffers)
			continue;

		i_vpr_e(inst, "count: type: %8s, min: %2d, extra: %2d, actual: %2d\n",
			buf_type_name[i].name, buffers->min_count,
			buffers->extra_count, buffers->actual_count);

		list_for_each_entry(buf, &buffers->list, list) {
			if (!buf->valid || !buf->dmabuf)
				continue;
			dbuf = (struct dma_buf *)buf->dmabuf;
			i_vpr_e(inst,
				"buf: type: %8s, index: %2d, fd: %4d, size: %9u, off: %8u, filled: %9u, iova: %8x, inode: %9ld, flags: %8x, ts: %16lld, attr: %8x\n",
				buf_type_name[i].name, buf->index, buf->fd, buf->buffer_size,
				buf->data_offset, buf->data_size, buf->device_addr,
				file_inode(dbuf->file)->i_ino,
				buf->flags, buf->timestamp, buf->attr);
		}
	}

	return 0;
}

void msm_vidc_print_core_info(struct msm_vidc_core *core)
{
	struct msm_vidc_inst *inst = NULL;
	struct msm_vidc_inst *instances[MAX_SUPPORTED_INSTANCES];
	s32 num_instances = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	core_lock(core, __func__);
	list_for_each_entry(inst, &core->instances, list)
		instances[num_instances++] = inst;
	core_unlock(core, __func__);

	while (num_instances--) {
		inst = instances[num_instances];
		inst = get_inst_ref(core, inst);
		if (!inst)
			continue;
		inst_lock(inst, __func__);
		msm_vidc_print_inst_info(inst);
		inst_unlock(inst, __func__);
		put_inst(inst);
	}
}

int msm_vidc_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data)
{
	struct msm_vidc_core *core = data;

	if (!domain || !core || !core->capabilities) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, domain, core);
		return -EINVAL;
	}

	if (core->smmu_fault_handled) {
		if (core->capabilities[NON_FATAL_FAULTS].value) {
			dprintk_ratelimit(VIDC_ERR, "err ",
					"%s: non-fatal pagefault address: %lx\n",
					__func__, iova);
			return 0;
		}
	}

	d_vpr_e("%s: faulting address: %lx\n", __func__, iova);

	core->smmu_fault_handled = true;

	/* print noc error log registers */
	venus_hfi_noc_error_info(core);

	msm_vidc_print_core_info(core);
	/*
	 * Return -ENOSYS to elicit the default behaviour of smmu driver.
	 * If we return -ENOSYS, then smmu driver assumes page fault handler
	 * is not installed and prints a list of useful debug information like
	 * FAR, SID etc. This information is not printed if we return 0.
	 */
	return -ENOSYS;
}

int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
		u64 trigger_ssr_val)
{
	struct msm_vidc_ssr *ssr;

	if (!core) {
		d_vpr_e("%s: Invalid parameters\n", __func__);
		return -EINVAL;
	}
	ssr = &core->ssr;
	/*
	 * <test_addr><sub_client_id><ssr_type>
	 * ssr_type: 0-3 bits
	 * sub_client_id: 4-7 bits
	 * reserved: 8-31 bits
	 * test_addr: 32-63 bits
	 */
	ssr->ssr_type = (trigger_ssr_val &
			(unsigned long)SSR_TYPE) >> SSR_TYPE_SHIFT;
	ssr->sub_client_id = (trigger_ssr_val &
			(unsigned long)SSR_SUB_CLIENT_ID) >> SSR_SUB_CLIENT_ID_SHIFT;
	ssr->test_addr = (trigger_ssr_val &
			(unsigned long)SSR_ADDR_ID) >> SSR_ADDR_SHIFT;
	schedule_work(&core->ssr_work);
	return 0;
}

void msm_vidc_ssr_handler(struct work_struct *work)
{
	int rc;
	struct msm_vidc_core *core;
	struct msm_vidc_ssr *ssr;

	core = container_of(work, struct msm_vidc_core, ssr_work);
	if (!core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, core);
		return;
	}
	ssr = &core->ssr;

	core_lock(core, __func__);
	if (core->state == MSM_VIDC_CORE_INIT) {
		/*
		 * In current implementation, user-initiated SSR triggers
		 * a fatal error from hardware. However, there is no way
		 * to know if fatal error is due to SSR or not. Handle
		 * user SSR as non-fatal.
		 */
		core->ssr.trigger = true;
		rc = venus_hfi_trigger_ssr(core, ssr->ssr_type,
			ssr->sub_client_id, ssr->test_addr);
		if (rc) {
			d_vpr_e("%s: trigger_ssr failed\n", __func__);
			core->ssr.trigger = false;
		}
	} else {
		d_vpr_e("%s: video core not initialized\n", __func__);
	}
	core_unlock(core, __func__);
}

void msm_vidc_pm_work_handler(struct work_struct *work)
{
}

void msm_vidc_fw_unload_handler(struct work_struct *work)
{
	struct msm_vidc_core *core = NULL;
	int rc = 0;

	core = container_of(work, struct msm_vidc_core, fw_unload_work.work);
	if (!core) {
		d_vpr_e("%s: invalid work or core handle\n", __func__);
		return;
	}

	d_vpr_h("%s: deinitializing video core\n",__func__);
	rc = msm_vidc_core_deinit(core, false);
	if (rc)
		d_vpr_e("%s: Failed to deinit core\n", __func__);

}

void msm_vidc_batch_handler(struct work_struct *work)
{
	struct msm_vidc_inst *inst;
	enum msm_vidc_allow allow;
	int rc = 0;

	inst = container_of(work, struct msm_vidc_inst, decode_batch.work.work);
	inst = get_inst_ref(g_core, inst);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	inst_lock(inst, __func__);
	if (is_session_error(inst)) {
		i_vpr_e(inst, "%s: failled. Session error\n", __func__);
		goto exit;
	}

	allow = msm_vidc_allow_qbuf(inst, OUTPUT_MPLANE);
	if (allow != MSM_VIDC_ALLOW) {
		i_vpr_e(inst, "%s: not allowed in state: %s\n", __func__,
			state_name(inst->state));
		goto exit;
	}

	i_vpr_h(inst, "%s: queue pending batch buffers\n", __func__);
	rc = msm_vidc_queue_buffer_batch(inst);
	if (rc) {
		i_vpr_e(inst, "%s: batch qbufs failed\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

exit:
	inst_unlock(inst, __func__);
	put_inst(inst);
}

int msm_vidc_flush_buffers(struct msm_vidc_inst* inst,
		enum msm_vidc_buffer_type type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf, *dummy;
	enum msm_vidc_buffer_type buffer_type[2];
	int i;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == MSM_VIDC_BUF_INPUT) {
		buffer_type[0] = MSM_VIDC_BUF_INPUT_META;
		buffer_type[1] = MSM_VIDC_BUF_INPUT;
	} else if (type == MSM_VIDC_BUF_OUTPUT) {
		buffer_type[0] = MSM_VIDC_BUF_OUTPUT_META;
		buffer_type[1] = MSM_VIDC_BUF_OUTPUT;
	} else {
		i_vpr_h(inst, "%s: invalid buffer type %d\n",
				__func__, type);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(buffer_type); i++) {
		buffers = msm_vidc_get_buffers(inst, buffer_type[i], __func__);
		if (!buffers)
			return -EINVAL;

		list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
			if (buf->attr & MSM_VIDC_ATTR_QUEUED ||
				buf->attr & MSM_VIDC_ATTR_DEFERRED) {
				print_vidc_buffer(VIDC_ERR, "err ", "flushing buffer", inst, buf);
				msm_vidc_vb2_buffer_done(inst, buf);
				msm_vidc_put_driver_buf(inst, buf);
			}
		}
	}

	return rc;
}

void msm_vidc_destroy_buffers(struct msm_vidc_inst *inst)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf, *dummy;
	static const enum msm_vidc_buffer_type buf_types[] = {
		MSM_VIDC_BUF_INPUT,
		MSM_VIDC_BUF_OUTPUT,
		MSM_VIDC_BUF_INPUT_META,
		MSM_VIDC_BUF_OUTPUT_META,
		MSM_VIDC_BUF_BIN,
		MSM_VIDC_BUF_ARP,
		MSM_VIDC_BUF_COMV,
		MSM_VIDC_BUF_NON_COMV,
		MSM_VIDC_BUF_LINE,
		MSM_VIDC_BUF_DPB,
		MSM_VIDC_BUF_PERSIST,
		MSM_VIDC_BUF_VPSS,
	};
	int i;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(buf_types); i++) {
		buffers = msm_vidc_get_buffers(inst, buf_types[i], __func__);
		if (!buffers)
			continue;
		list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
			i_vpr_h(inst,
				"destroying buffer: type %d idx %d fd %d addr %#x size %d\n",
				buf->type, buf->index, buf->fd, buf->device_addr, buf->buffer_size);
			if (is_internal_buffer(buf->type))
				msm_vidc_destroy_internal_buffer(inst, buf);
			else
				msm_vidc_put_driver_buf(inst, buf);
		}
	}
}

static void msm_vidc_close_helper(struct kref *kref)
{
	struct msm_vidc_inst *inst = container_of(kref,
		struct msm_vidc_inst, kref);

	i_vpr_h(inst, "%s()\n", __func__);
	msm_vidc_event_queue_deinit(inst);
	msm_vidc_vb2_queue_deinit(inst);
	msm_vidc_debugfs_deinit_inst(inst);
	if (is_decode_session(inst))
		msm_vdec_inst_deinit(inst);
	else if (is_encode_session(inst))
		msm_venc_inst_deinit(inst);
	msm_vidc_free_input_cr_list(inst);
	kfree(inst->capabilities);
	if (inst->response_workq)
		destroy_workqueue(inst->response_workq);
	kfree(inst);
}

struct msm_vidc_inst *get_inst_ref(struct msm_vidc_core *core,
		struct msm_vidc_inst *instance)
{
	struct msm_vidc_inst *inst = NULL;
	bool matches = false;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (inst == instance) {
			matches = true;
			break;
		}
	}
	inst = (matches && kref_get_unless_zero(&inst->kref)) ? inst : NULL;
	mutex_unlock(&core->lock);
	return inst;
}

struct msm_vidc_inst *get_inst(struct msm_vidc_core *core,
		u32 session_id)
{
	struct msm_vidc_inst *inst = NULL;
	bool matches = false;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (inst->session_id == session_id) {
			matches = true;
			break;
		}
	}
	inst = (matches && kref_get_unless_zero(&inst->kref)) ? inst : NULL;
	mutex_unlock(&core->lock);
	return inst;
}

void put_inst(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	kref_put(&inst->kref, msm_vidc_close_helper);
}

bool core_lock_check(struct msm_vidc_core *core, const char* func)
{
	return mutex_is_locked(&core->lock);
}

void core_lock(struct msm_vidc_core *core, const char *function)
{
	mutex_lock(&core->lock);
}

void core_unlock(struct msm_vidc_core *core, const char *function)
{
	mutex_unlock(&core->lock);
}

bool inst_lock_check(struct msm_vidc_inst *inst, const char* func)
{
	return mutex_is_locked(&inst->lock);
}

void inst_lock(struct msm_vidc_inst *inst, const char *function)
{
	mutex_lock(&inst->lock);
}

void inst_unlock(struct msm_vidc_inst *inst, const char *function)
{
	mutex_unlock(&inst->lock);
}

int msm_vidc_update_bitstream_buffer_size(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (is_decode_session(inst)) {
		fmt = &inst->fmts[INPUT_PORT];
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_INPUT);
	}

	return 0;
}

int msm_vidc_update_meta_port_settings(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[INPUT_META_PORT];
	if (is_input_meta_enabled(inst)) {
		fmt->fmt.meta.buffersize = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_INPUT_META);
		inst->buffers.input_meta.min_count =
				inst->buffers.input.min_count;
		inst->buffers.input_meta.extra_count =
				inst->buffers.input.extra_count;
		inst->buffers.input_meta.actual_count =
				inst->buffers.input.actual_count;
		inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
	} else {
		fmt->fmt.meta.buffersize = 0;
		inst->buffers.input_meta.min_count = 0;
		inst->buffers.input_meta.extra_count = 0;
		inst->buffers.input_meta.actual_count = 0;
		inst->buffers.input_meta.size = 0;
	}

	fmt = &inst->fmts[OUTPUT_META_PORT];
	if (is_output_meta_enabled(inst)) {
		fmt->fmt.meta.buffersize = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
		inst->buffers.output_meta.min_count =
				inst->buffers.output.min_count;
		inst->buffers.output_meta.extra_count =
				inst->buffers.output.extra_count;
		inst->buffers.output_meta.actual_count =
				inst->buffers.output.actual_count;
		inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
	} else {
		fmt->fmt.meta.buffersize = 0;
		inst->buffers.output_meta.min_count = 0;
		inst->buffers.output_meta.extra_count = 0;
		inst->buffers.output_meta.actual_count = 0;
		inst->buffers.output_meta.size = 0;
	}
	return 0;
}

void msm_vidc_schedule_core_deinit(struct msm_vidc_core *core)
{
	if (!core)
		return;

	if (!core->capabilities[FW_UNLOAD].value)
		return;

	cancel_delayed_work(&core->fw_unload_work);

	schedule_delayed_work(&core->fw_unload_work,
		msecs_to_jiffies(core->capabilities[FW_UNLOAD_DELAY].value));

	d_vpr_h("firmware unload delayed by %u ms\n",
		core->capabilities[FW_UNLOAD_DELAY].value);

	return;
}

static const char *get_codec_str(enum msm_vidc_codec_type type)
{
	switch (type) {
	case MSM_VIDC_H264: return "h264";
	case MSM_VIDC_HEVC: return "h265";
	case MSM_VIDC_VP9:  return " vp9";
	case MSM_VIDC_HEIC: return "heic";
	}

	return "....";
}

static const char *get_domain_str(enum msm_vidc_domain_type type)
{
	switch (type) {
	case MSM_VIDC_ENCODER: return "e";
	case MSM_VIDC_DECODER: return "d";
	}

	return ".";
}

int msm_vidc_update_debug_str(struct msm_vidc_inst *inst)
{
	u32 sid;
	const char *codec;
	const char *domain;

	if (!inst) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	sid = inst->session_id;
	codec = get_codec_str(inst->codec);
	domain = get_domain_str(inst->domain);
	snprintf(inst->debug_str, sizeof(inst->debug_str), "%08x: %s%s", sid, codec, domain);
	d_vpr_h("%s: sid: %08x, codec: %s, domain: %s, final: %s\n",
		__func__, sid, codec, domain, inst->debug_str);

	return 0;
}

int msm_vidc_check_mbps_supported(struct msm_vidc_inst *inst)
{
	u32 mbps = 0;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *instance;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry(instance, &core->instances, list) {
		/* ignore invalid/error session */
		if (instance->state == MSM_VIDC_ERROR)
			continue;

		/* ignore thumbnail, image, and non realtime sessions */
		if (is_thumbnail_session(instance) ||
			is_image_session(instance) ||
			!is_realtime_session(instance))
			continue;

		mbps += msm_vidc_get_inst_load(instance);
	}
	core_unlock(core, __func__);

	if (mbps > core->capabilities[MAX_MBPS].value) {
		/* todo: print running instances */
		//msm_vidc_print_running_insts(inst->core);
		return -ENOMEM;
	}

	return 0;
}

static int msm_vidc_check_mbpf_supported(struct msm_vidc_inst *inst)
{
	u32 mbpf = 0;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *instance;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry(instance, &core->instances, list) {
		/* ignore invalid/error session */
		if (instance->state == MSM_VIDC_ERROR)
			continue;

		/* ignore thumbnail, image, and non realtime sessions */
		if (is_thumbnail_session(instance) ||
			is_image_session(instance) ||
			!is_realtime_session(instance))
			continue;

		mbpf += msm_vidc_get_mbs_per_frame(instance);
	}
	core_unlock(core, __func__);

	if (mbpf > core->capabilities[MAX_MBPF].value) {
		/* todo: print running instances */
		//msm_vidc_print_running_insts(inst->core);
		return -ENOMEM;
	}

	return 0;
}

int msm_vidc_check_session_supported(struct msm_vidc_inst *inst)
{
	struct msm_vidc_inst_capability *capability;
	struct v4l2_format *fmt;
	u32 pix_fmt, profile;
	bool allow = false;
	int rc = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	/* todo: enable checks for all session type */
	if (!is_image_session(inst))
		return 0;

	pix_fmt = capability->cap[PIX_FMTS].value;
	profile = capability->cap[PROFILE].value;

	if (is_image_encode_session(inst)) {
		/* is linear color fmt */
		allow = is_linear_colorformat(pix_fmt);
		if (!allow) {
			i_vpr_e(inst, "%s: compressed fmt: %#x\n", __func__, pix_fmt);
			goto exit;
		}

		/* is input grid aligned */
		fmt = &inst->fmts[INPUT_PORT];
		allow = IS_ALIGNED(fmt->fmt.pix_mp.width, HEIC_GRID_DIMENSION);
		allow &= IS_ALIGNED(fmt->fmt.pix_mp.height, HEIC_GRID_DIMENSION);
		if (!allow) {
			i_vpr_e(inst, "%s: input is not grid aligned: %u x %u\n", __func__,
				fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height);
			goto exit;
		}

		/* is output grid dimension */
		fmt = &inst->fmts[OUTPUT_PORT];
		allow = fmt->fmt.pix_mp.width == HEIC_GRID_DIMENSION;
		allow &= fmt->fmt.pix_mp.height == HEIC_GRID_DIMENSION;
		if (!allow) {
			i_vpr_e(inst, "%s: output is not a grid dimension: %u x %u\n", __func__,
				fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height);
			goto exit;
		}

		/* is bitrate mode CQ */
		allow = capability->cap[BITRATE_MODE].value == HFI_RC_CQ;
		if (!allow) {
			i_vpr_e(inst, "%s: bitrate mode is not CQ: %#x\n", __func__,
				capability->cap[BITRATE_MODE].value);
			goto exit;
		}

		/* is all intra */
		allow = !capability->cap[GOP_SIZE].value;
		allow &= !capability->cap[B_FRAME].value;
		if (!allow) {
			i_vpr_e(inst, "%s: not all intra: gop: %u, bframe: %u\n", __func__,
				capability->cap[GOP_SIZE].value, capability->cap[B_FRAME].value);
			goto exit;
		}

		/* is time delta based rc disabled */
		allow = !capability->cap[TIME_DELTA_BASED_RC].value;
		if (!allow) {
			i_vpr_e(inst, "%s: time delta based rc not disabled: %#x\n", __func__,
				capability->cap[TIME_DELTA_BASED_RC].value);
			goto exit;
		}

		/* is profile type Still Pic */
		if (is_10bit_colorformat(pix_fmt))
			allow = profile == V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10;
		else
			allow = profile == V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_STILL_PICTURE;
		if (!allow) {
			i_vpr_e(inst, "%s: profile not valid: %#x\n", __func__,
				capability->cap[PROFILE].value);
			goto exit;
		}
	}

	rc = msm_vidc_check_mbps_supported(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_check_mbpf_supported(inst);
	if (rc)
		goto exit;

	/* todo: add additional checks related to capabilities */

	return 0;

exit:
	i_vpr_e(inst, "%s: current session not supported\n", __func__);
	return -EINVAL;
}

int msm_vidc_check_scaling_supported(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_image_session(inst) || is_decode_session(inst)) {
		i_vpr_h(inst, "%s: Scaling is supported for encode session only\n", __func__);
		return 0;
	}

	/* todo: add scaling check for encode session */
	return 0;
}


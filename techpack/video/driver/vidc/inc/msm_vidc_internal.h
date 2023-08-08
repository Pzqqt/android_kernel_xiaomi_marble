/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _MSM_VIDC_INTERNAL_H_
#define _MSM_VIDC_INTERNAL_H_

#include <linux/version.h>
#include <linux/bits.h>
#include <linux/workqueue.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ctrls.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>

#define MAX_NAME_LENGTH   128
#define VENUS_VERSION_LENGTH 128
#define MAX_MATRIX_COEFFS 9
#define MAX_BIAS_COEFFS   3
#define MAX_LIMIT_COEFFS  6
#define MAX_DEBUGFS_NAME  50
#define DEFAULT_HEIGHT    240
#define DEFAULT_WIDTH     320
#define DEFAULT_FPS       30
#define MAXIMUM_VP9_FPS   60
#define MAX_SUPPORTED_INSTANCES  16
#define DEFAULT_BSE_VPP_DELAY    2
#define MAX_CAP_PARENTS          20
#define MAX_CAP_CHILDREN         20
#define DEFAULT_MAX_HOST_BUF_COUNT  64
#define DEFAULT_MAX_HOST_BURST_BUF_COUNT 256
#define BIT_DEPTH_8 (8 << 16 | 8)
#define BIT_DEPTH_10 (10 << 16 | 10)
#define CODED_FRAMES_PROGRESSIVE 0x0
#define CODED_FRAMES_INTERLACE 0x1
#define MAX_VP9D_INST_COUNT     6
/* TODO: move below macros to waipio.c */
#define MAX_ENH_LAYER_HB        3
#define MAX_HEVC_ENH_LAYER_SLIDING_WINDOW     5
#define MAX_AVC_ENH_LAYER_SLIDING_WINDOW      3
#define MAX_AVC_ENH_LAYER_HYBRID_HP           5
#define INVALID_DEFAULT_MARK_OR_USE_LTR      -1
#define MAX_SLICES_PER_FRAME                 10
#define MAX_SLICES_FRAME_RATE                60
#define MAX_MB_SLICE_WIDTH                 4096
#define MAX_MB_SLICE_HEIGHT                2160
#define MAX_BYTES_SLICE_WIDTH              1920
#define MAX_BYTES_SLICE_HEIGHT             1088
#define MIN_HEVC_SLICE_WIDTH                384
#define MIN_AVC_SLICE_WIDTH                 192
#define MIN_SLICE_HEIGHT                    128
#define MAX_SUPPORTED_MIN_QUALITY            70
#define MIN_CHROMA_QP_OFFSET                -12
#define MAX_CHROMA_QP_OFFSET                  0

#define DCVS_WINDOW 16
#define ENC_FPS_WINDOW 3
#define DEC_FPS_WINDOW 10

#define INPUT_MPLANE V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define OUTPUT_MPLANE V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
#define INPUT_META_PLANE V4L2_BUF_TYPE_META_OUTPUT
#define OUTPUT_META_PLANE V4L2_BUF_TYPE_META_CAPTURE

#define VIDC_IFACEQ_MAX_PKT_SIZE                1024
#define VIDC_IFACEQ_MED_PKT_SIZE                768
#define VIDC_IFACEQ_MIN_PKT_SIZE                8
#define VIDC_IFACEQ_VAR_SMALL_PKT_SIZE          100
#define VIDC_IFACEQ_VAR_LARGE_PKT_SIZE          512
#define VIDC_IFACEQ_VAR_HUGE_PKT_SIZE          (1024*12)

#define NUM_MBS_PER_SEC(__height, __width, __fps) \
	(NUM_MBS_PER_FRAME(__height, __width) * __fps)

#define NUM_MBS_PER_FRAME(__height, __width) \
	((ALIGN(__height, 16) / 16) * (ALIGN(__width, 16) / 16))

#ifdef V4L2_CTRL_CLASS_CODEC
#define IS_PRIV_CTRL(idx) ( \
	(V4L2_CTRL_ID2WHICH(idx) == V4L2_CTRL_CLASS_CODEC) && \
	V4L2_CTRL_DRIVER_PRIV(idx))
#else
#define IS_PRIV_CTRL(idx) ( \
	(V4L2_CTRL_ID2WHICH(idx) == V4L2_CTRL_CLASS_MPEG) && \
	V4L2_CTRL_DRIVER_PRIV(idx))
#endif

#define BUFFER_ALIGNMENT_SIZE(x) x
#define NUM_MBS_360P (((480 + 15) >> 4) * ((360 + 15) >> 4))
#define NUM_MBS_720P (((1280 + 15) >> 4) * ((720 + 15) >> 4))
#define NUM_MBS_FHD (((1920 + 15) >> 4) * ((1080 + 15) >> 4))
#define NUM_MBS_4k (((4096 + 15) >> 4) * ((2304 + 15) >> 4))
#define MB_SIZE_IN_PIXEL (16 * 16)

#define DB_H264_DISABLE_SLICE_BOUNDARY \
		V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY

#define DB_HEVC_DISABLE_SLICE_BOUNDARY \
		V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY

/*
 * Convert Q16 number into Integer and Fractional part upto 2 places.
 * Ex : 105752 / 65536 = 1.61; 1.61 in Q16 = 105752;
 * Integer part =  105752 / 65536 = 1;
 * Reminder = 105752 * 0xFFFF = 40216; Last 16 bits.
 * Fractional part = 40216 * 100 / 65536 = 61;
 * Now convert to FP(1, 61, 100).
 */
#define Q16_INT(q) ((q) >> 16)
#define Q16_FRAC(q) ((((q) & 0xFFFF) * 100) >> 16)

/* define timeout values */
#define HW_RESPONSE_TIMEOUT_VALUE     (1000)
#define SW_PC_DELAY_VALUE             (HW_RESPONSE_TIMEOUT_VALUE + 500)
#define FW_UNLOAD_DELAY_VALUE         (SW_PC_DELAY_VALUE + 1500)

#define MAX_MAP_OUTPUT_COUNT 64
#define MAX_DPB_COUNT 32
 /*
  * max dpb count in firmware = 16
  * each dpb: 4 words - <base_address, addr_offset, data_offset>
  * dpb list array size = 16 * 4
  * dpb payload size = 16 * 4 * 4
  */
#define MAX_DPB_LIST_ARRAY_SIZE (16 * 4)
#define MAX_DPB_LIST_PAYLOAD_SIZE (16 * 4 * 4)

enum msm_vidc_domain_type {
	MSM_VIDC_ENCODER           = BIT(0),
	MSM_VIDC_DECODER           = BIT(1),
};

enum msm_vidc_codec_type {
	MSM_VIDC_H264              = BIT(0),
	MSM_VIDC_HEVC              = BIT(1),
	MSM_VIDC_VP9               = BIT(2),
	MSM_VIDC_HEIC              = BIT(3),
};

enum priority_level {
	MSM_VIDC_PRIORITY_HIGH     = 0,
	MSM_VIDC_PRIORITY_LOW      = 1,
};

enum msm_vidc_colorformat_type {
	MSM_VIDC_FMT_NONE          = 0,
	MSM_VIDC_FMT_NV12C         = BIT(0),
	MSM_VIDC_FMT_NV12          = BIT(1),
	MSM_VIDC_FMT_NV21          = BIT(2),
	MSM_VIDC_FMT_TP10C         = BIT(3),
	MSM_VIDC_FMT_P010          = BIT(4),
	MSM_VIDC_FMT_RGBA8888C     = BIT(5),
	MSM_VIDC_FMT_RGBA8888      = BIT(6),
};

enum msm_vidc_buffer_type {
	MSM_VIDC_BUF_INPUT                 = 1,
	MSM_VIDC_BUF_OUTPUT                = 2,
	MSM_VIDC_BUF_INPUT_META            = 3,
	MSM_VIDC_BUF_OUTPUT_META           = 4,
	MSM_VIDC_BUF_READ_ONLY             = 5,
	MSM_VIDC_BUF_QUEUE                 = 6,
	MSM_VIDC_BUF_BIN                   = 7,
	MSM_VIDC_BUF_ARP                   = 8,
	MSM_VIDC_BUF_COMV                  = 9,
	MSM_VIDC_BUF_NON_COMV              = 10,
	MSM_VIDC_BUF_LINE                  = 11,
	MSM_VIDC_BUF_DPB                   = 12,
	MSM_VIDC_BUF_PERSIST               = 13,
	MSM_VIDC_BUF_VPSS                  = 14,
};

/* always match with v4l2 flags V4L2_BUF_FLAG_* */
enum msm_vidc_buffer_flags {
	MSM_VIDC_BUF_FLAG_KEYFRAME         = 0x00000008,
	MSM_VIDC_BUF_FLAG_PFRAME           = 0x00000010,
	MSM_VIDC_BUF_FLAG_BFRAME           = 0x00000020,
	MSM_VIDC_BUF_FLAG_ERROR            = 0x00000040,
	MSM_VIDC_BUF_FLAG_LAST             = 0x00100000,
	MSM_VIDC_BUF_FLAG_CODECCONFIG      = 0x01000000,
	MSM_VIDC_BUF_FLAG_SUBFRAME         = 0x02000000,
};

enum msm_vidc_buffer_attributes {
	MSM_VIDC_ATTR_DEFERRED                  = BIT(0),
	MSM_VIDC_ATTR_READ_ONLY                 = BIT(1),
	MSM_VIDC_ATTR_PENDING_RELEASE           = BIT(2),
	MSM_VIDC_ATTR_QUEUED                    = BIT(3),
	MSM_VIDC_ATTR_DEQUEUED                  = BIT(4),
	MSM_VIDC_ATTR_BUFFER_DONE               = BIT(5),
};

enum msm_vidc_buffer_region {
	MSM_VIDC_REGION_NONE = 0,
	MSM_VIDC_NON_SECURE,
	MSM_VIDC_NON_SECURE_PIXEL,
	MSM_VIDC_SECURE_PIXEL,
	MSM_VIDC_SECURE_NONPIXEL,
	MSM_VIDC_SECURE_BITSTREAM,
};

enum msm_vidc_port_type {
	INPUT_PORT = 0,
	OUTPUT_PORT,
	INPUT_META_PORT,
	OUTPUT_META_PORT,
	PORT_NONE,
	MAX_PORT,
};

enum msm_vidc_stage_type {
	MSM_VIDC_STAGE_NONE = 0,
	MSM_VIDC_STAGE_1 = 1,
	MSM_VIDC_STAGE_2 = 2,
};

enum msm_vidc_pipe_type {
	MSM_VIDC_PIPE_NONE = 0,
	MSM_VIDC_PIPE_1 = 1,
	MSM_VIDC_PIPE_2 = 2,
	MSM_VIDC_PIPE_4 = 4,
};

enum msm_vidc_quality_mode {
	MSM_VIDC_MAX_QUALITY_MODE = 0x1,
	MSM_VIDC_POWER_SAVE_MODE = 0x2,
};

enum msm_vidc_color_primaries {
	MSM_VIDC_PRIMARIES_RESERVED                         = 0,
	MSM_VIDC_PRIMARIES_BT709                            = 1,
	MSM_VIDC_PRIMARIES_UNSPECIFIED                      = 2,
	MSM_VIDC_PRIMARIES_BT470_SYSTEM_M                   = 4,
	MSM_VIDC_PRIMARIES_BT470_SYSTEM_BG                  = 5,
	MSM_VIDC_PRIMARIES_BT601_525                        = 6,
	MSM_VIDC_PRIMARIES_SMPTE_ST240M                     = 7,
	MSM_VIDC_PRIMARIES_GENERIC_FILM                     = 8,
	MSM_VIDC_PRIMARIES_BT2020                           = 9,
	MSM_VIDC_PRIMARIES_SMPTE_ST428_1                    = 10,
	MSM_VIDC_PRIMARIES_SMPTE_RP431_2                    = 11,
	MSM_VIDC_PRIMARIES_SMPTE_EG431_1                    = 12,
	MSM_VIDC_PRIMARIES_SMPTE_EBU_TECH                   = 22,
};

enum msm_vidc_transfer_characteristics {
	MSM_VIDC_TRANSFER_RESERVED                          = 0,
	MSM_VIDC_TRANSFER_BT709                             = 1,
	MSM_VIDC_TRANSFER_UNSPECIFIED                       = 2,
	MSM_VIDC_TRANSFER_BT470_SYSTEM_M                    = 4,
	MSM_VIDC_TRANSFER_BT470_SYSTEM_BG                   = 5,
	MSM_VIDC_TRANSFER_BT601_525_OR_625                  = 6,
	MSM_VIDC_TRANSFER_SMPTE_ST240M                      = 7,
	MSM_VIDC_TRANSFER_LINEAR                            = 8,
	MSM_VIDC_TRANSFER_LOG_100_1                         = 9,
	MSM_VIDC_TRANSFER_LOG_SQRT                          = 10,
	MSM_VIDC_TRANSFER_XVYCC                             = 11,
	MSM_VIDC_TRANSFER_BT1361_0                          = 12,
	MSM_VIDC_TRANSFER_SRGB_SYCC                         = 13,
	MSM_VIDC_TRANSFER_BT2020_14                         = 14,
	MSM_VIDC_TRANSFER_BT2020_15                         = 15,
	MSM_VIDC_TRANSFER_SMPTE_ST2084_PQ                   = 16,
	MSM_VIDC_TRANSFER_SMPTE_ST428_1                     = 17,
	MSM_VIDC_TRANSFER_BT2100_2_HLG                      = 18,
};

enum msm_vidc_matrix_coefficients {
	MSM_VIDC_MATRIX_COEFF_SRGB_SMPTE_ST428_1             = 0,
	MSM_VIDC_MATRIX_COEFF_BT709                          = 1,
	MSM_VIDC_MATRIX_COEFF_UNSPECIFIED                    = 2,
	MSM_VIDC_MATRIX_COEFF_RESERVED                       = 3,
	MSM_VIDC_MATRIX_COEFF_FCC_TITLE_47                   = 4,
	MSM_VIDC_MATRIX_COEFF_BT470_SYS_BG_OR_BT601_625      = 5,
	MSM_VIDC_MATRIX_COEFF_BT601_525_BT1358_525_OR_625    = 6,
	MSM_VIDC_MATRIX_COEFF_SMPTE_ST240                    = 7,
	MSM_VIDC_MATRIX_COEFF_YCGCO                          = 8,
	MSM_VIDC_MATRIX_COEFF_BT2020_NON_CONSTANT            = 9,
	MSM_VIDC_MATRIX_COEFF_BT2020_CONSTANT                = 10,
	MSM_VIDC_MATRIX_COEFF_SMPTE_ST2085                   = 11,
	MSM_VIDC_MATRIX_COEFF_SMPTE_CHROM_DERV_NON_CONSTANT  = 12,
	MSM_VIDC_MATRIX_COEFF_SMPTE_CHROM_DERV_CONSTANT      = 13,
	MSM_VIDC_MATRIX_COEFF_BT2100                         = 14,
};

enum msm_vidc_ctrl_list_type {
	CHILD_LIST          = BIT(0),
	FW_LIST             = BIT(1),
};

enum msm_vidc_core_capability_type {
	CORE_CAP_NONE = 0,
	ENC_CODECS,
	DEC_CODECS,
	MAX_SESSION_COUNT,
	MAX_NUM_720P_SESSIONS,
	MAX_NUM_1080P_SESSIONS,
	MAX_NUM_4K_SESSIONS,
	MAX_NUM_8K_SESSIONS,
	MAX_SECURE_SESSION_COUNT,
	MAX_LOAD,
	MAX_RT_MBPF,
	MAX_MBPF,
	MAX_MBPS,
	MAX_IMAGE_MBPF,
	MAX_MBPF_HQ,
	MAX_MBPS_HQ,
	MAX_MBPF_B_FRAME,
	MAX_MBPS_B_FRAME,
	MAX_MBPS_ALL_INTRA,
	MAX_ENH_LAYER_COUNT,
	NUM_VPP_PIPE,
	SW_PC,
	SW_PC_DELAY,
	FW_UNLOAD,
	FW_UNLOAD_DELAY,
	HW_RESPONSE_TIMEOUT,
	PREFIX_BUF_COUNT_PIX,
	PREFIX_BUF_SIZE_PIX,
	PREFIX_BUF_COUNT_NON_PIX,
	PREFIX_BUF_SIZE_NON_PIX,
	PAGEFAULT_NON_FATAL,
	PAGETABLE_CACHING,
	DCVS,
	DECODE_BATCH,
	DECODE_BATCH_TIMEOUT,
	STATS_TIMEOUT_MS,
	AV_SYNC_WINDOW_SIZE,
	CLK_FREQ_THRESHOLD,
	NON_FATAL_FAULTS,
	ENC_AUTO_FRAMERATE,
	MMRM,
	CORE_CAP_MAX,
};

enum msm_vidc_inst_capability_type {
	INST_CAP_NONE = 0,
	FRAME_WIDTH,
	LOSSLESS_FRAME_WIDTH,
	SECURE_FRAME_WIDTH,
	FRAME_HEIGHT,
	LOSSLESS_FRAME_HEIGHT,
	SECURE_FRAME_HEIGHT,
	PIX_FMTS,
	MIN_BUFFERS_INPUT,
	MIN_BUFFERS_OUTPUT,
	MBPF,
	LOSSLESS_MBPF,
	BATCH_MBPF,
	BATCH_FPS,
	SECURE_MBPF,
	MBPS,
	POWER_SAVE_MBPS,
	FRAME_RATE,
	OPERATING_RATE,
	SCALE_FACTOR,
	MB_CYCLES_VSP,
	MB_CYCLES_VPP,
	MB_CYCLES_LP,
	MB_CYCLES_FW,
	MB_CYCLES_FW_VPP,
	SECURE_MODE,
	TS_REORDER,
	HFLIP,
	VFLIP,
	ROTATION,
	SUPER_FRAME,
	SLICE_INTERFACE,
	HEADER_MODE,
	PREPEND_SPSPPS_TO_IDR,
	VUI_TIMING_INFO,
	META_SEQ_HDR_NAL,
	WITHOUT_STARTCODE,
	NAL_LENGTH_FIELD,
	REQUEST_I_FRAME,
	BIT_RATE,
	BITRATE_MODE,
	LOSSLESS,
	FRAME_SKIP_MODE,
	FRAME_RC_ENABLE,
	CONSTANT_QUALITY,
	GOP_SIZE,
	GOP_CLOSURE,
	B_FRAME,
	BLUR_TYPES,
	BLUR_RESOLUTION,
	CSC,
	CSC_CUSTOM_MATRIX,
	GRID,
	LOWLATENCY_MODE,
	LTR_COUNT,
	USE_LTR,
	MARK_LTR,
	BASELAYER_PRIORITY,
	IR_RANDOM,
	AU_DELIMITER,
	TIME_DELTA_BASED_RC,
	CONTENT_ADAPTIVE_CODING,
	BITRATE_BOOST,
	MIN_QUALITY,
	VBV_DELAY,
	PEAK_BITRATE,
	MIN_FRAME_QP,
	I_FRAME_MIN_QP,
	P_FRAME_MIN_QP,
	B_FRAME_MIN_QP,
	MAX_FRAME_QP,
	I_FRAME_MAX_QP,
	P_FRAME_MAX_QP,
	B_FRAME_MAX_QP,
	I_FRAME_QP,
	P_FRAME_QP,
	B_FRAME_QP,
	LAYER_TYPE,
	LAYER_ENABLE,
	ENH_LAYER_COUNT,
	L0_BR,
	L1_BR,
	L2_BR,
	L3_BR,
	L4_BR,
	L5_BR,
	ENTROPY_MODE,
	PROFILE,
	LEVEL,
	HEVC_TIER,
	LF_MODE,
	LF_ALPHA,
	LF_BETA,
	SLICE_MODE,
	SLICE_MAX_BYTES,
	SLICE_MAX_MB,
	MB_RC,
	TRANSFORM_8X8,
	CHROMA_QP_INDEX_OFFSET,
	DISPLAY_DELAY_ENABLE,
	DISPLAY_DELAY,
	CONCEAL_COLOR_8BIT,
	CONCEAL_COLOR_10BIT,
	STAGE,
	PIPE,
	POC,
	QUALITY_MODE,
	CODED_FRAMES,
	BIT_DEPTH,
	CODEC_CONFIG,
	BITSTREAM_SIZE_OVERWRITE,
	THUMBNAIL_MODE,
	DEFAULT_HEADER,
	RAP_FRAME,
	SEQ_CHANGE_AT_SYNC_FRAME,
	PRIORITY,
	ENC_IP_CR,
	DPB_LIST,
	ALL_INTRA,
	META_LTR_MARK_USE,
	META_DPB_MISR,
	META_OPB_MISR,
	META_INTERLACE,
	META_TIMESTAMP,
	META_CONCEALED_MB_CNT,
	META_HIST_INFO,
	META_SEI_MASTERING_DISP,
	META_SEI_CLL,
	META_HDR10PLUS,
	META_EVA_STATS,
	META_BUF_TAG,
	META_DPB_TAG_LIST,
	META_OUTPUT_BUF_TAG,
	META_SUBFRAME_OUTPUT,
	META_ENC_QP_METADATA,
	META_ROI_INFO,
	META_DEC_QP_METADATA,
	COMPLEXITY,
	META_MAX_NUM_REORDER_FRAMES,
	INST_CAP_MAX,
};

enum msm_vidc_inst_capability_flags {
	CAP_FLAG_NONE                    = 0,
	CAP_FLAG_ROOT                    = BIT(0),
	CAP_FLAG_DYNAMIC_ALLOWED         = BIT(1),
	CAP_FLAG_MENU                    = BIT(2),
	CAP_FLAG_INPUT_PORT              = BIT(3),
	CAP_FLAG_OUTPUT_PORT             = BIT(4),
	CAP_FLAG_CLIENT_SET              = BIT(5),
};

struct msm_vidc_inst_cap {
	enum msm_vidc_inst_capability_type cap;
	s32 min;
	s32 max;
	u32 step_or_mask;
	s32 value;
	u32 v4l2_id;
	u32 hfi_id;
	enum msm_vidc_inst_capability_flags flags;
	enum msm_vidc_inst_capability_type parents[MAX_CAP_PARENTS];
	enum msm_vidc_inst_capability_type children[MAX_CAP_CHILDREN];
	int (*adjust)(void *inst,
		struct v4l2_ctrl *ctrl);
	int (*set)(void *inst,
		enum msm_vidc_inst_capability_type cap_id);
};

struct msm_vidc_inst_capability {
	enum msm_vidc_domain_type domain;
	enum msm_vidc_codec_type codec;
	struct msm_vidc_inst_cap cap[INST_CAP_MAX+1];
};

struct msm_vidc_core_capability {
	enum msm_vidc_core_capability_type type;
	u32 value;
};

struct msm_vidc_inst_cap_entry {
	/* list of struct msm_vidc_inst_cap_entry */
	struct list_head list;
	enum msm_vidc_inst_capability_type cap_id;
};

struct debug_buf_count {
	u64 etb;
	u64 ftb;
	u64 fbd;
	u64 ebd;
};

struct msm_vidc_statistics {
	struct debug_buf_count             count;
	u64                                data_size;
	u64                                time_ms;
};

enum efuse_purpose {
	SKU_VERSION = 0,
};

enum sku_version {
	SKU_VERSION_0 = 0,
	SKU_VERSION_1,
	SKU_VERSION_2,
};

enum msm_vidc_ssr_trigger_type {
	SSR_ERR_FATAL = 1,
	SSR_SW_DIV_BY_ZERO,
	SSR_HW_WDOG_IRQ,
};

enum msm_vidc_stability_trigger_type {
	STABILITY_VCODEC_HUNG = 1,
	STABILITY_ENC_BUFFER_FULL,
};

enum msm_vidc_cache_op {
	MSM_VIDC_CACHE_CLEAN,
	MSM_VIDC_CACHE_INVALIDATE,
	MSM_VIDC_CACHE_CLEAN_INVALIDATE,
};

enum msm_vidc_dcvs_flags {
	MSM_VIDC_DCVS_INCR               = BIT(0),
	MSM_VIDC_DCVS_DECR               = BIT(1),
};

enum msm_vidc_clock_properties {
	CLOCK_PROP_HAS_SCALING           = BIT(0),
	CLOCK_PROP_HAS_MEM_RETENTION     = BIT(1),
};

enum profiling_points {
	FRAME_PROCESSING       = 0,
	MAX_PROFILING_POINTS,
};

enum signal_session_response {
	SIGNAL_CMD_STOP_INPUT = 0,
	SIGNAL_CMD_STOP_OUTPUT,
	SIGNAL_CMD_CLOSE,
	MAX_SIGNAL,
};

#define HFI_MASK_QHDR_TX_TYPE			0xFF000000
#define HFI_MASK_QHDR_RX_TYPE			0x00FF0000
#define HFI_MASK_QHDR_PRI_TYPE			0x0000FF00
#define HFI_MASK_QHDR_Q_ID_TYPE			0x000000FF
#define HFI_Q_ID_HOST_TO_CTRL_CMD_Q		0x00
#define HFI_Q_ID_CTRL_TO_HOST_MSG_Q		0x01
#define HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q	0x02
#define HFI_MASK_QHDR_STATUS			0x000000FF

#define VIDC_IFACEQ_NUMQ					3
#define VIDC_IFACEQ_CMDQ_IDX				0
#define VIDC_IFACEQ_MSGQ_IDX				1
#define VIDC_IFACEQ_DBGQ_IDX				2
#define VIDC_IFACEQ_MAX_BUF_COUNT			50
#define VIDC_IFACE_MAX_PARALLEL_CLNTS		16
#define VIDC_IFACEQ_DFLT_QHDR				0x01010000

struct hfi_queue_table_header {
	u32 qtbl_version;
	u32 qtbl_size;
	u32 qtbl_qhdr0_offset;
	u32 qtbl_qhdr_size;
	u32 qtbl_num_q;
	u32 qtbl_num_active_q;
	void *device_addr;
	char name[256];
};

struct hfi_queue_header {
	u32 qhdr_status;
	u32 qhdr_start_addr;
	u32 qhdr_type;
	u32 qhdr_q_size;
	u32 qhdr_pkt_size;
	u32 qhdr_pkt_drop_cnt;
	u32 qhdr_rx_wm;
	u32 qhdr_tx_wm;
	u32 qhdr_rx_req;
	u32 qhdr_tx_req;
	u32 qhdr_rx_irq_status;
	u32 qhdr_tx_irq_status;
	u32 qhdr_read_idx;
	u32 qhdr_write_idx;
};

#define VIDC_IFACEQ_TABLE_SIZE (sizeof(struct hfi_queue_table_header) \
	+ sizeof(struct hfi_queue_header) * VIDC_IFACEQ_NUMQ)

#define VIDC_IFACEQ_QUEUE_SIZE	(VIDC_IFACEQ_MAX_PKT_SIZE *  \
	VIDC_IFACEQ_MAX_BUF_COUNT * VIDC_IFACE_MAX_PARALLEL_CLNTS)

#define VIDC_IFACEQ_GET_QHDR_START_ADDR(ptr, i)     \
	(void *)((ptr + sizeof(struct hfi_queue_table_header)) + \
		(i * sizeof(struct hfi_queue_header)))

#define QDSS_SIZE 4096
#define SFR_SIZE 4096

#define QUEUE_SIZE (VIDC_IFACEQ_TABLE_SIZE + \
	(VIDC_IFACEQ_QUEUE_SIZE * VIDC_IFACEQ_NUMQ))

#define ALIGNED_QDSS_SIZE ALIGN(QDSS_SIZE, SZ_4K)
#define ALIGNED_SFR_SIZE ALIGN(SFR_SIZE, SZ_4K)
#define ALIGNED_QUEUE_SIZE ALIGN(QUEUE_SIZE, SZ_4K)
#define SHARED_QSIZE ALIGN(ALIGNED_SFR_SIZE + ALIGNED_QUEUE_SIZE + \
			ALIGNED_QDSS_SIZE, SZ_1M)
#define TOTAL_QSIZE (SHARED_QSIZE - ALIGNED_SFR_SIZE - ALIGNED_QDSS_SIZE)

struct profile_data {
	u64                    start;
	u64                    stop;
	u64                    cumulative;
	char                   name[64];
	u32                    sampling;
	u64                    average;
};

struct msm_vidc_debug {
	struct profile_data    pdata[MAX_PROFILING_POINTS];
	u32                    profile;
	u32                    samples;
};

struct msm_vidc_input_cr_data {
	struct list_head       list;
	u32                    index;
	u32                    input_cr;
};

struct msm_vidc_session_idle {
	bool                   idle;
	u64                    last_activity_time_ns;
};

struct msm_vidc_color_info {
	u32 colorspace;
	u32 ycbcr_enc;
	u32 xfer_func;
	u32 quantization;
};

struct msm_vidc_rectangle {
	u32 left;
	u32 top;
	u32 width;
	u32 height;
};

struct msm_vidc_subscription_params {
	u32                    bitstream_resolution;
	u32                    crop_offsets[2];
	u32                    bit_depth;
	u32                    coded_frames;
	u32                    fw_min_count;
	u32                    pic_order_cnt;
	u32                    color_info;
	u32                    profile;
	u32                    level;
	u32                    tier;
};

struct msm_vidc_hfi_frame_info {
	u32                    picture_type;
	u32                    no_output;
	u32                    subframe_input;
	u32                    cr;
	u32                    cf;
	u32                    data_corrupt;
	u32                    overflow;
};

struct msm_vidc_decode_vpp_delay {
	bool                   enable;
	u32                    size;
};

struct msm_vidc_decode_batch {
	bool                   enable;
	u32                    size;
	struct delayed_work    work;
};

enum msm_vidc_power_mode {
	VIDC_POWER_NORMAL = 0,
	VIDC_POWER_LOW,
	VIDC_POWER_TURBO,
};

struct vidc_bus_vote_data {
	enum msm_vidc_domain_type domain;
	enum msm_vidc_codec_type codec;
	enum msm_vidc_power_mode power_mode;
	u32 color_formats[2];
	int num_formats; /* 1 = DPB-OPB unified; 2 = split */
	int input_height, input_width, bitrate;
	int output_height, output_width;
	int rotation;
	int compression_ratio;
	int complexity_factor;
	int input_cr;
	u32 lcu_size;
	u32 fps;
	u32 work_mode;
	bool use_sys_cache;
	bool b_frames_enabled;
	u64 calc_bw_ddr;
	u64 calc_bw_llcc;
	u32 num_vpp_pipes;
};

struct msm_vidc_power {
	enum msm_vidc_power_mode power_mode;
	u32                    buffer_counter;
	u32                    min_threshold;
	u32                    nom_threshold;
	u32                    max_threshold;
	bool                   dcvs_mode;
	u32                    dcvs_window;
	u64                    min_freq;
	u64                    curr_freq;
	u32                    ddr_bw;
	u32                    sys_cache_bw;
	u32                    dcvs_flags;
	u32                    fw_cr;
	u32                    fw_cf;
};

struct msm_vidc_alloc {
	struct list_head            list;
	enum msm_vidc_buffer_type   type;
	enum msm_vidc_buffer_region region;
	u32                         size;
	u8                          secure:1;
	u8                          map_kernel:1;
	struct dma_buf             *dmabuf;
#if (KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE)
	struct dma_buf_map          dmabuf_map;
#endif
	void                       *kvaddr;
};

struct msm_vidc_allocations {
	struct list_head            list; // list of "struct msm_vidc_alloc"
};

struct msm_vidc_map {
	struct list_head            list;
	enum msm_vidc_buffer_type   type;
	enum msm_vidc_buffer_region region;
	struct dma_buf             *dmabuf;
	u32                         refcount;
	u64                         device_addr;
	struct sg_table            *table;
	struct dma_buf_attachment  *attach;
	u32                         skip_delayed_unmap:1;
};

struct msm_vidc_mappings {
	struct list_head            list; // list of "struct msm_vidc_map"
};

struct msm_vidc_buffer {
	struct list_head                   list;
	enum msm_vidc_buffer_type          type;
	u32                                index;
	int                                fd;
	u32                                buffer_size;
	u32                                data_offset;
	u32                                data_size;
	u64                                device_addr;
	void                              *dmabuf;
	u32                                flags;
	u64                                timestamp;
	enum msm_vidc_buffer_attributes    attr;
};

struct msm_vidc_buffers {
	struct list_head       list; // list of "struct msm_vidc_buffer"
	u32                    min_count;
	u32                    extra_count;
	u32                    actual_count;
	u32                    size;
	bool                   reuse;
};

struct msm_vidc_sort {
	struct list_head       list;
	s64                    val;
};

struct msm_vidc_timestamp {
	struct msm_vidc_sort   sort;
	u64                    rank;
};

struct msm_vidc_timestamps {
	struct list_head       list;
	u32                    count;
	u64                    rank;
};

enum msm_vidc_allow {
	MSM_VIDC_DISALLOW = 0,
	MSM_VIDC_ALLOW,
	MSM_VIDC_DEFER,
	MSM_VIDC_DISCARD,
	MSM_VIDC_IGNORE,
};

enum response_work_type {
	RESP_WORK_INPUT_PSC = 1,
	RESP_WORK_OUTPUT_PSC,
	RESP_WORK_LAST_FLAG,
};

struct response_work {
	struct list_head        list;
	enum response_work_type type;
	void                   *data;
	u32                     data_size;
};

struct msm_vidc_ssr {
	bool                               trigger;
	enum msm_vidc_ssr_trigger_type     ssr_type;
	u32                                sub_client_id;
	u32                                test_addr;
};

struct msm_vidc_stability {
	enum msm_vidc_stability_trigger_type     stability_type;
	u32                                      sub_client_id;
	u32                                      value;
};

struct msm_vidc_sfr {
	u32 bufSize;
	u8 rg_data[1];
};

#define call_mem_op(c, op, ...)			\
	(((c) && (c)->mem_ops && (c)->mem_ops->op) ? \
	((c)->mem_ops->op(__VA_ARGS__)) : 0)

struct msm_vidc_memory_ops {
	int (*allocate)(void *inst, struct msm_vidc_buffer *mbuf);
	int (*dma_map)(void *inst, struct msm_vidc_buffer *mbuf);
	int (*dma_unmap)(void *inst, struct msm_vidc_buffer *mbuf);
	int (*free)(void *inst, struct msm_vidc_buffer *mbuf);
	int (*cache_op)(void *inst, struct msm_vidc_buffer *mbuf,
				enum msm_vidc_cache_op cache_op);
};

#endif // _MSM_VIDC_INTERNAL_H_

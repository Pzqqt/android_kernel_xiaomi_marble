/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_INTERNAL_H_
#define _MSM_VIDC_INTERNAL_H_

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
#define MAX_MATRIX_COEFFS 9
#define MAX_BIAS_COEFFS   3
#define MAX_LIMIT_COEFFS  6
#define MAX_DEBUGFS_NAME  50
#define DEFAULT_TIMEOUT   3
#define DEFAULT_HEIGHT    240
#define DEFAULT_WIDTH     320
#define MAX_HEIGHT        4320
#define MAX_WIDTH         8192
#define MIN_SUPPORTED_WIDTH   32
#define MIN_SUPPORTED_HEIGHT  32
#define DEFAULT_FPS       30
#define MINIMUM_FPS       1
#define MAXIMUM_FPS       960
#define SINGLE_INPUT_BUFFER   1
#define SINGLE_OUTPUT_BUFFER  1
#define MAX_NUM_INPUT_BUFFERS    VIDEO_MAX_FRAME // same as VB2_MAX_FRAME
#define MAX_NUM_OUTPUT_BUFFERS   VIDEO_MAX_FRAME // same as VB2_MAX_FRAME
#define MAX_SUPPORTED_INSTANCES  16
#define MAX_BSE_VPP_DELAY        6
#define DEFAULT_BSE_VPP_DELAY    2
#define MAX_CAP_PARENTS          16
#define MAX_CAP_CHILDREN         16
/* TODO
 * #define MAX_SUPERFRAME_COUNT     32
*/

/* Maintains the number of FTB's between each FBD over a window */
#define DCVS_FTB_WINDOW 16
/* Superframe can have maximum of 32 frames */
#define VIDC_SUPERFRAME_MAX 32
#define COLOR_RANGE_UNSPECIFIED (-1)

#define V4L2_EVENT_VIDC_BASE  10
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

#define IS_PRIV_CTRL(idx) ( \
	(V4L2_CTRL_ID2WHICH(idx) == V4L2_CTRL_CLASS_MPEG) && \
	V4L2_CTRL_DRIVER_PRIV(idx))

#define BUFFER_ALIGNMENT_SIZE(x) x
#define NUM_MBS_720P (((1280 + 15) >> 4) * ((720 + 15) >> 4))
#define NUM_MBS_4k (((4096 + 15) >> 4) * ((2304 + 15) >> 4))

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

enum msm_vidc_domain_type {
	MSM_VIDC_ENCODER           = BIT(0),
	MSM_VIDC_DECODER           = BIT(1),
};

enum msm_vidc_codec_type {
	MSM_VIDC_H264              = BIT(0),
	MSM_VIDC_HEVC              = BIT(1),
	MSM_VIDC_VP9               = BIT(2),
	MSM_VIDC_MPEG2             = BIT(3),
};

enum msm_vidc_colorformat_type {
	MSM_VIDC_FMT_NONE = 0,
	MSM_VIDC_FMT_NV12,
	MSM_VIDC_FMT_NV21,
	MSM_VIDC_FMT_NV12_UBWC,
	MSM_VIDC_FMT_NV12_P010,
	MSM_VIDC_FMT_NV12_TP10_UBWC,
	MSM_VIDC_FMT_RGBA8888,
	MSM_VIDC_FMT_RGBA8888_UBWC,
};

enum msm_vidc_buffer_type {
	MSM_VIDC_BUF_NONE = 0,
	MSM_VIDC_BUF_INPUT,
	MSM_VIDC_BUF_OUTPUT,
	MSM_VIDC_BUF_INPUT_META,
	MSM_VIDC_BUF_OUTPUT_META,
	MSM_VIDC_BUF_QUEUE,
	MSM_VIDC_BUF_BIN,
	MSM_VIDC_BUF_ARP,
	MSM_VIDC_BUF_COMV,
	MSM_VIDC_BUF_NON_COMV,
	MSM_VIDC_BUF_LINE,
	MSM_VIDC_BUF_DPB,
	MSM_VIDC_BUF_PERSIST,
	MSM_VIDC_BUF_VPSS,
};

/* always match with v4l2 flags V4L2_BUF_FLAG_* */
enum msm_vidc_buffer_flags {
	MSM_VIDC_BUF_FLAG_KEYFRAME         = 0x00000008,
	MSM_VIDC_BUF_FLAG_PFRAME           = 0x00000010,
	MSM_VIDC_BUF_FLAG_BFRAME           = 0x00000020,
	MSM_VIDC_BUF_FLAG_ERROR            = 0x00000040,
	MSM_VIDC_BUF_FLAG_LAST             = 0x00100000,
	// TODO: remove below flags
	MSM_VIDC_BUF_FLAG_CODECCONFIG      = 0x01000000,
	MSM_VIDC_BUF_FLAG_SUBFRAME         = 0x02000000,
};

enum msm_vidc_buffer_attributes {
	MSM_VIDC_ATTR_DEFERRED                  = BIT(0),
	MSM_VIDC_ATTR_READ_ONLY                 = BIT(1),
	MSM_VIDC_ATTR_PENDING_RELEASE           = BIT(2),
	MSM_VIDC_ATTR_QUEUED                    = BIT(3),
};

enum msm_vidc_buffer_region {
	MSM_VIDC_REGION_NONE = 0,
	MSM_VIDC_NON_SECURE,
	MSM_VIDC_SECURE_PIXEL,
	MSM_VIDC_SECURE_NONPIXEL,
	MSM_VIDC_SECURE_BITSTREAM,
};

enum msm_vidc_port_type {
	INPUT_PORT = 0,
	OUTPUT_PORT,
	INPUT_META_PORT,
	OUTPUT_META_PORT,
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

enum msm_vidc_core_capability_type {
	CORE_CAP_NONE = 0,
	ENC_CODECS,
	DEC_CODECS,
	MAX_SESSION_COUNT,
	MAX_SECURE_SESSION_COUNT,
	MAX_LOAD,
	MAX_MBPF,
	MAX_MBPS,
	MAX_MBPF_HQ,
	MAX_MBPS_HQ,
	MAX_MBPF_B_FRAME,
	MAX_MBPS_B_FRAME,
	NUM_VPP_PIPE,
	SW_PC,
	SW_PC_DELAY,
	FW_UNLOAD,
	FW_UNLOAD_DELAY,
	HW_RESPONSE_TIMEOUT,
	DEBUG_TIMEOUT,
	PREFIX_BUF_COUNT_PIX,
	PREFIX_BUF_SIZE_PIX,
	PREFIX_BUF_COUNT_NON_PIX,
	PREFIX_BUF_SIZE_NON_PIX,
	PAGEFAULT_NON_FATAL,
	PAGETABLE_CACHING,
	DCVS,
	DECODE_BATCH,
	DECODE_BATCH_TIMEOUT,
	AV_SYNC_WINDOW_SIZE,
	CLK_FREQ_THRESHOLD,
	CORE_CAP_MAX,
};

enum msm_vidc_inst_capability_type {
	INST_CAP_NONE = 0,
	FRAME_WIDTH,
	FRAME_HEIGHT,
	PIX_FMTS,
	MIN_BUFFERS_INPUT,
	MIN_BUFFERS_OUTPUT,
	MBPF,
	MBPS,
	FRAME_RATE,
	SCALE_X,
	SCALE_Y,
	B_FRAME,
	POWER_SAVE_MBPS,
	BATCH_MBPF,
	BATCH_FRAME_RATE,
	LOSSLESS_FRAME_WIDTH,
	LOSSLESS_FRAME_HEIGHT,
	LOSSLESS_MBPF,
	ALL_INTRA_FRAME_RATE,
	HEVC_IMAGE_FRAME_WIDTH,
	HEVC_IMAGE_FRAME_HEIGHT,
	HEIC_IMAGE_FRAME_WIDTH,
	HEIC_IMAGE_FRAME_HEIGHT,
	MB_CYCLES_VSP,
	MB_CYCLES_VPP,
	MB_CYCLES_LP,
	MB_CYCLES_FW,
	MB_CYCLES_FW_VPP,
	HFLIP,
	VFLIP,
	PREPEND_SPSPPS_TO_IDR,
	REQUEST_I_FRAME,
	SLICE_INTERFACE,
	FRAME_RC,
	BITRATE_MODE,
	HEADER_MODE,
	GOP_SIZE,
	GOP_CLOSURE,
	BIT_RATE,
	SECURE_FRAME_WIDTH,
	SECURE_FRAME_HEIGHT,
	SECURE_MBPF,
	SECURE_MODE,
	BLUR_TYPES,
	BLUR_RESOLUTION,
	CSC_CUSTOM_MATRIX,
	HEIC,
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
	ROTATION,
	VBV_DELAY,
	MIN_FRAME_QP,
	MAX_FRAME_QP,
	HEVC_HIER_QP,
	I_FRAME_QP,
	P_FRAME_QP,
	I_FRAME_MIN_QP,
	I_FRAME_MAX_QP,
	P_FRAME_MIN_QP,
	P_FRAME_MAX_QP,
	B_FRAME_QP,
	B_FRAME_MIN_QP,
	B_FRAME_MAX_QP,
	HIER_CODING_TYPE,
	HIER_CODING_LAYER,
	L0_QP,
	L1_QP,
	L2_QP,
	L3_QP,
	L4_QP,
	L5_QP,
	PROFILE,
	LEVEL,
	HEVC_TIER,
	LF_MODE,
	LF_ALPHA,
	LF_BETA,
	LF_TC,
	LOSSLESS,
	L0_BR,
	L1_BR,
	L2_BR,
	L3_BR,
	L4_BR,
	L5_BR,
	SLICE_MAX_BYTES,
	SLICE_MAX_MB,
	SLICE_MODE,
	CABAC_BITRATE,
	MB_RC,
	TRANSFORM_8X8,
	ENTROPY_MODE,
	HIER_CODING,
	HIER_LAYER_QP,
	CHROMA_QP_INDEX_OFFSET,
	DISPLAY_DELAY_ENABLE,
	DISPLAY_DELAY,
	CONCEAL_COLOR_8BIT,
	CONCEAL_COLOR_10BIT,
	STAGE,
	PIPE,
	POC,
	INST_CAP_MAX,
};

enum msm_vidc_inst_capability_flags {
	CAP_FLAG_NONE                    = 0,
	CAP_FLAG_ROOT                    = BIT(0),
	CAP_FLAG_DYNAMIC_ALLOWED         = BIT(1),
	CAP_FLAG_MENU                    = BIT(2),
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
	struct msm_vidc_inst_cap cap[INST_CAP_MAX];
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

struct buf_count {
	u32                    etb;
	u32                    ftb;
	u32                    fbd;
	u32                    ebd;
};

struct profile_data {
	u32                    start;
	u32                    stop;
	u32                    cumulative;
	char                   name[64];
	u32                    sampling;
	u32                    average;
};

struct msm_vidc_debug {
	struct profile_data    pdata[MAX_PROFILING_POINTS];
	u32                    profile;
	u32                    samples;
	struct buf_count       count;
};

struct msm_vidc_input_cr_data {
	struct list_head       list;
	u32                    index;
	u32                    input_cr;
};

struct msm_vidc_timestamps {
	struct list_head       list;
	u64                    timestamp_us;
	u32                    framerate;
	bool                   is_valid;
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

struct msm_vidc_crop {
	u32 left;
	u32 top;
	u32 width;
	u32 height;
};

struct msm_vidc_properties {
	u32                    frame_rate;
	u32                    operating_rate;
};

struct msm_vidc_subscription_params {
	u32                    bitstream_resolution;
	u64                    crop_offsets;
	u32                    bit_depth;
	u32                    cabac;
	u32                    coded_frames;
	u32                    fw_min_count;
	u32                    pic_order_cnt;
	u32                    color_info;
	u32                    profile;
	u32                    level;
	u32                    tier;
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

struct msm_vidc_power {
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
};

struct msm_vidc_alloc {
	struct list_head            list;
	enum msm_vidc_buffer_type   type;
	enum msm_vidc_buffer_region region;
	u32                         size;
	u8                          cached:1;
	u8                          secure:1;
	u8                          map_kernel:1;
	struct dma_buf             *dmabuf;
	void                       *kvaddr;
};

struct msm_vidc_allocations {
	struct list_head            list; // list of "struct msm_vidc_alloc"
};

struct msm_vidc_map {
	struct list_head            list;
	bool                        valid;
	enum msm_vidc_buffer_type   type;
	enum msm_vidc_buffer_region region;
	struct dma_buf             *dmabuf;
	u32                         refcount;
	u64                         device_addr;
	struct sg_table            *table;
	struct dma_buf_attachment  *attach;
};

struct msm_vidc_mappings {
	struct list_head            list; // list of "struct msm_vidc_map"
};

struct msm_vidc_buffer {
	struct list_head                   list;
	bool                               valid;
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
};

struct msm_vidc_ssr {
	bool                               trigger;
	enum msm_vidc_ssr_trigger_type     ssr_type;
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

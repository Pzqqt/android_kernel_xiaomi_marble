/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_HFI_PROPERTY_H__
#define __H_HFI_PROPERTY_H__

#define HFI_PROP_BEGIN                                          0x03000000
#define HFI_PROP_IMAGE_VERSION                                  0x03000001
#define HFI_PROP_INTRA_FRAME_POWER_COLLAPSE                     0x03000002
#define HFI_PROP_UBWC_MAX_CHANNELS                              0x03000003
#define HFI_PROP_UBWC_MAL_LENGTH                                0x03000004
#define HFI_PROP_UBWC_HBB                                       0x03000005
#define HFI_PROP_UBWC_BANK_SWZL_LEVEL1                          0x03000006
#define HFI_PROP_UBWC_BANK_SWZL_LEVEL2                          0x03000007
#define HFI_PROP_UBWC_BANK_SWZL_LEVEL3                          0x03000008
#define HFI_PROP_UBWC_BANK_SPREADING                            0x03000009

enum hfi_debug_config {
    HFI_DEBUG_CONFIG_DEFAULT = 0x00000000,
    HFI_DEBUG_CONFIG_CLRDBGQ = 0x00000001,
    HFI_DEBUG_CONFIG_WFI     = 0x00000002,
    HFI_DEBUG_CONFIG_ARM9WD  = 0x00000004,
};
#define HFI_PROP_DEBUG_CONFIG                                   0x0300000a

enum hfi_debug_log_level {
    HFI_DEBUG_LOG_NONE   = 0x00000000,
    HFI_DEBUG_LOG_ERROR  = 0x00000001,
    HFI_DEBUG_LOG_FATAL  = 0x00000002,
    HFI_DEBUG_LOG_PERF   = 0x00000004,
    HFI_DEBUG_LOG_HIGH   = 0x00000008,
    HFI_DEBUG_LOG_MEDIUM = 0x00000010,
    HFI_DEBUG_LOG_LOW    = 0x00000020,
};

struct hfi_debug_header {
	u32   size;
	u32   debug_level;
	u32   reserved[2];
};
#define HFI_PROP_DEBUG_LOG_LEVEL                                0x0300000b

enum hfi_codec_type {
    HFI_CODEC_DECODE_AVC   = 1,
    HFI_CODEC_ENCODE_AVC   = 2,
    HFI_CODEC_DECODE_HEVC  = 3,
    HFI_CODEC_ENCODE_HEVC  = 4,
    HFI_CODEC_DECODE_VP9   = 5,
    HFI_CODEC_DECODE_MPEG2 = 6,
};
#define HFI_PROP_CODEC                                          0x03000100

enum hfi_color_format {
    HFI_COLOR_FMT_OPAQUE        = 0,
    HFI_COLOR_FMT_NV12          = 1,
    HFI_COLOR_FMT_NV12_UBWC     = 2,
    HFI_COLOR_FMT_P010          = 3,
    HFI_COLOR_FMT_TP10_UBWC     = 4,
    HFI_COLOR_FMT_RGBA8888      = 5,
    HFI_COLOR_FMT_RGBA8888_UBWC = 6,
    HFI_COLOR_FMT_NV21          = 7,
};
#define HFI_PROP_COLOR_FORMAT                                   0x03000101

#define HFI_PROP_SECURE                                         0x03000102

#define HFI_BITMASK_BITSTREAM_WIDTH                             0xffff0000
#define HFI_BITMASK_BITSTREAM_HEIGHT                            0x0000ffff
#define HFI_PROP_BITSTREAM_RESOLUTION                           0x03000103

#define HFI_BITMASK_LINEAR_STRIDE            0xffff0000
#define HFI_BITMASK_LINEAR_SCANLINE          0x0000ffff
#define HFI_PROP_LINEAR_STRIDE_SCANLINE                         0x03000104

#define HFI_BITMASK_CROP_RIGHT_OFFSET        0xffff0000
#define HFI_BITMASK_CROP_BOTTOM_OFFSET       0x0000ffff
#define HFI_BITMASK_CROP_LEFT_OFFSET         0xffff0000
#define HFI_BITMASK_CROP_TOP_OFFSET          0x0000ffff
#define HFI_PROP_CROP_OFFSETS                                   0x03000105

#define HFI_PROP_REALTIME                                       0x03000106

enum hfi_avc_profile_type {
    HFI_AVC_PROFILE_BASELINE             = 0,
    HFI_AVC_PROFILE_CONSTRAINED_BASELINE = 1,
    HFI_AVC_PROFILE_MAIN                 = 2,
    HFI_AVC_PROFILE_HIGH                 = 4,
    HFI_AVC_PROFILE_CONSTRAINED_HIGH     = 17
};

enum hfi_hevc_profile_type {
    HFI_H265_PROFILE_MAIN               = 0,
    HFI_H265_PROFILE_MAIN_STILL_PICTURE = 1,
    HFI_H265_PROFILE_MAIN_10            = 2,
};

enum hfi_vp9_profile_type {
    HFI_VP9_PROFILE_0 = 0,
    HFI_VP9_PROFILE_1 = 1,
    HFI_VP9_PROFILE_2 = 2,
    HFI_VP9_PROFILE_3 = 3,
};

enum hfi_mpeg2_profile_type {
    HFI_MP2_PROFILE_SIMPLE = 0,
    HFI_MP2_PROFILE_MAIN   = 1,
};

#define HFI_PROP_PROFILE                                        0x03000107

enum hfi_avc_level_type {
    HFI_AVC_LEVEL_1_0 = 0,
    HFI_AVC_LEVEL_1B  = 1,
    HFI_AVC_LEVEL_1_1 = 2,
    HFI_AVC_LEVEL_1_2 = 3,
    HFI_AVC_LEVEL_1_3 = 4,
    HFI_AVC_LEVEL_2_0 = 5,
    HFI_AVC_LEVEL_2_1 = 6,
    HFI_AVC_LEVEL_2_2 = 7,
    HFI_AVC_LEVEL_3_0 = 8,
    HFI_AVC_LEVEL_3_1 = 9,
    HFI_AVC_LEVEL_3_2 = 10,
    HFI_AVC_LEVEL_4_0 = 11,
    HFI_AVC_LEVEL_4_1 = 12,
    HFI_AVC_LEVEL_4_2 = 13,
    HFI_AVC_LEVEL_5_0 = 14,
    HFI_AVC_LEVEL_5_1 = 15,
    HFI_AVC_LEVEL_5_2 = 16,
    HFI_AVC_LEVEL_6_0 = 17,
    HFI_AVC_LEVEL_6_1 = 18,
    HFI_AVC_LEVEL_6_2 = 19,
};

enum hfi_hevc_level_type {
    HFI_H265_LEVEL_1   = 0,
    HFI_H265_LEVEL_2   = 1,
    HFI_H265_LEVEL_2_1 = 2,
    HFI_H265_LEVEL_3   = 3,
    HFI_H265_LEVEL_3_1 = 4,
    HFI_H265_LEVEL_4   = 5,
    HFI_H265_LEVEL_4_1 = 6,
    HFI_H265_LEVEL_5   = 7,
    HFI_H265_LEVEL_5_1 = 8,
    HFI_H265_LEVEL_5_2 = 9,
    HFI_H265_LEVEL_6   = 10,
    HFI_H265_LEVEL_6_1 = 11,
    HFI_H265_LEVEL_6_2 = 12,
};

enum hfi_vp9_level_type {
    HFI_VP9_LEVEL_1_0 = 0,
    HFI_VP9_LEVEL_1_1 = 1,
    HFI_VP9_LEVEL_2_0 = 2,
    HFI_VP9_LEVEL_2_1 = 3,
    HFI_VP9_LEVEL_3_0 = 4,
    HFI_VP9_LEVEL_3_1 = 5,
    HFI_VP9_LEVEL_4_0 = 6,
    HFI_VP9_LEVEL_4_1 = 7,
    HFI_VP9_LEVEL_5_0 = 8,
    HFI_VP9_LEVEL_5_1 = 9,
    HFI_VP9_LEVEL_6_0 = 10,
    HFI_VP9_LEVEL_6_1 = 11,
};

enum hfi_mpeg2_level_type {
    HFI_MP2_LEVEL_LOW       = 0,
    HFI_MP2_LEVEL_MAIN      = 1,
    HFI_MP2_LEVEL_HIGH_1440 = 2,
    HFI_MP2_LEVEL_HIGH      = 3,
};

enum hfi_codec_level_type {
    HFI_LEVEL_NONE = 0xFFFFFFFF,
};

#define HFI_PROP_LEVEL                                          0x03000108

enum hfi_hevc_tier_type {
    HFI_H265_TIER_MAIN = 0,
    HFI_H265_TIER_HIGH = 1,
};
#define HFI_PROP_TIER                                           0x03000109

#define HFI_PROP_STAGE                                          0x0300010a

#define HFI_PROP_PIPE                                           0x0300010b

#define HFI_PROP_FRAME_RATE                                     0x0300010c

#define HFI_BITMASK_CONCEAL_LUMA       0x000003ff
#define HFI_BITMASK_CONCEAL_CB         0x000ffC00
#define HFI_BITMASK_CONCEAL_CR         0x3ff00000
#define HFI_PROP_CONCEAL_COLOR_8BIT                             0x0300010d

#define HFI_BITMASK_CONCEAL_LUMA       0x000003ff
#define HFI_BITMASK_CONCEAL_CB         0x000ffC00
#define HFI_BITMASK_CONCEAL_CR         0x3ff00000
#define HFI_PROP_CONCEAL_COLOR_10BIT                            0x0300010e

#define HFI_BITMASK_LUMA_BIT_DEPTH     0xffff0000
#define HFI_BITMASK_CHROMA_BIT_DEPTH   0x0000ffff
#define HFI_PROP_LUMA_CHROMA_BIT_DEPTH                          0x0300010f

#define HFI_BITMASK_FRAME_MBS_ONLY_FLAG           0x00000001
#define HFI_BITMASK_MB_ADAPTIVE_FRAME_FIELD_FLAG  0x00000002
#define HFI_PROP_CODED_FRAMES                                   0x03000120

#define HFI_PROP_CABAC_SESSION                                  0x03000121

#define HFI_PROP_8X8_TRANSFORM                                  0x03000122

#define HFI_PROP_BUFFER_HOST_MAX_COUNT                          0x03000123

#define HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT                     0x03000124

#define HFI_PROP_BUFFER_MAXDPB_COUNT                            0x03000125

#define HFI_PROP_BUFFER_MAX_NUM_REFERENCE                       0x03000126

#define HFI_PROP_MAX_NUM_REORDER_FRAMES                         0x03000127

#define HFI_PROP_PIC_ORDER_CNT_TYPE                             0x03000128

enum hfi_deblock_mode {
    HFI_DEBLOCK_ALL_BOUNDARY              = 0x0,
    HFI_DEBLOCK_DISABLE                   = 0x1,
    HFI_DEBLOCK_DISABLE_AT_SLICE_BOUNDARY = 0x2,
};
#define HFI_PROP_DEBLOCKING_MODE                                0x03000129

enum hfi_rate_control {
    HFI_RC_VBR_CFR        = 0x00000000,
    HFI_RC_CBR_CFR        = 0x00000001,
    HFI_RC_CQ             = 0x00000002,
    HFI_RC_OFF            = 0x00000003,
    HFI_RC_CBR_VFR        = 0x00000004,
    HFI_RC_LOSSLESS       = 0x00000005,
    HFI_RC_CBR_VFR_LEGACY = 0x00000006,
};
#define HFI_PROP_RATE_CONTROL                                   0x0300012a

#define HFI_PROP_TIME_DELTA_BASED_RATE_CONTROL                  0x0300012b

#define HFI_PROP_CONTENT_ADAPTIVE_CODING                        0x0300012c

#define HFI_PROP_CONST_QUALITY_BITRATE_BOOST                    0x0300012d

#define HFI_BITMASK_QP_I          0x000000ff
#define HFI_BITMASK_QP_P          0x0000ff00
#define HFI_BITMASK_QP_B          0x00ff0000
#define HFI_BITMASK_QP_ENABLE     0x0f000000
#define HFI_BITMASK_QP_LAYERS     0xf0000000
#define HFI_PROP_QP_PACKED                                      0x0300012e

#define HFI_PROP_MIN_QP_PACKED                                  0x0300012f

#define HFI_PROP_MAX_QP_PACKED                                  0x03000130

#define HFI_PROP_IR_RANDOM_PERIOD                               0x03000131

#define HFI_PROP_MULTI_SLICE_MB_COUNT                           0x03000132

#define HFI_PROP_MULTI_SLICE_BYTES_COUNT                        0x03000133

#define HFI_PROP_LTR_COUNT                                      0x03000134

#define HFI_PROP_LTR_MARK                                       0x03000135

#define HFI_PROP_LTR_USE                                        0x03000136

#define HFI_PROP_LTR_MARK_USE_DETAILS                           0x03000137

enum hfi_layer_encoding_type {
    HFI_HIER_P_SLIDING_WINDOW = 0x1,
    HFI_HIER_P_HYBRID_LTR     = 0x2,
    HFI_HIER_B                = 0x3,
};
#define HFI_PROP_LAYER_ENCODING_TYPE                            0x03000138

#define HFI_PROP_LAYER_COUNT                                    0x03000139

#define HFI_BITMASK_CHROMA_CB_OFFSET     0x0000ffff
#define HFI_BITMASK_CHROMA_CR_OFFSET     0xffff0000
#define HFI_PROP_CHROMA_QP_OFFSET                               0x0300013a

#define HFI_PROP_TOTAL_BITRATE                                  0x0300013b

#define HFI_PROP_BITRATE_LAYER1                                 0x0300013c

#define HFI_PROP_BITRATE_LAYER2                                 0x0300013d

#define HFI_PROP_BITRATE_LAYER3                                 0x0300013e

#define HFI_PROP_BITRATE_LAYER4                                 0x0300013f

#define HFI_PROP_BITRATE_LAYER5                                 0x03000140

#define HFI_PROP_BITRATE_LAYER6                                 0x03000141

#define HFI_PROP_BASELAYER_PRIORITYID                           0x03000142

#define HFI_PROP_CONSTANT_QUALITY                               0x03000143

#define HFI_PROP_HEIC_GRID_ENABLE                               0x03000144

#define HFI_PROP_REQUEST_SYNC_FRAME                             0x03000145

#define HFI_PROP_MAX_GOP_FRAMES                                 0x03000146

#define HFI_PROP_MAX_B_FRAMES                                   0x03000147

enum hfi_quality_mode {
    HFI_MODE_MAX_QUALITY = 0x1,
    HFI_MODE_POWER_SAVE  = 0x2,
};
#define HFI_PROP_QUALITY_MODE                                   0x03000148

enum hfi_seq_header_mode {
    HFI_SEQ_HEADER_SEPERATE_FRAME         = 0x00000001,
    HFI_SEQ_HEADER_JOINED_WITH_1ST_FRAME  = 0x00000002,
    HFI_SEQ_HEADER_PREFIX_WITH_SYNC_FRAME = 0x00000004,
    HFI_SEQ_HEADER_METADATA               = 0x00000008,
};
#define HFI_PROP_SEQ_HEADER_MODE                                0x03000149

#define HFI_PROP_METADATA_SEQ_HEADER_NAL                        0x0300014a

enum hfi_rotation {
    HFI_ROTATION_NONE = 0x00000000,
    HFI_ROTATION_90   = 0x00000001,
    HFI_ROTATION_180  = 0x00000002,
    HFI_ROTATION_270  = 0x00000003,
};
#define HFI_PROP_ROTATION                                       0x0300014b

enum hfi_flip {
    HFI_DISABLE_FLIP    = 0x00000000,
    HFI_HORIZONTAL_FLIP = 0x00000001,
    HFI_VERTICAL_FLIP   = 0x00000002,
};
#define HFI_PROP_FLIP                                           0x0300014c

#define HFI_PROP_SCALAR                                         0x0300014d

enum hfi_blur_types {
    HFI_BLUR_NONE     = 0x00000000,
    HFI_BLUR_EXTERNAL = 0x00000001,
    HFI_BLUR_ADAPTIVE = 0x00000002,
};
#define HFI_PROP_BLUR_TYPES                                     0x0300014e

#define HFI_BITMASK_BLUR_WIDTH        0xffff0000
#define HFI_BITMASK_BLUR_HEIGHT       0x0000ffff
#define HFI_PROP_BLUR_RESOLUTION                                0x0300014f

#define HFI_BITMASK_SPS_ID            0x000000ff
#define HFI_BITMASK_PPS_ID            0x0000ff00
#define HFI_BITMASK_VPS_ID            0x00ff0000
#define HFI_PROP_SEQUENCE_HEADER_IDS                            0x03000150

#define HFI_PROP_AUD                                            0x03000151

#define HFI_PROP_DPB_LUMA_CHROMA_MISR                           0x03000153

#define HFI_PROP_OPB_LUMA_CHROMA_MISR                           0x03000154

#define HFI_BITMASK_QP_I               0x000000ff
#define HFI_BITMASK_QP_P               0x0000ff00
#define HFI_BITMASK_QP_B               0x00ff0000
#define HFI_BITMASK_QP_ENABLE          0x0f000000
#define HFI_BITMASK_QP_LAYERS          0xf0000000
#define HFI_PROP_SIGNAL_COLOR_INFO                              0x03000155

enum hfi_interlace_info {
    HFI_INTERLACE_INFO_NONE                = 0x00000000,
    HFI_FRAME_PROGRESSIVE                  = 0x00000001,
    HFI_FRAME_MBAFF                        = 0x00000002,
    HFI_FRAME_INTERLEAVE_TOPFIELD_FIRST    = 0x00000004,
    HFI_FRAME_INTERLEAVE_BOTTOMFIELD_FIRST = 0x00000008,
    HFI_FRAME_INTERLACE_TOPFIELD_FIRST     = 0x00000010,
    HFI_FRAME_INTERLACE_BOTTOMFIELD_FIRST  = 0x00000020,
};
#define HFI_PROP_INTERLACE_INFO                                 0x03000156

#define HFI_PROP_CSC                                            0x03000157

#define HFI_PROP_CSC_MATRIX                                     0x03000158

#define HFI_PROP_CSC_BIAS                                       0x03000159

#define HFI_PROP_CSC_LIMIT                                      0x0300015a

#define HFI_PROP_DECODE_ORDER_OUTPUT                            0x0300015b

#define HFI_PROP_TIMESTAMP                                      0x0300015c

#define HFI_PROP_FRAMERATE_FROM_BITSTREAM                       0x0300015d

#define HFI_PROP_SEI_RECOVERY_POINT                             0x0300015e

#define HFI_PROP_CONEALED_MB_COUNT                              0x0300015f

#define HFI_BITMASK_SAR_WIDTH        0xffff0000
#define HFI_BITMASK_SAR_HEIGHT       0x0000ffff
#define HFI_PROP_SAR_RESOLUTION                                 0x03000160

#define HFI_PROP_HISTOGRAM_INFO                                 0x03000161

enum hfi_picture_type {
    HFI_PICTURE_IDR = 0x00000001,
    HFI_PICTURE_P   = 0x00000002,
    HFI_PICTURE_B   = 0x00000004,
    HFI_PICTURE_I   = 0x00000008,
    HFI_PICTURE_CRA = 0x00000010,
    HFI_PICTURE_BLA = 0x00000020,
};
#define HFI_PROP_PICTURE_TYPE                                   0x03000162

#define HFI_PROP_SEI_MASTERING_DISPLAY_COLOUR                   0x03000163

#define HFI_PROP_SEI_CONTENT_LIGHT_LEVEL                        0x03000164

#define HFI_PROP_SEI_HDR10PLUS_USERDATA                         0x03000165

#define HFI_PROP_SEI_STREAM_USERDATA                            0x03000166

#define HFI_PROP_EVA_STAT_INFO                                  0x03000167

#define HFI_PROP_DEC_DEFAULT_HEADER                             0x03000168

#define HFI_PROP_DEC_START_FROM_RAP_FRAME                       0x03000169

#define HFI_PROP_NO_OUTPUT                                      0x0300016a

#define HFI_PROP_BUFFER_TAG                                     0x0300016b

#define HFI_PROP_BUFFER_MARK                                    0x0300016c

#define HFI_PROP_SUBFRAME_OUTPUT                                0x0300016d

#define HFI_PROP_ENC_QP_METADATA                                0x0300016e

#define HFI_PROP_DEC_QP_METADATA                                0x0300016f

#define HFI_PROP_SEI_FRAME_PACKING_ARRANGEMENT                  0x03000170

#define HFI_PROP_SEI_PAN_SCAN_RECT                              0x03000171

#define HFI_PROP_THUMBNAIL_MODE                                 0x03000172

#define HFI_PROP_ROI_INFO                                       0x03000173

#define HFI_PROP_END                                            0x03FFFFFF

#define HFI_SESSION_ERROR_BEGIN                                 0x04000000

#define HFI_ERROR_UNKNOWN_SESSION                               0x04000001

#define HFI_ERROR_MAX_SESSIONS                                  0x04000002

#define HFI_ERROR_FATAL                                         0x04000003

#define HFI_ERROR_INVALID_STATE                                 0x04000004

#define HFI_ERROR_INSUFFICIENT_RESOURCES                        0x04000005

#define HFI_ERROR_BUFFER_NOT_SET                                0x04000006

#define HFI_SESSION_ERROR_END                                   0x04FFFFFF

#define HFI_SYSTEM_ERROR_BEGIN                                  0x05000000

#define HFI_SYS_ERROR_WD_TIMEOUT                                0x05000001

#define HFI_SYS_ERROR_NOC                                       0x05000002

#define HFI_SYS_ERROR_FATAL                                     0x05000003

#define HFI_SYSTEM_ERROR_END                                    0x05FFFFFF

#define HFI_INFORMATION_BEGIN                                   0x06000000

#define HFI_INFO_UNSUPPORTED                                    0x06000001

#define HFI_INFO_DATA_CORRUPT                                   0x06000002

#define HFI_INFORMATION_END                                     0x06FFFFFF

#endif //__H_HFI_PROPERTY_H__

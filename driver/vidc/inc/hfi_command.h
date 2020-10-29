/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_HFI_COMMAND_H__
#define __H_HFI_COMMAND_H__

#include <linux/types.h>
#include <linux/bits.h>

#define HFI_VIDEO_ARCH_LX               0x1

struct hfi_header {
	u32 size;
	u32 session_id;
	u32 header_id;
	u32 reserved[4];
	u32 num_packets;
};

struct hfi_packet {
	u32 size;
	u32 type;
	u32 flags;
	u32 payload_info;
	u32 port;
	u32 packet_id;
	u32 reserved[2];
};

struct hfi_buffer {
	u32 type;
	u32 index;
	u64 base_address;
	u32 addr_offset;
	u32 buffer_size;
	u32 data_offset;
	u32 data_size;
	u64 timestamp;
	u32 flags;
	u32 reserved[5];
};

enum hfi_packet_host_flags {
	HFI_HOST_FLAGS_NONE                = 0x00000000,
	HFI_HOST_FLAGS_INTR_REQUIRED       = 0x00000001,
	HFI_HOST_FLAGS_RESPONSE_REQUIRED   = 0x00000002,
	HFI_HOST_FLAGS_NON_DISCARDABLE     = 0x00000004,
	HFI_HOST_FLAGS_GET_PROPERTY        = 0x00000008,
	HFI_HOST_FLAGS_CB_NON_SECURE       = 0x00000010,
	HFI_HOST_FLAGS_CB_SECURE_PIXEL     = 0x00000020,
	HFI_HOST_FLAGS_CB_SECURE_BITSTREAM = 0x00000040,
	HFI_HOST_FLAGS_CB_SECURE_NON_PIXEL = 0x00000080,
	HFI_HOST_FLAGS_CB_NON_SECURE_PIXEL = 0x00000100,
};

enum hfi_packet_firmware_flags {
	HFI_FW_FLAGS_NONE          = 0x00000000,
	HFI_FW_FLAGS_SUCCESS       = 0x00000001,
	HFI_FW_FLAGS_INFORMATION   = 0x00000002,
	HFI_FW_FLAGS_SESSION_ERROR = 0x00000004,
	HFI_FW_FLAGS_SYSTEM_ERROR  = 0x00000008,
};

enum hfi_packet_payload_info {
	HFI_PAYLOAD_NONE      = 0x00000000,
	HFI_PAYLOAD_U32       = 0x00000001,
	HFI_PAYLOAD_S32       = 0x00000002,
	HFI_PAYLOAD_U64       = 0x00000003,
	HFI_PAYLOAD_S64       = 0x00000004,
	HFI_PAYLOAD_STRUCTURE = 0x00000005,
	HFI_PAYLOAD_BLOB      = 0x00000006,
	HFI_PAYLOAD_STRING    = 0x00000007,
	HFI_PAYLOAD_Q16       = 0x00000008,
	HFI_PAYLOAD_U32_ENUM  = 0x00000009,
	HFI_PAYLOAD_32_PACKED = 0x0000000a,
	HFI_PAYLOAD_U32_ARRAY = 0x0000000b,
	HFI_PAYLOAD_S32_ARRAY = 0x0000000c,
	HFI_PAYLOAD_64_PACKED = 0x0000000d,
};

enum hfi_packet_port_type {
	HFI_PORT_NONE      = 0x00000000,
	HFI_PORT_BITSTREAM = 0x00000001,
	HFI_PORT_RAW       = 0x00000002,
};

enum hfi_buffer_type {
	HFI_BUFFER_BITSTREAM  = 0x00000001,
	HFI_BUFFER_RAW        = 0x00000002,
	HFI_BUFFER_METADATA   = 0x00000003,
	HFI_BUFFER_SUBCACHE   = 0x00000004,
	HFI_BUFFER_SFR        = 0x00000005,
	HFI_BUFFER_DPB        = 0x00000006,
	HFI_BUFFER_BIN        = 0x00000007,
	HFI_BUFFER_LINE       = 0x00000008,
	HFI_BUFFER_ARP        = 0x00000009,
	HFI_BUFFER_COMV       = 0x0000000A,
	HFI_BUFFER_NON_COMV   = 0x0000000B,
	HFI_BUFFER_PERSIST    = 0x0000000C,
	HFI_BUFFER_VPSS       = 0x0000000D,
};

enum hfi_buffer_host_flags {
	HFI_BUF_HOST_FLAG_NONE         = 0x00000000,
	HFI_BUF_HOST_FLAG_RELEASE      = 0x00000001,
	HFI_BUF_HOST_FLAG_READONLY     = 0x00000002,
	HFI_BUF_HOST_FLAG_CODEC_CONFIG = 0x00000004,
};

enum hfi_buffer_firmware_flags {
	HFI_BUF_FW_FLAG_NONE         = 0x00000000,
	HFI_BUF_FW_FLAG_LAST         = 0x00000001,
	HFI_BUF_FW_FLAG_READONLY     = 0x00000002,
	HFI_BUF_FW_FLAG_CODEC_CONFIG = 0x00000004,
};

enum hfi_metapayload_header_flags {
	HFI_METADATA_FLAGS_NONE         = 0x00000000,
	HFI_METADATA_FLAGS_TOP_FIELD    = 0x00000001,
	HFI_METADATA_FLAGS_BOTTOM_FIELD = 0x00000002,
};

struct metabuf_header {
	u32 count;
	u32 size;
	u32 version;
	u32 reserved[5];
};

struct metapayload_header {
	u32 type;
	u32 size;
	u32 version;
	u32 offset;
	u32 flags;
	u32 reserved[3];
};

enum hfi_property_mode_type {
	HFI_MODE_NONE                 = 0x00000000,
	HFI_MODE_PORT_SETTINGS_CHANGE = 0x00000001,
	HFI_MODE_PROPERTY             = 0x00000002,
	HFI_MODE_METADATA             = 0x00000004,
};

#define HFI_CMD_BEGIN                                           0x01000000
#define HFI_CMD_INIT                                            0x01000001
#define HFI_CMD_POWER_COLLAPSE                                  0x01000002
#define HFI_CMD_OPEN                                            0x01000003
#define HFI_CMD_CLOSE                                           0x01000004
#define HFI_CMD_START                                           0x01000005
#define HFI_CMD_STOP                                            0x01000006
#define HFI_CMD_DRAIN                                           0x01000007
#define HFI_CMD_RESUME                                          0x01000008
#define HFI_CMD_BUFFER                                          0x01000009
#define HFI_CMD_DELIVERY_MODE                                   0x0100000A
#define HFI_CMD_SUBSCRIBE_MODE                                  0x0100000B
#define HFI_CMD_SETTINGS_CHANGE                                 0x0100000C

#define HFI_SSR_TYPE_SW_ERR_FATAL       0x1
#define HFI_SSR_TYPE_SW_DIV_BY_ZERO     0x2
#define HFI_SSR_TYPE_HW_WDOG_IRQ        0x3
#define HFI_SSR_TYPE_NOC_ERROR          0x4
#define HFI_SSR_TYPE_VCODEC_HUNG        0x5

#define HFI_BITMASK_SSR_SUB_CLIENT_ID          0x000000f0
#define HFI_BITMASK_SSR_TYPE                   0x0000000f
#define HFI_CMD_SSR                                             0x0100000D

#define HFI_CMD_END                                             0x01FFFFFF

#endif //__H_HFI_COMMAND_H__

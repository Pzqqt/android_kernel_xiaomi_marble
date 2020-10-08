/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_HFI_COMMAND_H__
#define __H_HFI_COMMAND_H__

#include <linux/types.h>
#include <linux/bits.h>

#define HFI_VIDEO_ARCH_OX               0x1

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
	HFI_HOST_FLAGS_NONE              = 0,
	HFI_HOST_FLAGS_INTR_REQUIRED     = BIT(0),
	HFI_HOST_FLAGS_RESPONSE_REQUIRED = BIT(1),
	HFI_HOST_FLAGS_NON_DISCARDABLE   = BIT(2),
	HFI_HOST_FLAGS_GET_PROPERTY      = BIT(3),
};

enum hfi_packet_firmware_flags {
	HFI_FW_FLAGS_NONE          = 0,
	HFI_FW_FLAGS_SUCCESS       = BIT(0),
	HFI_FW_FLAGS_INFORMATION   = BIT(1),
	HFI_FW_FLAGS_SESSION_ERROR = BIT(2),
	HFI_FW_FLAGS_SYSTEM_ERROR  = BIT(3),
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
};

enum hfi_packet_port_type {
	HFI_PORT_NONE      = 0,
	HFI_PORT_BITSTREAM = BIT(0),
	HFI_PORT_RAW       = BIT(1),
};

enum hfi_buffer_type {
	HFI_BUFFER_BITSTREAM  = 0x00000001,
	HFI_BUFFER_RAW        = 0x00000002,
	HFI_BUFFER_METADATA   = 0x00000003,
	HFI_BUFFER_DPB        = 0x00000004,
	HFI_BUFFER_BIN        = 0x00000005,
	HFI_BUFFER_LINE       = 0x00000006,
	HFI_BUFFER_ARP        = 0x00000007,
	HFI_BUFFER_COMV       = 0x00000008,
	HFI_BUFFER_NON_COMV   = 0x00000009,
	HFI_BUFFER_PERSIST    = 0x0000000A,
	HFI_BUFFER_SUBCACHE   = 0x0000000B,
	HFI_BUFFER_SFR        = 0x0000000C,
};

enum hfi_buffer_host_flags {
	HFI_BUF_HOST_FLAG_NONE           = 0,
	HFI_BUF_HOST_FLAG_RELEASE        = BIT(0),
	HFI_BUF_HOST_FLAG_READONLY       = BIT(1),
	HFI_BUF_HOST_FLAG_CODEC_CONFIG   = BIT(2),
};

enum hfi_buffer_firmware_flags {
	HFI_BUF_FW_FLAG_NONE           = 0,
	HFI_BUF_FW_FLAG_LAST           = BIT(0),
	HFI_BUF_FW_FLAG_READONLY       = BIT(1),
	HFI_BUF_FW_FLAG_CODEC_CONFIG   = BIT(2),
	// TODO
	HFI_BUF_FW_FLAG_SUBFRAME       = BIT(3),
	HFI_BUF_FW_FLAG_KEYFRAME       = BIT(4),
	HFI_BUF_FW_FLAG_CORRUPT        = BIT(28),
	HFI_BUF_FW_FLAG_UNSUPPORTED    = BIT(29),
};

enum hfi_metapayload_header_flags {
	HFI_METADATA_FLAGS_NONE           = 0,
	HFI_METADATA_FLAGS_TOP_FIELD      = BIT(0),
	HFI_METADATA_FLAGS_BOTTOM_FIELDS  = BIT(1),
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
	HFI_MODE_NONE                  = 0,
	HFI_MODE_PORT_SETTINGS_CHANGE  = BIT(0),
	HFI_MODE_PROPERTY              = BIT(1),
	HFI_MODE_METADATA              = BIT(2),
};

#define HFI_CMD_BEGIN                                           0x0
#define HFI_CMD_INIT                                            0x1
#define HFI_CMD_POWER_COLLAPSE                                  0x2
#define HFI_CMD_OPEN                                            0x3
#define HFI_CMD_CLOSE                                           0x4
#define HFI_CMD_START                                           0x5
#define HFI_CMD_STOP                                            0x6
#define HFI_CMD_DRAIN                                           0x7
#define HFI_CMD_RESUME                                          0x8
#define HFI_CMD_BUFFER                                          0x9
#define HFI_CMD_DELIVERY_MODE                                   0xA
#define HFI_CMD_SUBSCRIBE_MODE                                  0xB
#define HFI_CMD_SETTINGS_CHANGE                                 0xC
#define HFI_CMD_END                                             0xD

#endif //__H_HFI_COMMAND_H__

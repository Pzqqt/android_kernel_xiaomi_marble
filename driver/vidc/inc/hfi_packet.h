/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _HFI_PACKET_H_
#define _HFI_PACKET_H_

#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"

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
	u32 flags;
	u64 timestamp;
	u32 reserved[5];
};

int hfi_packet_sys_init(struct msm_vidc_core *core,
		void *packet, u32 packet_size);
int hfi_packet_image_version(struct msm_vidc_core *core,
		void *packet, u32 packet_size);
int hfi_packet_sys_debug_config(struct msm_vidc_core *core,
		void *packet, u32 packet_size, u32 mode);
int hfi_packet_sys_pc_prep(struct msm_vidc_core *core,
		void *packet, u32 packet_size);
int hfi_packet_session_open(struct msm_vidc_inst *inst,
		void *packet, u32 pkt_size);
int hfi_packet_session_close(struct msm_vidc_inst *inst,
		void *packet, u32 pkt_size);
#endif // _HFI_PACKET_H_

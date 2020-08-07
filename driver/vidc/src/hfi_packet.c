// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "hfi_packet.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_debug.h"

int hfi_packet_sys_init(struct msm_vidc_core *core,
		void *packet, u32 pkt_size)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

int hfi_packet_sys_pc_prep(struct msm_vidc_core *core,
		void *packet, u32 pkt_size)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

int hfi_packet_sys_debug_config(struct msm_vidc_core *core,
		void *packet, u32 pkt_size, u32 config)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

int hfi_packet_image_version(struct msm_vidc_core *core,
		void *packet, u32 pkt_size)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

int hfi_packet_session_open(struct msm_vidc_inst *inst,
		void *packet, u32 pkt_size)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

int hfi_packet_session_close(struct msm_vidc_inst *inst,
		void *packet, u32 pkt_size)
{
	d_vpr_h("%s()\n", __func__);
	return 0;
}

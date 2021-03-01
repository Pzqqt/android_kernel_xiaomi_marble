/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _VENUS_HFI_H_
#define _VENUS_HFI_H_

#include <linux/irqreturn.h>

#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"

#define VIDC_MAX_NAME_LENGTH 		64
#define VIDC_MAX_PC_SKIP_COUNT 		10
#define VIDC_MAX_SUBCACHES 			4
#define VIDC_MAX_SUBCACHE_SIZE 		52

struct vidc_buffer_addr_info {
	enum msm_vidc_buffer_type buffer_type;
	u32 buffer_size;
	u32 num_buffers;
	u32 align_device_addr;
	u32 extradata_addr;
	u32 extradata_size;
	u32 response_required;
};

int venus_hfi_session_property(struct msm_vidc_inst *inst,
	u32 pkt_type, u32 flags, u32 port,
	u32 payload_type, void *payload, u32 payload_size);
int venus_hfi_session_command(struct msm_vidc_inst *inst,
	u32 cmd, enum msm_vidc_port_type port, u32 payload_type,
	void *payload, u32 payload_size);
int venus_hfi_queue_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer, struct msm_vidc_buffer *metabuf);
int venus_hfi_queue_super_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer, struct msm_vidc_buffer *metabuf);
int venus_hfi_release_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer);
int venus_hfi_start(struct msm_vidc_inst *inst, enum msm_vidc_port_type port);
int venus_hfi_stop(struct msm_vidc_inst *inst, enum msm_vidc_port_type port);
int venus_hfi_session_close(struct msm_vidc_inst *inst);
int venus_hfi_session_open(struct msm_vidc_inst *inst);
int venus_hfi_session_set_codec(struct msm_vidc_inst *inst);
int venus_hfi_core_init(struct msm_vidc_core *core);
int venus_hfi_core_deinit(struct msm_vidc_core *core);
int venus_hfi_suspend(struct msm_vidc_core *core);
int venus_hfi_scale_clocks(struct msm_vidc_inst* inst, u64 freq);
int venus_hfi_scale_buses(struct msm_vidc_inst* inst, u64 bw_ddr, u64 bw_llcc);

void venus_hfi_work_handler(struct work_struct *work);
void venus_hfi_pm_work_handler(struct work_struct *work);
irqreturn_t venus_hfi_isr(int irq, void *data);

void __write_register(struct msm_vidc_core *core,
		u32 reg, u32 value);
int __read_register(struct msm_vidc_core *core, u32 reg);
int __iface_cmdq_write(struct msm_vidc_core *core,
	void *pkt);
int __iface_msgq_read(struct msm_vidc_core *core, void *pkt);
int __iface_dbgq_read(struct msm_vidc_core *core, void *pkt);
void __disable_unprepare_clks(struct msm_vidc_core *core);
int __disable_regulators(struct msm_vidc_core *core);
int __unvote_buses(struct msm_vidc_core *core);
int __prepare_pc(struct msm_vidc_core *core);

int __reset_ahb2axi_bridge(struct msm_vidc_core *core);
int __clock_config_on_enable(struct msm_vidc_core *core);
int __interrupt_init(struct msm_vidc_core *core);
int __setup_ucregion_memmap(struct msm_vidc_core *core);
int __raise_interrupt(struct msm_vidc_core *core);
int __power_off(struct msm_vidc_core *core);
bool __core_in_valid_state(struct msm_vidc_core *core);
int __load_fw(struct msm_vidc_core *core);
void __unload_fw(struct msm_vidc_core *core);

#endif // _VENUS_HFI_H_

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/interrupt.h>

#include "msm_vidc_iris2.h"
#include "msm_vidc_buffer_iris2.h"
#include "venus_hfi.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_buffer.h"
#include "msm_vidc_debug.h"


#define VBIF_BASE_OFFS_IRIS2                   0x00080000
#define CPU_BASE_OFFS_IRIS2                    0x000A0000
#define AON_BASE_OFFS			               0x000E0000
#define CPU_CS_BASE_OFFS_IRIS2		           (CPU_BASE_OFFS_IRIS2)
#define CPU_IC_BASE_OFFS_IRIS2		           (CPU_BASE_OFFS_IRIS2)

#define CPU_CS_A2HSOFTINTCLR_IRIS2             (CPU_CS_BASE_OFFS_IRIS2 + 0x1C)
#define CPU_CS_VCICMD_IRIS2                    (CPU_CS_BASE_OFFS_IRIS2 + 0x20)
#define CPU_CS_VCICMDARG0_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x24)
#define CPU_CS_VCICMDARG1_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x28)
#define CPU_CS_VCICMDARG2_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x2C)
#define CPU_CS_VCICMDARG3_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x30)
#define CPU_CS_VMIMSG_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x34)
#define CPU_CS_VMIMSGAG0_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x38)
#define CPU_CS_VMIMSGAG1_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x3C)
#define CPU_CS_SCIACMD_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x48)
#define CPU_CS_H2XSOFTINTEN_IRIS2	(CPU_CS_BASE_OFFS_IRIS2 + 0x148)

/* HFI_CTRL_STATUS */
#define CPU_CS_SCIACMDARG0_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x4C)
#define CPU_CS_SCIACMDARG0_HFI_CTRL_ERROR_STATUS_BMSK_IRIS2	0xfe
#define CPU_CS_SCIACMDARG0_HFI_CTRL_PC_READY_IRIS2           0x100
#define CPU_CS_SCIACMDARG0_HFI_CTRL_INIT_IDLE_MSG_BMSK_IRIS2     0x40000000

/* HFI_QTBL_INFO */
#define CPU_CS_SCIACMDARG1_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x50)

/* HFI_QTBL_ADDR */
#define CPU_CS_SCIACMDARG2_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x54)

/* HFI_VERSION_INFO */
#define CPU_CS_SCIACMDARG3_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x58)

/* SFR_ADDR */
#define CPU_CS_SCIBCMD_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x5C)

/* MMAP_ADDR */
#define CPU_CS_SCIBCMDARG0_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x60)

/* UC_REGION_ADDR */
#define CPU_CS_SCIBARG1_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x64)

/* UC_REGION_ADDR */
#define CPU_CS_SCIBARG2_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x68)

/* FAL10 Feature Control */
#define CPU_CS_X2RPMh_IRIS2		(CPU_CS_BASE_OFFS_IRIS2 + 0x168)
#define CPU_CS_X2RPMh_MASK0_BMSK_IRIS2	0x1
#define CPU_CS_X2RPMh_MASK0_SHFT_IRIS2	0x0
#define CPU_CS_X2RPMh_MASK1_BMSK_IRIS2	0x2
#define CPU_CS_X2RPMh_MASK1_SHFT_IRIS2	0x1
#define CPU_CS_X2RPMh_SWOVERRIDE_BMSK_IRIS2	0x4
#define CPU_CS_X2RPMh_SWOVERRIDE_SHFT_IRIS2	0x3

#define CPU_IC_SOFTINT_IRIS2		(CPU_IC_BASE_OFFS_IRIS2 + 0x150)
#define CPU_IC_SOFTINT_H2A_SHFT_IRIS2	0x0

/*
 * --------------------------------------------------------------------------
 * MODULE: wrapper
 * --------------------------------------------------------------------------
 */
#define WRAPPER_BASE_OFFS_IRIS2		0x000B0000
#define WRAPPER_INTR_STATUS_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x0C)
#define WRAPPER_INTR_STATUS_A2HWD_BMSK_IRIS2	0x8
#define WRAPPER_INTR_STATUS_A2H_BMSK_IRIS2	0x4

#define WRAPPER_INTR_MASK_IRIS2		(WRAPPER_BASE_OFFS_IRIS2 + 0x10)
#define WRAPPER_INTR_MASK_A2HWD_BMSK_IRIS2	0x8
#define WRAPPER_INTR_MASK_A2HCPU_BMSK_IRIS2	0x4

#define WRAPPER_CPU_CLOCK_CONFIG_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x2000)
#define WRAPPER_CPU_CGC_DIS_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x2010)
#define WRAPPER_CPU_STATUS_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x2014)

#define WRAPPER_DEBUG_BRIDGE_LPI_CONTROL_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x54)
#define WRAPPER_DEBUG_BRIDGE_LPI_STATUS_IRIS2	(WRAPPER_BASE_OFFS_IRIS2 + 0x58)
/*
 * --------------------------------------------------------------------------
 * MODULE: tz_wrapper
 * --------------------------------------------------------------------------
 */
#define WRAPPER_TZ_BASE_OFFS	0x000C0000
#define WRAPPER_TZ_CPU_CLOCK_CONFIG	(WRAPPER_TZ_BASE_OFFS)
#define WRAPPER_TZ_CPU_STATUS	(WRAPPER_TZ_BASE_OFFS + 0x10)

#define CTRL_INIT_IRIS2		CPU_CS_SCIACMD_IRIS2

#define CTRL_STATUS_IRIS2	CPU_CS_SCIACMDARG0_IRIS2
#define CTRL_ERROR_STATUS__M_IRIS2 \
		CPU_CS_SCIACMDARG0_HFI_CTRL_ERROR_STATUS_BMSK_IRIS2
#define CTRL_INIT_IDLE_MSG_BMSK_IRIS2 \
		CPU_CS_SCIACMDARG0_HFI_CTRL_INIT_IDLE_MSG_BMSK_IRIS2
#define CTRL_STATUS_PC_READY_IRIS2 \
		CPU_CS_SCIACMDARG0_HFI_CTRL_PC_READY_IRIS2


#define QTBL_INFO_IRIS2		CPU_CS_SCIACMDARG1_IRIS2

#define QTBL_ADDR_IRIS2		CPU_CS_SCIACMDARG2_IRIS2

#define VERSION_INFO_IRIS2	    CPU_CS_SCIACMDARG3_IRIS2

#define SFR_ADDR_IRIS2		    CPU_CS_SCIBCMD_IRIS2
#define MMAP_ADDR_IRIS2		CPU_CS_SCIBCMDARG0_IRIS2
#define UC_REGION_ADDR_IRIS2	CPU_CS_SCIBARG1_IRIS2
#define UC_REGION_SIZE_IRIS2	CPU_CS_SCIBARG2_IRIS2

#define AON_WRAPPER_MVP_NOC_LPI_CONTROL	(AON_BASE_OFFS)
#define AON_WRAPPER_MVP_NOC_LPI_STATUS	(AON_BASE_OFFS + 0x4)

/*
 * --------------------------------------------------------------------------
 * MODULE: vcodec noc error log registers (iris2)
 * --------------------------------------------------------------------------
 */
#define VCODEC_NOC_VIDEO_A_NOC_BASE_OFFS		0x00010000
#define VCODEC_NOC_ERL_MAIN_SWID_LOW			0x00011200
#define VCODEC_NOC_ERL_MAIN_SWID_HIGH			0x00011204
#define VCODEC_NOC_ERL_MAIN_MAINCTL_LOW			0x00011208
#define VCODEC_NOC_ERL_MAIN_ERRVLD_LOW			0x00011210
#define VCODEC_NOC_ERL_MAIN_ERRCLR_LOW			0x00011218
#define VCODEC_NOC_ERL_MAIN_ERRLOG0_LOW			0x00011220
#define VCODEC_NOC_ERL_MAIN_ERRLOG0_HIGH		0x00011224
#define VCODEC_NOC_ERL_MAIN_ERRLOG1_LOW			0x00011228
#define VCODEC_NOC_ERL_MAIN_ERRLOG1_HIGH		0x0001122C
#define VCODEC_NOC_ERL_MAIN_ERRLOG2_LOW			0x00011230
#define VCODEC_NOC_ERL_MAIN_ERRLOG2_HIGH		0x00011234
#define VCODEC_NOC_ERL_MAIN_ERRLOG3_LOW			0x00011238
#define VCODEC_NOC_ERL_MAIN_ERRLOG3_HIGH		0x0001123C

static int __interrupt_init_iris2(struct msm_vidc_core *vidc_core)
{
	u32 mask_val = 0;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/* All interrupts should be disabled initially 0x1F6 : Reset value */
	mask_val = __read_register(core, WRAPPER_INTR_MASK_IRIS2);

	/* Write 0 to unmask CPU and WD interrupts */
	mask_val &= ~(WRAPPER_INTR_MASK_A2HWD_BMSK_IRIS2|
			WRAPPER_INTR_MASK_A2HCPU_BMSK_IRIS2);
	__write_register(core, WRAPPER_INTR_MASK_IRIS2, mask_val);

	return 0;
}

static int __setup_ucregion_memory_map_iris2(struct msm_vidc_core *vidc_core)
{
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	__write_register(core, UC_REGION_ADDR_IRIS2,
			(u32)core->iface_q_table.align_device_addr);
	__write_register(core, UC_REGION_SIZE_IRIS2, SHARED_QSIZE);
	__write_register(core, QTBL_ADDR_IRIS2,
			(u32)core->iface_q_table.align_device_addr);
	__write_register(core, QTBL_INFO_IRIS2, 0x01);
	/* TODO: darshana, remove below comment later with FW support*/
	/*if (core->sfr.align_device_addr)
		__write_register(core, SFR_ADDR_IRIS2,
				(u32)core->sfr.align_device_addr);*/
	/* update queues vaddr for debug purpose */
	__write_register(core, CPU_CS_VCICMDARG0_IRIS2,
		(u32)core->iface_q_table.align_virtual_addr);
	__write_register(core, CPU_CS_VCICMDARG1_IRIS2,
		(u32)((u64)core->iface_q_table.align_virtual_addr >> 32));

	return 0;
}

static int __power_off_iris2(struct msm_vidc_core *vidc_core)
{
	u32 lpi_status, reg_status = 0, count = 0, max_count = 10;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!core->power_enabled)
		return 0;

	if (!(core->intr_status & WRAPPER_INTR_STATUS_A2HWD_BMSK_IRIS2))
		disable_irq_nosync(core->dt->irq);
	core->intr_status = 0;

	/* HPG 6.1.2 Step 1  */
	__write_register(core, CPU_CS_X2RPMh_IRIS2, 0x3);

	/* HPG 6.1.2 Step 2, noc to low power */
	//if (core->res->vpu_ver == VPU_VERSION_IRIS2_1)
	//	goto skip_aon_mvp_noc;

	__write_register(core, AON_WRAPPER_MVP_NOC_LPI_CONTROL, 0x1);
	while (!reg_status && count < max_count) {
		lpi_status =
			 __read_register(core,
				AON_WRAPPER_MVP_NOC_LPI_STATUS);
		reg_status = lpi_status & BIT(0);
		d_vpr_h("Noc: lpi_status %d noc_status %d (count %d)\n",
			lpi_status, reg_status, count);
		usleep_range(50, 100);
		count++;
	}
	if (count == max_count)
		d_vpr_e("NOC not in qaccept status %d\n", reg_status);

//skip_aon_mvp_noc:
	/* HPG 6.1.2 Step 3, debug bridge to low power */
	__write_register(core,
		WRAPPER_DEBUG_BRIDGE_LPI_CONTROL_IRIS2, 0x7);
	reg_status = 0;
	count = 0;
	while ((reg_status != 0x7) && count < max_count) {
		lpi_status = __read_register(core,
				 WRAPPER_DEBUG_BRIDGE_LPI_STATUS_IRIS2);
		reg_status = lpi_status & 0x7;
		d_vpr_h("DBLP Set : lpi_status %d reg_status %d (count %d)\n",
			lpi_status, reg_status, count);
		usleep_range(50, 100);
		count++;
	}
	if (count == max_count)
		d_vpr_e("DBLP Set: status %d\n", reg_status);

	/* HPG 6.1.2 Step 4, debug bridge to lpi release */
	__write_register(core,
		WRAPPER_DEBUG_BRIDGE_LPI_CONTROL_IRIS2, 0x0);
	lpi_status = 0x1;
	count = 0;
	while (lpi_status && count < max_count) {
		lpi_status = __read_register(core,
				 WRAPPER_DEBUG_BRIDGE_LPI_STATUS_IRIS2);
		d_vpr_h("DBLP Release: lpi_status %d(count %d)\n",
			lpi_status, count);
		usleep_range(50, 100);
		count++;
	}
	if (count == max_count)
		d_vpr_e("DBLP Release: lpi_status %d\n", lpi_status);

	/* HPG 6.1.2 Step 6 */
	__disable_unprepare_clks(core);

	/* HPG 6.1.2 Step 5 */
	if (__disable_regulators(core))
		d_vpr_e("%s: Failed to disable regulators\n", __func__);

	if (__unvote_buses(core))
		d_vpr_e("%s: Failed to unvote for buses\n", __func__);
	core->power_enabled = false;

	return 0;
}

static int __prepare_pc_iris2(struct msm_vidc_core *vidc_core)
{
	int rc = 0;
	u32 wfi_status = 0, idle_status = 0, pc_ready = 0;
	u32 ctrl_status = 0;
	int count = 0;
	const int max_tries = 10;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	ctrl_status = __read_register(core, CTRL_STATUS_IRIS2);
	pc_ready = ctrl_status & CTRL_STATUS_PC_READY_IRIS2;
	idle_status = ctrl_status & BIT(30);

	if (pc_ready) {
		d_vpr_h("Already in pc_ready state\n");
		return 0;
	}

	wfi_status = BIT(0) & __read_register(core, WRAPPER_TZ_CPU_STATUS);
	if (!wfi_status || !idle_status) {
		d_vpr_e("Skipping PC, wfi status not set\n");
		goto skip_power_off;
	}

	rc = __prepare_pc(core);
	if (rc) {
		d_vpr_e("Failed __prepare_pc %d\n", rc);
		goto skip_power_off;
	}

	while (count < max_tries) {
		wfi_status = BIT(0) & __read_register(core,
				WRAPPER_TZ_CPU_STATUS);
		ctrl_status = __read_register(core,
				CTRL_STATUS_IRIS2);
		if (wfi_status && (ctrl_status & CTRL_STATUS_PC_READY_IRIS2))
			break;
		usleep_range(150, 250);
		count++;
	}

	if (count == max_tries) {
		d_vpr_e("Skip PC. Core is not in right state\n");
		goto skip_power_off;
	}

	return rc;

skip_power_off:
	d_vpr_e("Skip PC, wfi=%#x, idle=%#x, pcr=%#x, ctrl=%#x)\n",
		wfi_status, idle_status, pc_ready, ctrl_status);
	return -EAGAIN;
}

static int __raise_interrupt_iris2(struct msm_vidc_core *vidc_core)
{
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	__write_register(core, CPU_IC_SOFTINT_IRIS2,
				1 << CPU_IC_SOFTINT_H2A_SHFT_IRIS2);
	return 0;
}

static int __watchdog_iris2(struct msm_vidc_core *vidc_core, u32 intr_status)
{
	int rc = 0;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (intr_status & WRAPPER_INTR_STATUS_A2HWD_BMSK_IRIS2)
		rc = 1;

	return rc;
}

static int __noc_error_info_iris2(struct msm_vidc_core *vidc_core)
{
	u32 val = 0;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	//if (core->res->vpu_ver == VPU_VERSION_IRIS2_1)
	//	return;

	val = __read_register(core, VCODEC_NOC_ERL_MAIN_SWID_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_SWID_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_SWID_HIGH);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_SWID_HIGH:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_MAINCTL_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_MAINCTL_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRVLD_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRVLD_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRCLR_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRCLR_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG0_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG0_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG0_HIGH);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG0_HIGH:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG1_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG1_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG1_HIGH);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG1_HIGH:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG2_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG2_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG2_HIGH);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG2_HIGH:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG3_LOW);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG3_LOW:     %#x\n", val);
	val = __read_register(core, VCODEC_NOC_ERL_MAIN_ERRLOG3_HIGH);
	d_vpr_e("VCODEC_NOC_ERL_MAIN_ERRLOG3_HIGH:     %#x\n", val);

	return 0;
}

static int __clear_interrupt_iris2(struct msm_vidc_core *vidc_core)
{
	u32 intr_status = 0, mask = 0;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: NULL core\n", __func__);
		return 0;
	}

	intr_status = __read_register(core, WRAPPER_INTR_STATUS_IRIS2);
	mask = (WRAPPER_INTR_STATUS_A2H_BMSK_IRIS2|
		WRAPPER_INTR_STATUS_A2HWD_BMSK_IRIS2|
		CTRL_INIT_IDLE_MSG_BMSK_IRIS2);

	if (intr_status & mask) {
		core->intr_status |= intr_status;
		core->reg_count++;
		d_vpr_l("INTERRUPT: times: %d interrupt_status: %d\n",
			core->reg_count, intr_status);
	} else {
		core->spur_count++;
	}

	__write_register(core, CPU_CS_A2HSOFTINTCLR_IRIS2, 1);

	return 0;
}

static int __boot_firmware_iris2(struct msm_vidc_core *vidc_core)
{
	int rc = 0;
	u32 ctrl_init_val = 0, ctrl_status = 0, count = 0, max_tries = 1000;
	struct msm_vidc_core *core = vidc_core;

	if (!core) {
		d_vpr_e("%s: NULL core\n", __func__);
		return 0;
	}

	ctrl_init_val = BIT(0);

	__write_register(core, CTRL_INIT_IRIS2, ctrl_init_val);
	while (!ctrl_status && count < max_tries) {
		ctrl_status = __read_register(core, CTRL_STATUS_IRIS2);
		if ((ctrl_status & CTRL_ERROR_STATUS__M_IRIS2) == 0x4) {
			d_vpr_e("invalid setting for UC_REGION\n");
			break;
		}

		usleep_range(50, 100);
		count++;
	}

	if (count >= max_tries) {
		d_vpr_e("Error booting up vidc firmware\n");
		rc = -ETIME;
	}

	/* Enable interrupt before sending commands to venus */
	__write_register(core, CPU_CS_H2XSOFTINTEN_IRIS2, 0x1);
	__write_register(core, CPU_CS_X2RPMh_IRIS2, 0x0);

	return rc;
}

static struct msm_vidc_venus_ops iris2_ops = {
	.boot_firmware = __boot_firmware_iris2,
	.interrupt_init = __interrupt_init_iris2,
	.raise_interrupt = __raise_interrupt_iris2,
	.clear_interrupt = __clear_interrupt_iris2,
	.setup_ucregion_memmap = __setup_ucregion_memory_map_iris2,
	.clock_config_on_enable = NULL,
	.reset_ahb2axi_bridge = __reset_ahb2axi_bridge,
	.power_off = __power_off_iris2,
	.prepare_pc = __prepare_pc_iris2,
	.watchdog = __watchdog_iris2,
	.noc_error_info = __noc_error_info_iris2,
};

static struct msm_vidc_session_ops msm_session_ops = {
	.buffer_size = msm_buffer_size_iris2,
	.min_count = msm_buffer_min_count_iris2,
	.extra_count = msm_buffer_extra_count_iris2,
	.calc_freq = NULL,
	.calc_bw = NULL,
	.decide_work_route = NULL,
	.decide_work_mode = NULL,
	.decide_core_and_power_mode = NULL,
};

int msm_vidc_init_iris2(struct msm_vidc_core *core)
{
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s()\n", __func__);
	core->venus_ops = &iris2_ops;
	core->session_ops = &msm_session_ops;

	return 0;
}

int msm_vidc_deinit_iris2(struct msm_vidc_core *core)
{
	/* do nothing */
	return 0;
}

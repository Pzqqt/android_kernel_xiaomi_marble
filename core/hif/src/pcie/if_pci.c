/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include <osdep.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#ifdef CONFIG_PCI_MSM
#include <linux/msm_pcie.h>
#endif
#include "hif_io32.h"
#include "if_pci.h"
#include "hif.h"
#include "hif_main.h"
#include "ce_api.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "bmi_msg.h"            /* TARGET_TYPE_ */
#include "regtable.h"
#include "ol_fw.h"
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <osapi_linux.h>
#include "cds_api.h"
#include "cdf_status.h"
#include "cds_sched.h"
#include "wma_api.h"
#include "cdf_atomic.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_main.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#else
#include "cnss_stub.h"
#endif
#include "epping_main.h"
#include "mp_dev.h"
#include "hif_debug.h"

#ifndef REMOVE_PKT_LOG
#include "ol_txrx_types.h"
#include "pktlog_ac_api.h"
#include "pktlog_ac.h"
#endif
#include "if_pci_internal.h"
#include "icnss_stub.h"
#include "ce_tasklet.h"
#include "cds_concurrency.h"

/* Maximum ms timeout for host to wake up target */
#define PCIE_WAKE_TIMEOUT 1000
#define RAMDUMP_EVENT_TIMEOUT 2500

unsigned int msienable = 0;
module_param(msienable, int, 0644);

int hif_pci_war1 = 0;
static DEFINE_SPINLOCK(pciwar_lock);

#ifndef REMOVE_PKT_LOG
struct ol_pl_os_dep_funcs *g_ol_pl_os_dep_funcs = NULL;
#endif

/* Setting SOC_GLOBAL_RESET during driver unload causes intermittent
 * PCIe data bus error
 * As workaround for this issue - changing the reset sequence to
 * use TargetCPU warm reset * instead of SOC_GLOBAL_RESET
 */
#define CPU_WARM_RESET_WAR
/*
 * Top-level interrupt handler for all PCI interrupts from a Target.
 * When a block of MSI interrupts is allocated, this top-level handler
 * is not used; instead, we directly call the correct sub-handler.
 */
struct ce_irq_reg_table {
	uint32_t irq_enable;
	uint32_t irq_status;
};

#if !defined(QCA_WIFI_3_0_ADRASTEA)
static inline void cnss_intr_notify_q6(void)
{
}
#endif

#if !defined(QCA_WIFI_3_0_ADRASTEA)
static inline void *cnss_get_target_smem(void)
{
	return NULL;
}
#endif

#ifndef QCA_WIFI_3_0_ADRASTEA
static inline void hif_pci_route_adrastea_interrupt(struct hif_pci_softc *sc)
{
	return;
}
#else
void hif_pci_route_adrastea_interrupt(struct hif_pci_softc *sc)
{
	struct ol_softc *scn = sc->ol_sc;
	unsigned int target_enable0, target_enable1;
	unsigned int target_cause0, target_cause1;

	target_enable0 = hif_read32_mb(sc->mem + Q6_ENABLE_REGISTER_0);
	target_enable1 = hif_read32_mb(sc->mem + Q6_ENABLE_REGISTER_1);
	target_cause0 = hif_read32_mb(sc->mem + Q6_CAUSE_REGISTER_0);
	target_cause1 = hif_read32_mb(sc->mem + Q6_CAUSE_REGISTER_1);

	if ((target_enable0 & target_cause0) ||
	    (target_enable1 & target_cause1)) {
		hif_write32_mb(sc->mem + Q6_ENABLE_REGISTER_0, 0);
		hif_write32_mb(sc->mem + Q6_ENABLE_REGISTER_1, 0);

		if (scn->notice_send)
			cnss_intr_notify_q6();
	}
}
#endif

static irqreturn_t hif_pci_interrupt_handler(int irq, void *arg)
{
	struct hif_pci_softc *sc = (struct hif_pci_softc *)arg;
	struct ol_softc *scn = sc->ol_sc;
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	volatile int tmp;
	uint16_t val;
	uint32_t bar0;
	uint32_t fw_indicator_address, fw_indicator;
	bool ssr_irq = false;
	unsigned int host_cause, host_enable;

	if (LEGACY_INTERRUPTS(sc)) {
		if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
			return IRQ_HANDLED;

		if (ADRASTEA_BU) {
			host_enable = hif_read32_mb(sc->mem +
						    PCIE_INTR_ENABLE_ADDRESS);
			host_cause = hif_read32_mb(sc->mem +
						   PCIE_INTR_CAUSE_ADDRESS);
			if (!(host_enable & host_cause)) {
				hif_pci_route_adrastea_interrupt(sc);
				return IRQ_HANDLED;
			}
		}

		/* Clear Legacy PCI line interrupts
		 * IMPORTANT: INTR_CLR regiser has to be set
		 * after INTR_ENABLE is set to 0,
		 * otherwise interrupt can not be really cleared */
		hif_write32_mb(sc->mem +
			      (SOC_CORE_BASE_ADDRESS |
			       PCIE_INTR_ENABLE_ADDRESS), 0);

		hif_write32_mb(sc->mem +
			      (SOC_CORE_BASE_ADDRESS | PCIE_INTR_CLR_ADDRESS),
			       ADRASTEA_BU ?
			       (host_enable & host_cause) :
			      HOST_GROUP0_MASK);

		if (ADRASTEA_BU)
			hif_write32_mb(sc->mem + 0x2f100c , (host_cause >> 1));

		/* IMPORTANT: this extra read transaction is required to
		 * flush the posted write buffer */
		if (!ADRASTEA_BU) {
		tmp =
			hif_read32_mb(sc->mem +
				     (SOC_CORE_BASE_ADDRESS |
				      PCIE_INTR_ENABLE_ADDRESS));

		if (tmp == 0xdeadbeef) {
			HIF_ERROR("BUG(%s): SoC returns 0xdeadbeef!!",
			       __func__);

			pci_read_config_word(sc->pdev, PCI_VENDOR_ID, &val);
			HIF_ERROR("%s: PCI Vendor ID = 0x%04x",
			       __func__, val);

			pci_read_config_word(sc->pdev, PCI_DEVICE_ID, &val);
			HIF_ERROR("%s: PCI Device ID = 0x%04x",
			       __func__, val);

			pci_read_config_word(sc->pdev, PCI_COMMAND, &val);
			HIF_ERROR("%s: PCI Command = 0x%04x", __func__,
			       val);

			pci_read_config_word(sc->pdev, PCI_STATUS, &val);
			HIF_ERROR("%s: PCI Status = 0x%04x", __func__,
			       val);

			pci_read_config_dword(sc->pdev, PCI_BASE_ADDRESS_0,
					      &bar0);
			HIF_ERROR("%s: PCI BAR0 = 0x%08x", __func__,
			       bar0);

			HIF_ERROR("%s: RTC_STATE_ADDRESS = 0x%08x",
				  __func__,
				  hif_read32_mb(sc->mem +
						PCIE_LOCAL_BASE_ADDRESS
						+ RTC_STATE_ADDRESS));
			HIF_ERROR("%s: PCIE_SOC_WAKE_ADDRESS = 0x%08x",
				  __func__,
				  hif_read32_mb(sc->mem +
						PCIE_LOCAL_BASE_ADDRESS
						+ PCIE_SOC_WAKE_ADDRESS));
			HIF_ERROR("%s: 0x80008 = 0x%08x, 0x8000c = 0x%08x",
				  __func__,
				  hif_read32_mb(sc->mem + 0x80008),
				  hif_read32_mb(sc->mem + 0x8000c));
			HIF_ERROR("%s: 0x80010 = 0x%08x, 0x80014 = 0x%08x",
				  __func__,
				  hif_read32_mb(sc->mem + 0x80010),
				  hif_read32_mb(sc->mem + 0x80014));
			HIF_ERROR("%s: 0x80018 = 0x%08x, 0x8001c = 0x%08x",
				  __func__,
				  hif_read32_mb(sc->mem + 0x80018),
				  hif_read32_mb(sc->mem + 0x8001c));
			CDF_BUG(0);
		}

		PCI_CLR_CAUSE0_REGISTER(sc);
		}

		if (HAS_FW_INDICATOR) {
			fw_indicator_address = hif_state->fw_indicator_address;
			fw_indicator = A_TARGET_READ(scn, fw_indicator_address);
			if ((fw_indicator != ~0) &&
			   (fw_indicator & FW_IND_EVENT_PENDING))
				ssr_irq = true;
		}

		if (Q_TARGET_ACCESS_END(scn) < 0)
			return IRQ_HANDLED;
	}
	/* TBDXXX: Add support for WMAC */

	if (ssr_irq) {
		sc->irq_event = irq;
		cdf_atomic_set(&scn->tasklet_from_intr, 1);

		cdf_atomic_inc(&scn->active_tasklet_cnt);
		tasklet_schedule(&sc->intr_tq);
	} else {
		icnss_dispatch_ce_irq(scn);
	}

	return IRQ_HANDLED;
}

static irqreturn_t hif_pci_msi_fw_handler(int irq, void *arg)
{
	struct hif_pci_softc *sc = (struct hif_pci_softc *)arg;

	(irqreturn_t) hif_fw_interrupt_handler(sc->irq_event, sc->ol_sc);

	return IRQ_HANDLED;
}

bool hif_targ_is_awake(struct ol_softc *scn, void *__iomem *mem)
{
	HIF_PCI_TARG_IS_AWAKE(scn, mem);
}

bool hif_pci_targ_is_present(struct ol_softc *scn, void *__iomem *mem)
{
	return 1;               /* FIX THIS */
}

/**
 * hif_pci_cancel_deferred_target_sleep() - cancels the defered target sleep
 * @scn: ol_softc
 *
 * Return: void
 */
#if CONFIG_ATH_PCIE_MAX_PERF == 0
void hif_pci_cancel_deferred_target_sleep(struct ol_softc *scn)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	A_target_id_t pci_addr = scn->mem;

	cdf_spin_lock_irqsave(&hif_state->keep_awake_lock);
	/*
	 * If the deferred sleep timer is running cancel it
	 * and put the soc into sleep.
	 */
	if (hif_state->fake_sleep == true) {
		cdf_softirq_timer_cancel(&hif_state->sleep_timer);
		if (hif_state->verified_awake == false) {
			hif_write32_mb(pci_addr + PCIE_LOCAL_BASE_ADDRESS +
				      PCIE_SOC_WAKE_ADDRESS,
				      PCIE_SOC_WAKE_RESET);
		}
		hif_state->fake_sleep = false;
	}
	cdf_spin_unlock_irqrestore(&hif_state->keep_awake_lock);
}
#else
inline void hif_pci_cancel_deferred_target_sleep(struct ol_softc *scn)
{
	return;
}
#endif

#define A_PCIE_LOCAL_REG_READ(mem, addr) \
	hif_read32_mb((char *)(mem) + \
	PCIE_LOCAL_BASE_ADDRESS + (uint32_t)(addr))

#define A_PCIE_LOCAL_REG_WRITE(mem, addr, val) \
	hif_write32_mb(((char *)(mem) + \
	PCIE_LOCAL_BASE_ADDRESS + (uint32_t)(addr)), (val))

#define ATH_PCI_RESET_WAIT_MAX 10       /* Ms */
static void hif_pci_device_reset(struct hif_pci_softc *sc)
{
	void __iomem *mem = sc->mem;
	int i;
	uint32_t val;
	struct ol_softc *scn = sc->ol_sc;

	if (!scn->hostdef)
		return;

	/* NB: Don't check resetok here.  This form of reset
	 * is integral to correct operation. */

	if (!SOC_GLOBAL_RESET_ADDRESS) {
		return;
	}

	if (!mem) {
		return;
	}

	HIF_ERROR("%s: Reset Device", __func__);

	/*
	 * NB: If we try to write SOC_GLOBAL_RESET_ADDRESS without first
	 * writing WAKE_V, the Target may scribble over Host memory!
	 */
	A_PCIE_LOCAL_REG_WRITE(mem, PCIE_SOC_WAKE_ADDRESS,
			       PCIE_SOC_WAKE_V_MASK);
	for (i = 0; i < ATH_PCI_RESET_WAIT_MAX; i++) {
		if (hif_targ_is_awake(scn, mem))
			break;

		cdf_mdelay(1);
	}

	/* Put Target, including PCIe, into RESET. */
	val = A_PCIE_LOCAL_REG_READ(mem, SOC_GLOBAL_RESET_ADDRESS);
	val |= 1;
	A_PCIE_LOCAL_REG_WRITE(mem, SOC_GLOBAL_RESET_ADDRESS, val);
	for (i = 0; i < ATH_PCI_RESET_WAIT_MAX; i++) {
		if (A_PCIE_LOCAL_REG_READ(mem, RTC_STATE_ADDRESS) &
		    RTC_STATE_COLD_RESET_MASK)
			break;

		cdf_mdelay(1);
	}

	/* Pull Target, including PCIe, out of RESET. */
	val &= ~1;
	A_PCIE_LOCAL_REG_WRITE(mem, SOC_GLOBAL_RESET_ADDRESS, val);
	for (i = 0; i < ATH_PCI_RESET_WAIT_MAX; i++) {
		if (!
		    (A_PCIE_LOCAL_REG_READ(mem, RTC_STATE_ADDRESS) &
		     RTC_STATE_COLD_RESET_MASK))
			break;

		cdf_mdelay(1);
	}

	A_PCIE_LOCAL_REG_WRITE(mem, PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_RESET);
}

/* CPU warm reset function
 * Steps:
 * 1. Disable all pending interrupts - so no pending interrupts on WARM reset
 * 2. Clear the FW_INDICATOR_ADDRESS -so Traget CPU intializes FW
 *    correctly on WARM reset
 * 3. Clear TARGET CPU LF timer interrupt
 * 4. Reset all CEs to clear any pending CE tarnsactions
 * 5. Warm reset CPU
 */
void hif_pci_device_warm_reset(struct hif_pci_softc *sc)
{
	void __iomem *mem = sc->mem;
	int i;
	uint32_t val;
	uint32_t fw_indicator;
	struct ol_softc *scn = sc->ol_sc;

	/* NB: Don't check resetok here.  This form of reset is
	 * integral to correct operation. */

	if (!mem) {
		return;
	}

	HIF_INFO_MED("%s: Target Warm Reset", __func__);

	/*
	 * NB: If we try to write SOC_GLOBAL_RESET_ADDRESS without first
	 * writing WAKE_V, the Target may scribble over Host memory!
	 */
	A_PCIE_LOCAL_REG_WRITE(mem, PCIE_SOC_WAKE_ADDRESS,
			       PCIE_SOC_WAKE_V_MASK);
	for (i = 0; i < ATH_PCI_RESET_WAIT_MAX; i++) {
		if (hif_targ_is_awake(scn, mem))
			break;
		cdf_mdelay(1);
	}

	/*
	 * Disable Pending interrupts
	 */
	val =
		hif_read32_mb(mem +
			     (SOC_CORE_BASE_ADDRESS |
			      PCIE_INTR_CAUSE_ADDRESS));
	HIF_INFO_MED("%s: Host Intr Cause reg 0x%x : value : 0x%x", __func__,
		    (SOC_CORE_BASE_ADDRESS | PCIE_INTR_CAUSE_ADDRESS), val);
	/* Target CPU Intr Cause */
	val = hif_read32_mb(mem + (SOC_CORE_BASE_ADDRESS | CPU_INTR_ADDRESS));
	HIF_INFO_MED("%s: Target CPU Intr Cause 0x%x", __func__, val);

	val =
		hif_read32_mb(mem +
			     (SOC_CORE_BASE_ADDRESS |
			      PCIE_INTR_ENABLE_ADDRESS));
	hif_write32_mb((mem +
		       (SOC_CORE_BASE_ADDRESS | PCIE_INTR_ENABLE_ADDRESS)), 0);
	hif_write32_mb((mem + (SOC_CORE_BASE_ADDRESS + PCIE_INTR_CLR_ADDRESS)),
		      HOST_GROUP0_MASK);

	cdf_mdelay(100);

	/* Clear FW_INDICATOR_ADDRESS */
	if (HAS_FW_INDICATOR) {
		fw_indicator = hif_read32_mb(mem + FW_INDICATOR_ADDRESS);
		hif_write32_mb(mem + FW_INDICATOR_ADDRESS, 0);
	}

	/* Clear Target LF Timer interrupts */
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS +
			      SOC_LF_TIMER_CONTROL0_ADDRESS));
	HIF_INFO_MED("%s: addr 0x%x :  0x%x", __func__,
	       (RTC_SOC_BASE_ADDRESS + SOC_LF_TIMER_CONTROL0_ADDRESS), val);
	val &= ~SOC_LF_TIMER_CONTROL0_ENABLE_MASK;
	hif_write32_mb(mem +
		      (RTC_SOC_BASE_ADDRESS + SOC_LF_TIMER_CONTROL0_ADDRESS),
		      val);

	/* Reset CE */
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS |
			      SOC_RESET_CONTROL_ADDRESS));
	val |= SOC_RESET_CONTROL_CE_RST_MASK;
	hif_write32_mb((mem +
		       (RTC_SOC_BASE_ADDRESS | SOC_RESET_CONTROL_ADDRESS)),
		      val);
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS |
			      SOC_RESET_CONTROL_ADDRESS));
	cdf_mdelay(10);

	/* CE unreset */
	val &= ~SOC_RESET_CONTROL_CE_RST_MASK;
	hif_write32_mb(mem + (RTC_SOC_BASE_ADDRESS | SOC_RESET_CONTROL_ADDRESS),
		      val);
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS |
			      SOC_RESET_CONTROL_ADDRESS));
	cdf_mdelay(10);

	/* Read Target CPU Intr Cause */
	val = hif_read32_mb(mem + (SOC_CORE_BASE_ADDRESS | CPU_INTR_ADDRESS));
	HIF_INFO_MED("%s: Target CPU Intr Cause after CE reset 0x%x",
		    __func__, val);

	/* CPU warm RESET */
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS |
			      SOC_RESET_CONTROL_ADDRESS));
	val |= SOC_RESET_CONTROL_CPU_WARM_RST_MASK;
	hif_write32_mb(mem + (RTC_SOC_BASE_ADDRESS | SOC_RESET_CONTROL_ADDRESS),
		      val);
	val =
		hif_read32_mb(mem +
			     (RTC_SOC_BASE_ADDRESS |
			      SOC_RESET_CONTROL_ADDRESS));
	HIF_INFO_MED("%s: RESET_CONTROL after cpu warm reset 0x%x",
		    __func__, val);

	cdf_mdelay(100);
	HIF_INFO_MED("%s: Target Warm reset complete", __func__);

}

#ifndef QCA_WIFI_3_0
int hif_check_fw_reg(struct ol_softc *scn)
{
	struct hif_pci_softc *sc = scn->hif_sc;
	void __iomem *mem = sc->mem;
	uint32_t val;

	A_TARGET_ACCESS_BEGIN_RET(scn);
	val = hif_read32_mb(mem + FW_INDICATOR_ADDRESS);
	A_TARGET_ACCESS_END_RET(scn);

	HIF_INFO_MED("%s: FW_INDICATOR register is 0x%x", __func__, val);

	if (val & FW_IND_HELPER)
		return 0;

	return 1;
}
#endif

int hif_check_soc_status(struct ol_softc *scn)
{
	uint16_t device_id;
	uint32_t val;
	uint16_t timeout_count = 0;
	struct hif_pci_softc *sc = scn->hif_sc;

	/* Check device ID from PCIe configuration space for link status */
	pci_read_config_word(sc->pdev, PCI_DEVICE_ID, &device_id);
	if (device_id != sc->devid) {
		HIF_ERROR("%s: device ID does match (read 0x%x, expect 0x%x)",
			  __func__, device_id, sc->devid);
		return -EACCES;
	}

	/* Check PCIe local register for bar/memory access */
	val = hif_read32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
			   RTC_STATE_ADDRESS);
	HIF_INFO_MED("%s: RTC_STATE_ADDRESS is %08x", __func__, val);

	/* Try to wake up taget if it sleeps */
	hif_write32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
		PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_V_MASK);
	HIF_INFO_MED("%s: PCIE_SOC_WAKE_ADDRESS is %08x", __func__,
		hif_read32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
		PCIE_SOC_WAKE_ADDRESS));

	/* Check if taget can be woken up */
	while (!hif_targ_is_awake(scn, sc->mem)) {
		if (timeout_count >= PCIE_WAKE_TIMEOUT) {
			HIF_ERROR("%s: wake up timeout, %08x, %08x",
				__func__,
				hif_read32_mb(sc->mem +
					     PCIE_LOCAL_BASE_ADDRESS +
					     RTC_STATE_ADDRESS),
				hif_read32_mb(sc->mem +
					     PCIE_LOCAL_BASE_ADDRESS +
					PCIE_SOC_WAKE_ADDRESS));
			return -EACCES;
		}

		hif_write32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
			      PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_V_MASK);

		cdf_mdelay(100);
		timeout_count += 100;
	}

	/* Check Power register for SoC internal bus issues */
	val =
		hif_read32_mb(sc->mem + RTC_SOC_BASE_ADDRESS +
			     SOC_POWER_REG_OFFSET);
	HIF_INFO_MED("%s: Power register is %08x", __func__, val);

	return 0;
}

void dump_ce_debug_register(struct ol_softc *scn)
{
	struct hif_pci_softc *sc = scn->hif_sc;
	void __iomem *mem = sc->mem;
	uint32_t val, i, j;
	uint32_t wrapper_idx[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	uint32_t ce_base;

	A_TARGET_ACCESS_BEGIN(scn);

	/* DEBUG_INPUT_SEL_SRC = 0x6 */
	val =
		hif_read32_mb(mem + GPIO_BASE_ADDRESS +
			     WLAN_DEBUG_INPUT_SEL_OFFSET);
	val &= ~WLAN_DEBUG_INPUT_SEL_SRC_MASK;
	val |= WLAN_DEBUG_INPUT_SEL_SRC_SET(0x6);
	hif_write32_mb(mem + GPIO_BASE_ADDRESS + WLAN_DEBUG_INPUT_SEL_OFFSET,
		      val);

	/* DEBUG_CONTROL_ENABLE = 0x1 */
	val = hif_read32_mb(mem + GPIO_BASE_ADDRESS +
			   WLAN_DEBUG_CONTROL_OFFSET);
	val &= ~WLAN_DEBUG_CONTROL_ENABLE_MASK;
	val |= WLAN_DEBUG_CONTROL_ENABLE_SET(0x1);
	hif_write32_mb(mem + GPIO_BASE_ADDRESS +
		      WLAN_DEBUG_CONTROL_OFFSET, val);

	HIF_INFO_MED("%s: Debug: inputsel: %x dbgctrl: %x", __func__,
	       hif_read32_mb(mem + GPIO_BASE_ADDRESS +
			    WLAN_DEBUG_INPUT_SEL_OFFSET),
	       hif_read32_mb(mem + GPIO_BASE_ADDRESS +
			    WLAN_DEBUG_CONTROL_OFFSET));

	HIF_INFO_MED("%s: Debug CE", __func__);
	/* Loop CE debug output */
	/* AMBA_DEBUG_BUS_SEL = 0xc */
	val = hif_read32_mb(mem + GPIO_BASE_ADDRESS + AMBA_DEBUG_BUS_OFFSET);
	val &= ~AMBA_DEBUG_BUS_SEL_MASK;
	val |= AMBA_DEBUG_BUS_SEL_SET(0xc);
	hif_write32_mb(mem + GPIO_BASE_ADDRESS + AMBA_DEBUG_BUS_OFFSET, val);

	for (i = 0; i < sizeof(wrapper_idx) / sizeof(uint32_t); i++) {
		/* For (i=1,2,3,4,8,9) write CE_WRAPPER_DEBUG_SEL = i */
		val = hif_read32_mb(mem + CE_WRAPPER_BASE_ADDRESS +
				   CE_WRAPPER_DEBUG_OFFSET);
		val &= ~CE_WRAPPER_DEBUG_SEL_MASK;
		val |= CE_WRAPPER_DEBUG_SEL_SET(wrapper_idx[i]);
		hif_write32_mb(mem + CE_WRAPPER_BASE_ADDRESS +
			      CE_WRAPPER_DEBUG_OFFSET, val);

		HIF_INFO_MED("%s: ce wrapper: %d amdbg: %x cewdbg: %x",
			    __func__, wrapper_idx[i],
			    hif_read32_mb(mem + GPIO_BASE_ADDRESS +
				AMBA_DEBUG_BUS_OFFSET),
			    hif_read32_mb(mem + CE_WRAPPER_BASE_ADDRESS +
				CE_WRAPPER_DEBUG_OFFSET));

		if (wrapper_idx[i] <= 7) {
			for (j = 0; j <= 5; j++) {
				ce_base = CE_BASE_ADDRESS(wrapper_idx[i]);
				/* For (j=0~5) write CE_DEBUG_SEL = j */
				val =
					hif_read32_mb(mem + ce_base +
						     CE_DEBUG_OFFSET);
				val &= ~CE_DEBUG_SEL_MASK;
				val |= CE_DEBUG_SEL_SET(j);
				hif_write32_mb(mem + ce_base + CE_DEBUG_OFFSET,
					      val);

				/* read (@gpio_athr_wlan_reg)
				 * WLAN_DEBUG_OUT_DATA */
				val = hif_read32_mb(mem + GPIO_BASE_ADDRESS +
						   WLAN_DEBUG_OUT_OFFSET);
				val = WLAN_DEBUG_OUT_DATA_GET(val);

				HIF_INFO_MED("%s: module%d: cedbg: %x out: %x",
					    __func__, j,
					    hif_read32_mb(mem + ce_base +
						    CE_DEBUG_OFFSET), val);
			}
		} else {
			/* read (@gpio_athr_wlan_reg) WLAN_DEBUG_OUT_DATA */
			val =
				hif_read32_mb(mem + GPIO_BASE_ADDRESS +
					     WLAN_DEBUG_OUT_OFFSET);
			val = WLAN_DEBUG_OUT_DATA_GET(val);

			HIF_INFO_MED("%s: out: %x", __func__, val);
		}
	}

	HIF_INFO_MED("%s: Debug PCIe:", __func__);
	/* Loop PCIe debug output */
	/* Write AMBA_DEBUG_BUS_SEL = 0x1c */
	val = hif_read32_mb(mem + GPIO_BASE_ADDRESS + AMBA_DEBUG_BUS_OFFSET);
	val &= ~AMBA_DEBUG_BUS_SEL_MASK;
	val |= AMBA_DEBUG_BUS_SEL_SET(0x1c);
	hif_write32_mb(mem + GPIO_BASE_ADDRESS + AMBA_DEBUG_BUS_OFFSET, val);

	for (i = 0; i <= 8; i++) {
		/* For (i=1~8) write AMBA_DEBUG_BUS_PCIE_DEBUG_SEL = i */
		val =
			hif_read32_mb(mem + GPIO_BASE_ADDRESS +
				     AMBA_DEBUG_BUS_OFFSET);
		val &= ~AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MASK;
		val |= AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_SET(i);
		hif_write32_mb(mem + GPIO_BASE_ADDRESS + AMBA_DEBUG_BUS_OFFSET,
			      val);

		/* read (@gpio_athr_wlan_reg) WLAN_DEBUG_OUT_DATA */
		val =
			hif_read32_mb(mem + GPIO_BASE_ADDRESS +
				     WLAN_DEBUG_OUT_OFFSET);
		val = WLAN_DEBUG_OUT_DATA_GET(val);

		HIF_INFO_MED("%s: amdbg: %x out: %x %x", __func__,
		       hif_read32_mb(mem + GPIO_BASE_ADDRESS +
				    WLAN_DEBUG_OUT_OFFSET), val,
		       hif_read32_mb(mem + GPIO_BASE_ADDRESS +
				    WLAN_DEBUG_OUT_OFFSET));
	}

	A_TARGET_ACCESS_END(scn);
}

/*
 * Handler for a per-engine interrupt on a PARTICULAR CE.
 * This is used in cases where each CE has a private
 * MSI interrupt.
 */
static irqreturn_t ce_per_engine_handler(int irq, void *arg)
{
	struct hif_pci_softc *sc = (struct hif_pci_softc *)arg;
	int CE_id = irq - MSI_ASSIGN_CE_INITIAL;

	/*
	 * NOTE: We are able to derive CE_id from irq because we
	 * use a one-to-one mapping for CE's 0..5.
	 * CE's 6 & 7 do not use interrupts at all.
	 *
	 * This mapping must be kept in sync with the mapping
	 * used by firmware.
	 */

	ce_per_engine_service(sc->ol_sc, CE_id);

	return IRQ_HANDLED;
}

#ifdef CONFIG_SLUB_DEBUG_ON

/* worker thread to schedule wlan_tasklet in SLUB debug build */
static void reschedule_tasklet_work_handler(struct work_struct *recovery)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct hif_pci_softc *sc;

	if (NULL == scn) {
		HIF_ERROR("%s: tasklet scn is null", __func__);
		return;
	}

	sc = scn->hif_sc;

	if (scn->hif_init_done == false) {
		HIF_ERROR("%s: wlan driver is unloaded", __func__);
		return;
	}

	tasklet_schedule(&sc->intr_tq);
	return;
}

static DECLARE_WORK(reschedule_tasklet_work, reschedule_tasklet_work_handler);

#endif

static void wlan_tasklet(unsigned long data)
{
	struct hif_pci_softc *sc = (struct hif_pci_softc *)data;
	struct ol_softc *scn = sc->ol_sc;

	if (scn->hif_init_done == false)
		goto end;

	if (cdf_atomic_read(&scn->link_suspended))
		goto end;

	if (!ADRASTEA_BU) {
		(irqreturn_t) hif_fw_interrupt_handler(sc->irq_event, scn);
		if (sc->ol_sc->target_status == OL_TRGET_STATUS_RESET)
			goto end;
	}

end:
	cdf_atomic_set(&scn->tasklet_from_intr, 0);
	cdf_atomic_dec(&scn->active_tasklet_cnt);
}

#ifdef FEATURE_RUNTIME_PM
#define HIF_PCI_RUNTIME_PM_STATS(_s, _sc, _name) \
	seq_printf(_s, "%30s: %u\n", #_name, _sc->pm_stats._name)

/**
 * hif_pci_runtime_pm_warn() - Runtime PM Debugging API
 * @sc: hif_pci_softc context
 * @msg: log message
 *
 * log runtime pm stats when something seems off.
 *
 * Return: void
 */
void hif_pci_runtime_pm_warn(struct hif_pci_softc *sc, const char *msg)
{
	struct hif_pm_runtime_lock *ctx;

	HIF_ERROR("%s: usage_count: %d, pm_state: %d, prevent_suspend_cnt: %d",
			msg, atomic_read(&sc->dev->power.usage_count),
			atomic_read(&sc->pm_state),
			sc->prevent_suspend_cnt);

	HIF_ERROR("runtime_status: %d, runtime_error: %d, disable_depth: %d autosuspend_delay: %d",
			sc->dev->power.runtime_status,
			sc->dev->power.runtime_error,
			sc->dev->power.disable_depth,
			sc->dev->power.autosuspend_delay);

	HIF_ERROR("runtime_get: %u, runtime_put: %u, request_resume: %u",
			sc->pm_stats.runtime_get, sc->pm_stats.runtime_put,
			sc->pm_stats.request_resume);

	HIF_ERROR("allow_suspend: %u, prevent_suspend: %u",
			sc->pm_stats.allow_suspend,
			sc->pm_stats.prevent_suspend);

	HIF_ERROR("prevent_suspend_timeout: %u, allow_suspend_timeout: %u",
			sc->pm_stats.prevent_suspend_timeout,
			sc->pm_stats.allow_suspend_timeout);

	HIF_ERROR("Suspended: %u, resumed: %u count",
			sc->pm_stats.suspended,
			sc->pm_stats.resumed);

	HIF_ERROR("suspend_err: %u, runtime_get_err: %u",
			sc->pm_stats.suspend_err,
			sc->pm_stats.runtime_get_err);

	HIF_ERROR("Active Wakeup Sources preventing Runtime Suspend: ");

	list_for_each_entry(ctx, &sc->prevent_suspend_list, list) {
		HIF_ERROR("source %s; timeout %d ms", ctx->name, ctx->timeout);
	}

	WARN_ON(1);
}

/**
 * hif_pci_pm_runtime_debugfs_show(): show debug stats for runtimepm
 * @s: file to print to
 * @data: unused
 *
 * debugging tool added to the debug fs for displaying runtimepm stats
 *
 * Return: 0
 */
static int hif_pci_pm_runtime_debugfs_show(struct seq_file *s, void *data)
{
	struct hif_pci_softc *sc = s->private;
	static const char * const autopm_state[] = {"NONE", "ON", "INPROGRESS",
		"SUSPENDED"};
	unsigned int msecs_age;
	int pm_state = atomic_read(&sc->pm_state);
	unsigned long timer_expires, flags;
	struct hif_pm_runtime_lock *ctx;

	seq_printf(s, "%30s: %s\n", "Runtime PM state",
			autopm_state[pm_state]);
	seq_printf(s, "%30s: %pf\n", "Last Resume Caller",
			sc->pm_stats.last_resume_caller);

	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED) {
		msecs_age = jiffies_to_msecs(
				jiffies - sc->pm_stats.suspend_jiffies);
		seq_printf(s, "%30s: %d.%03ds\n", "Suspended Since",
				msecs_age / 1000, msecs_age % 1000);
	}

	seq_printf(s, "%30s: %d\n", "PM Usage count",
			atomic_read(&sc->dev->power.usage_count));

	seq_printf(s, "%30s: %u\n", "prevent_suspend_cnt",
			sc->prevent_suspend_cnt);

	HIF_PCI_RUNTIME_PM_STATS(s, sc, suspended);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, suspend_err);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, resumed);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, runtime_get);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, runtime_put);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, request_resume);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, prevent_suspend);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, allow_suspend);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, prevent_suspend_timeout);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, allow_suspend_timeout);
	HIF_PCI_RUNTIME_PM_STATS(s, sc, runtime_get_err);

	timer_expires = sc->runtime_timer_expires;
	if (timer_expires > 0) {
		msecs_age = jiffies_to_msecs(timer_expires - jiffies);
		seq_printf(s, "%30s: %d.%03ds\n", "Prevent suspend timeout",
				msecs_age / 1000, msecs_age % 1000);
	}

	spin_lock_irqsave(&sc->runtime_lock, flags);
	if (list_empty(&sc->prevent_suspend_list)) {
		spin_unlock_irqrestore(&sc->runtime_lock, flags);
		return 0;
	}

	seq_printf(s, "%30s: ", "Active Wakeup_Sources");
	list_for_each_entry(ctx, &sc->prevent_suspend_list, list) {
		seq_printf(s, "%s", ctx->name);
		if (ctx->timeout)
			seq_printf(s, "(%d ms)", ctx->timeout);
		seq_puts(s, " ");
	}
	seq_puts(s, "\n");
	spin_unlock_irqrestore(&sc->runtime_lock, flags);

	return 0;
}
#undef HIF_PCI_RUNTIME_PM_STATS

/**
 * hif_pci_autopm_open() - open a debug fs file to access the runtime pm stats
 * @inode
 * @file
 *
 * Return: linux error code of single_open.
 */
static int hif_pci_runtime_pm_open(struct inode *inode, struct file *file)
{
	return single_open(file, hif_pci_pm_runtime_debugfs_show,
			inode->i_private);
}

#ifdef WLAN_OPEN_SOURCE
static const struct file_operations hif_pci_runtime_pm_fops = {
	.owner          = THIS_MODULE,
	.open           = hif_pci_runtime_pm_open,
	.release        = single_release,
	.read           = seq_read,
	.llseek         = seq_lseek,
};

/**
 * hif_runtime_pm_debugfs_create() - creates runtimepm debugfs entry
 * @sc: pci context
 *
 * creates a debugfs entry to debug the runtime pm feature.
 */
static void hif_runtime_pm_debugfs_create(struct hif_pci_softc *sc)
{
	sc->pm_dentry = debugfs_create_file("cnss_runtime_pm",
					S_IRUSR, NULL, sc,
					&hif_pci_runtime_pm_fops);
}
/**
 * hif_runtime_pm_debugfs_remove() - removes runtimepm debugfs entry
 * @sc: pci context
 *
 * removes the debugfs entry to debug the runtime pm feature.
 */
static void hif_runtime_pm_debugfs_remove(struct hif_pci_softc *sc)
{
	debugfs_remove(sc->pm_dentry);
}
#else
static inline void hif_runtime_pm_debugfs_create(struct hif_pci_softc *sc)
{
}
static inline void hif_runtime_pm_debugfs_remove(struct hif_pci_softc *sc)
{
}
#endif

static void hif_pm_runtime_lock_timeout_fn(unsigned long data);

/**
 * hif_pm_runtime_start(): start the runtime pm
 * @sc: pci context
 *
 * After this call, runtime pm will be active.
 */
static void hif_pm_runtime_start(struct hif_pci_softc *sc)
{
	struct ol_softc *ol_sc;

	ol_sc = sc->ol_sc;

	if (!ol_sc->enable_runtime_pm) {
		HIF_INFO("%s: RUNTIME PM is disabled in ini\n", __func__);
		return;
	}

	if (cds_get_conparam() == CDF_FTM_MODE ||
			WLAN_IS_EPPING_ENABLED(cds_get_conparam())) {
		HIF_INFO("%s: RUNTIME PM is disabled for FTM/EPPING mode\n",
				__func__);
		return;
	}

	setup_timer(&sc->runtime_timer, hif_pm_runtime_lock_timeout_fn,
			(unsigned long)sc);

	HIF_INFO("%s: Enabling RUNTIME PM, Delay: %d ms", __func__,
			ol_sc->runtime_pm_delay);

	cnss_runtime_init(sc->dev, ol_sc->runtime_pm_delay);
	cdf_atomic_set(&sc->pm_state, HIF_PM_RUNTIME_STATE_ON);
	hif_runtime_pm_debugfs_create(sc);
}

/**
 * hif_pm_runtime_stop(): stop runtime pm
 * @sc: pci context
 *
 * Turns off runtime pm and frees corresponding resources
 * that were acquired by hif_runtime_pm_start().
 */
static void hif_pm_runtime_stop(struct hif_pci_softc *sc)
{
	struct ol_softc *ol_sc = sc->ol_sc;

	if (!ol_sc->enable_runtime_pm)
		return;

	if (cds_get_conparam() == CDF_FTM_MODE ||
			WLAN_IS_EPPING_ENABLED(cds_get_conparam()))
		return;

	cnss_runtime_exit(sc->dev);
	cnss_pm_runtime_request(sc->dev, CNSS_PM_RUNTIME_RESUME);

	cdf_atomic_set(&sc->pm_state, HIF_PM_RUNTIME_STATE_NONE);

	hif_runtime_pm_debugfs_remove(sc);
	del_timer_sync(&sc->runtime_timer);
	/* doesn't wait for penting trafic unlike cld-2.0 */
}

/**
 * hif_pm_runtime_open(): initialize runtime pm
 * @sc: pci data structure
 *
 * Early initialization
 */
static void hif_pm_runtime_open(struct hif_pci_softc *sc)
{
	spin_lock_init(&sc->runtime_lock);

	cdf_atomic_init(&sc->pm_state);
	sc->prevent_linkdown_lock =
		hif_runtime_lock_init("linkdown suspend disabled");
	cdf_atomic_set(&sc->pm_state, HIF_PM_RUNTIME_STATE_NONE);
	INIT_LIST_HEAD(&sc->prevent_suspend_list);
}

/**
 * hif_pm_runtime_close(): close runtime pm
 * @sc: pci bus handle
 *
 * ensure runtime_pm is stopped before closing the driver
 */
static void hif_pm_runtime_close(struct hif_pci_softc *sc)
{
	if (cdf_atomic_read(&sc->pm_state) == HIF_PM_RUNTIME_STATE_NONE)
		return;
	else
		hif_pm_runtime_stop(sc);
}


#else

static void hif_pm_runtime_close(struct hif_pci_softc *sc) {}
static void hif_pm_runtime_open(struct hif_pci_softc *sc) {}
static void hif_pm_runtime_start(struct hif_pci_softc *sc) {}
static void hif_pm_runtime_stop(struct hif_pci_softc *sc) {}
#endif

/**
 * hif_enable_power_management(): enable power management
 * @hif_ctx: hif context
 *
 * Currently only does runtime pm.  Eventually this function could
 * consolidate other power state features such as only letting
 * the soc sleep after the driver finishes loading and re-enabling
 * aspm (hif_enable_power_gating).
 */
void hif_enable_power_management(void *hif_ctx)
{
	struct hif_pci_softc *pci_ctx;

	if (hif_ctx == NULL) {
		HIF_ERROR("%s, hif_ctx null", __func__);
		return;
	}

	pci_ctx = ((struct ol_softc *)hif_ctx)->hif_sc;

	hif_pm_runtime_start(pci_ctx);
}

/**
 * hif_disable_power_management(): disable power management
 * @hif_ctx: hif context
 *
 * Currently disables runtime pm. Should be updated to behave
 * if runtime pm is not started. Should be updated to take care
 * of aspm and soc sleep for driver load.
 */
void hif_disable_power_management(void *hif_ctx)
{
	struct hif_pci_softc *pci_ctx;

	if (hif_ctx == NULL) {
		HIF_ERROR("%s, hif_ctx null", __func__);
		return;
	}

	pci_ctx = ((struct ol_softc *)hif_ctx)->hif_sc;

	hif_pm_runtime_stop(pci_ctx);
}

#define ATH_PCI_PROBE_RETRY_MAX 3
/**
 * hif_bus_open(): hif_bus_open
 * @scn: scn
 * @bus_type: bus type
 *
 * Return: n/a
 */
CDF_STATUS hif_bus_open(struct ol_softc *ol_sc, enum ath_hal_bus_type bus_type)
{
	struct hif_pci_softc *sc;

	sc = cdf_mem_malloc(sizeof(*sc));
	if (!sc) {
		HIF_ERROR("%s: no mem", __func__);
		return CDF_STATUS_E_NOMEM;
	}
	ol_sc->hif_sc = (void *)sc;
	sc->ol_sc = ol_sc;
	ol_sc->bus_type = bus_type;
	hif_pm_runtime_open(sc);

	cdf_spinlock_init(&ol_sc->irq_lock);

	return CDF_STATUS_SUCCESS;
}

/**
 * hif_bus_close(): hif_bus_close
 *
 * Return: n/a
 */
void hif_bus_close(struct ol_softc *ol_sc)
{
	struct hif_pci_softc *sc;

	if (ol_sc == NULL) {
		HIF_ERROR("%s: ol_softc is NULL", __func__);
		return;
	}
	sc = ol_sc->hif_sc;
	if (sc == NULL)
		return;

	hif_pm_runtime_close(sc);
	cdf_mem_free(sc);
	ol_sc->hif_sc = NULL;
}

#define BAR_NUM 0

int hif_enable_pci(struct hif_pci_softc *sc,
		struct pci_dev *pdev,
		const struct pci_device_id *id)
{
	void __iomem *mem;
	int ret = 0;
	uint16_t device_id;
	struct ol_softc *ol_sc = sc->ol_sc;

	pci_read_config_word(pdev,PCI_DEVICE_ID,&device_id);
	if(device_id != id->device)  {
		HIF_ERROR(
		   "%s: dev id mismatch, config id = 0x%x, probing id = 0x%x",
		   __func__, device_id, id->device);
		/* pci link is down, so returing with error code */
		return -EIO;
	}

	/* FIXME: temp. commenting out assign_resource
	 * call for dev_attach to work on 2.6.38 kernel
	 */
#if (!defined(__LINUX_ARM_ARCH__))
	if (pci_assign_resource(pdev, BAR_NUM)) {
		HIF_ERROR("%s: pci_assign_resource error", __func__);
		return -EIO;
	}
#endif
	if (pci_enable_device(pdev)) {
		HIF_ERROR("%s: pci_enable_device error",
			   __func__);
		return -EIO;
	}

	/* Request MMIO resources */
	ret = pci_request_region(pdev, BAR_NUM, "ath");
	if (ret) {
		HIF_ERROR("%s: PCI MMIO reservation error", __func__);
		ret = -EIO;
		goto err_region;
	}
#ifdef CONFIG_ARM_LPAE
	/* if CONFIG_ARM_LPAE is enabled, we have to set 64 bits mask
	 * for 32 bits device also. */
	ret =  pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (ret) {
		HIF_ERROR("%s: Cannot enable 64-bit pci DMA", __func__);
		goto err_dma;
	}
	ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
	if (ret) {
		HIF_ERROR("%s: Cannot enable 64-bit DMA", __func__);
		goto err_dma;
	}
#else
	ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (ret) {
		HIF_ERROR("%s: Cannot enable 32-bit pci DMA", __func__);
		goto err_dma;
	}
	ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	if (ret) {
		HIF_ERROR("%s: Cannot enable 32-bit consistent DMA!",
			   __func__);
		goto err_dma;
	}
#endif

	PCI_CFG_TO_DISABLE_L1SS_STATES(pdev, 0x188);

	/* Set bus master bit in PCI_COMMAND to enable DMA */
	pci_set_master(pdev);

	/* Arrange for access to Target SoC registers. */
	mem = pci_iomap(pdev, BAR_NUM, 0);
	if (!mem) {
		HIF_ERROR("%s: PCI iomap error", __func__);
		ret = -EIO;
		goto err_iomap;
	}
	sc->mem = mem;
	sc->pdev = pdev;
	sc->dev = &pdev->dev;
	ol_sc->aps_osdev.bdev = pdev;
	ol_sc->aps_osdev.device = &pdev->dev;
	ol_sc->aps_osdev.bc.bc_handle = (void *)mem;
	ol_sc->aps_osdev.bc.bc_bustype = HAL_BUS_TYPE_PCI;
	sc->devid = id->device;
	sc->cacheline_sz = dma_get_cache_alignment();
	/* Get RAM dump memory address and size */
	GET_VIRT_RAMDUMP_MEM(ol_sc);
	ol_sc->mem = mem;
	sc->pci_enabled = true;
	return ret;

err_iomap:
	pci_clear_master(pdev);
err_dma:
	pci_release_region(pdev, BAR_NUM);
err_region:
	pci_disable_device(pdev);
	return ret;
}

void hif_disable_pci(struct hif_pci_softc *sc)
{
	struct ol_softc *ol_sc;

	if (!sc)
		return;

	ol_sc = sc->ol_sc;
	if (ol_sc == NULL) {
		HIF_ERROR("%s: ol_sc = NULL", __func__);
		return;
	}
	pci_set_drvdata(sc->pdev, NULL);
	hif_pci_device_reset(sc);
	pci_iounmap(sc->pdev, sc->mem);
	sc->mem = NULL;
	ol_sc->mem = NULL;
	pci_clear_master(sc->pdev);
	pci_release_region(sc->pdev, BAR_NUM);
	pci_disable_device(sc->pdev);
}

int hif_pci_probe_tgt_wakeup(struct hif_pci_softc *sc)
{
	int ret = 0;
	int targ_awake_limit = 500;
#ifndef QCA_WIFI_3_0
	uint32_t fw_indicator;
#endif
	struct ol_softc *scn = sc->ol_sc;
	/*
	 * Verify that the Target was started cleanly.*
	 * The case where this is most likely is with an AUX-powered
	 * Target and a Host in WoW mode. If the Host crashes,
	 * loses power, or is restarted (without unloading the driver)
	 * then the Target is left (aux) powered and running.  On a
	 * subsequent driver load, the Target is in an unexpected state.
	 * We try to catch that here in order to reset the Target and
	 * retry the probe.
	 */
	hif_write32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
				  PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_V_MASK);
	while (!hif_targ_is_awake(scn, sc->mem)) {
		if (0 == targ_awake_limit) {
			HIF_ERROR("%s: target awake timeout", __func__);
			ret = -EAGAIN;
			goto end;
		}
		cdf_mdelay(1);
		targ_awake_limit--;
	}

#if PCIE_BAR0_READY_CHECKING
	{
		int wait_limit = 200;
		/* Synchronization point: wait the BAR0 is configured */
		while (wait_limit-- &&
			   !(hif_read32_mb(sc->mem +
					  PCIE_LOCAL_BASE_ADDRESS +
					  PCIE_SOC_RDY_STATUS_ADDRESS) \
					  & PCIE_SOC_RDY_STATUS_BAR_MASK)) {
			cdf_mdelay(10);
		}
		if (wait_limit < 0) {
			/* AR6320v1 doesn't support checking of BAR0 configuration,
				 takes one sec to wait BAR0 ready */
			HIF_INFO_MED("%s: AR6320v1 waits two sec for BAR0",
				    __func__);
		}
    }
#endif

#ifndef QCA_WIFI_3_0
	fw_indicator = hif_read32_mb(sc->mem + FW_INDICATOR_ADDRESS);
	hif_write32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
				  PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_RESET);

	if (fw_indicator & FW_IND_INITIALIZED) {
		HIF_ERROR("%s: Target is in an unknown state. EAGAIN",
			   __func__);
		ret = -EAGAIN;
		goto end;
	}
#endif

end:
	return ret;
}

static void wlan_tasklet_msi(unsigned long data)
{
	struct hif_tasklet_entry *entry = (struct hif_tasklet_entry *)data;
	struct hif_pci_softc *sc = (struct hif_pci_softc *) entry->hif_handler;
	struct ol_softc *scn = sc->ol_sc;

	if (sc->ol_sc->hif_init_done == false)
		goto irq_handled;

	if (cdf_atomic_read(&sc->ol_sc->link_suspended))
		goto irq_handled;

	cdf_atomic_inc(&scn->active_tasklet_cnt);

	if (entry->id == HIF_MAX_TASKLET_NUM) {
		/* the last tasklet is for fw IRQ */
		(irqreturn_t)hif_fw_interrupt_handler(sc->irq_event, sc->ol_sc);
		if (sc->ol_sc->target_status == OL_TRGET_STATUS_RESET)
			goto irq_handled;
	} else if (entry->id < sc->ol_sc->ce_count) {
		ce_per_engine_service(sc->ol_sc, entry->id);
	} else {
		HIF_ERROR("%s: ERROR - invalid CE_id = %d",
		       __func__, entry->id);
	}
	return;

irq_handled:
	cdf_atomic_dec(&scn->active_tasklet_cnt);

}

int hif_configure_msi(struct hif_pci_softc *sc)
{
	int ret = 0;
	int num_msi_desired;
	int rv = -1;
	struct ol_softc *scn = sc->ol_sc;

	HIF_TRACE("%s: E", __func__);

	num_msi_desired = MSI_NUM_REQUEST; /* Multiple MSI */
	if (num_msi_desired < 1) {
		HIF_ERROR("%s: MSI is not configured", __func__);
		return -EINVAL;
	}

	if (num_msi_desired > 1) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
		rv = pci_enable_msi_range(sc->pdev, num_msi_desired,
						num_msi_desired);
#else
		rv = pci_enable_msi_block(sc->pdev, num_msi_desired);
#endif
	}
	HIF_TRACE("%s: num_msi_desired = %d, available_msi = %d",
		  __func__, num_msi_desired, rv);

	if (rv == 0 || rv >= HIF_MAX_TASKLET_NUM) {
		int i;

		sc->num_msi_intrs = HIF_MAX_TASKLET_NUM;
		sc->tasklet_entries[HIF_MAX_TASKLET_NUM -1].hif_handler =
			(void *)sc;
		sc->tasklet_entries[HIF_MAX_TASKLET_NUM -1].id =
			HIF_MAX_TASKLET_NUM;
		tasklet_init(&sc->intr_tq, wlan_tasklet_msi,
			 (unsigned long)&sc->tasklet_entries[
			 HIF_MAX_TASKLET_NUM -1]);
		ret = request_irq(sc->pdev->irq + MSI_ASSIGN_FW,
				  hif_pci_msi_fw_handler,
				  IRQF_SHARED, "wlan_pci", sc);
		if(ret) {
			HIF_ERROR("%s: request_irq failed", __func__);
			goto err_intr;
		}
		for (i = 0; i <= scn->ce_count; i++) {
			sc->tasklet_entries[i].hif_handler = (void *)sc;
			sc->tasklet_entries[i].id = i;
			tasklet_init(&sc->intr_tq, wlan_tasklet_msi,
				 (unsigned long)&sc->tasklet_entries[i]);
			ret = request_irq((sc->pdev->irq +
					   i + MSI_ASSIGN_CE_INITIAL),
					  ce_per_engine_handler, IRQF_SHARED,
					  "wlan_pci", sc);
			if(ret) {
				HIF_ERROR("%s: request_irq failed", __func__);
				goto err_intr;
			}
		}
	} else if (rv > 0) {
		HIF_TRACE("%s: use single msi", __func__);

		if ((ret = pci_enable_msi(sc->pdev)) < 0) {
			HIF_ERROR("%s: single MSI allocation failed",
				  __func__);
			/* Try for legacy PCI line interrupts */
			sc->num_msi_intrs = 0;
		} else {
			sc->num_msi_intrs = 1;
			tasklet_init(&sc->intr_tq,
				wlan_tasklet, (unsigned long)sc);
			ret = request_irq(sc->pdev->irq,
					  hif_pci_interrupt_handler,
					  IRQF_SHARED, "wlan_pci", sc);
			if(ret) {
				HIF_ERROR("%s: request_irq failed", __func__);
				goto err_intr;
			}
		}
	} else {
		sc->num_msi_intrs = 0;
		ret = -EIO;
		HIF_ERROR("%s: do not support MSI, rv = %d", __func__, rv);
	}
	if ((ret = pci_enable_msi(sc->pdev)) < 0) {
		HIF_ERROR("%s: single MSI interrupt allocation failed",
			  __func__);
		/* Try for legacy PCI line interrupts */
		sc->num_msi_intrs = 0;
	} else {
		sc->num_msi_intrs = 1;
		tasklet_init(&sc->intr_tq, wlan_tasklet, (unsigned long)sc);
		ret = request_irq(sc->pdev->irq,
				  hif_pci_interrupt_handler, IRQF_SHARED,
				  "wlan_pci", sc);
		if(ret) {
			HIF_ERROR("%s: request_irq failed", __func__);
			goto err_intr;
		}
	}

	if (ret == 0) {
		hif_write32_mb(sc->mem+(SOC_CORE_BASE_ADDRESS |
			  PCIE_INTR_ENABLE_ADDRESS),
			  HOST_GROUP0_MASK);
		hif_write32_mb(sc->mem +
			  PCIE_LOCAL_BASE_ADDRESS + PCIE_SOC_WAKE_ADDRESS,
			  PCIE_SOC_WAKE_RESET);
	}
	HIF_TRACE("%s: X, ret = %d", __func__, ret);

	return ret;

err_intr:
if (sc->num_msi_intrs >= 1)
		pci_disable_msi(sc->pdev);
	return ret;
}

static int hif_pci_configure_legacy_irq(struct hif_pci_softc *sc)
{
	int ret = 0;
	struct ol_softc *scn = sc->ol_sc;

	HIF_TRACE("%s: E", __func__);

	/* do notn support MSI or MSI IRQ failed */
	tasklet_init(&sc->intr_tq, wlan_tasklet, (unsigned long)sc);
	ret = request_irq(sc->pdev->irq,
			  hif_pci_interrupt_handler, IRQF_SHARED,
			  "wlan_pci", sc);
	if(ret) {
		HIF_ERROR("%s: request_irq failed, ret = %d", __func__, ret);
		goto end;
	}
	/* Use Legacy PCI Interrupts */
	hif_write32_mb(sc->mem+(SOC_CORE_BASE_ADDRESS |
		  PCIE_INTR_ENABLE_ADDRESS),
		  HOST_GROUP0_MASK);
	hif_write32_mb(sc->mem + PCIE_LOCAL_BASE_ADDRESS +
		      PCIE_SOC_WAKE_ADDRESS,
		  PCIE_SOC_WAKE_RESET);
end:
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_ERROR,
			  "%s: X, ret = %d", __func__, ret);
	return ret;
}

/**
 * hif_nointrs(): disable IRQ
 *
 * This function stops interrupt(s)
 *
 * @scn: struct ol_softc
 *
 * Return: none
 */
void hif_nointrs(struct ol_softc *scn)
{
	int i;
	struct hif_pci_softc *sc = scn->hif_sc;

	if (scn->request_irq_done == false)
		return;
	if (sc->num_msi_intrs > 0) {
		/* MSI interrupt(s) */
		for (i = 0; i < sc->num_msi_intrs; i++) {
			free_irq(sc->pdev->irq + i, sc);
		}
		sc->num_msi_intrs = 0;
	} else {
		/* Legacy PCI line interrupt */
		free_irq(sc->pdev->irq, sc);
	}
	ce_unregister_irq(scn->hif_hdl, 0xfff);
	scn->request_irq_done = false;
}

/**
 * hif_disable_bus(): hif_disable_bus
 *
 * This function disables the bus
 *
 * @bdev: bus dev
 *
 * Return: none
 */
void hif_disable_bus(void *bdev)
{
	struct pci_dev *pdev = bdev;
	struct hif_pci_softc *sc = pci_get_drvdata(pdev);
	struct ol_softc *scn;
	void __iomem *mem;

	/* Attach did not succeed, all resources have been
	 * freed in error handler
	 */
	if (!sc)
		return;

	scn = sc->ol_sc;

	if (ADRASTEA_BU) {
		hif_write32_mb(sc->mem + PCIE_INTR_ENABLE_ADDRESS, 0);
		hif_write32_mb(sc->mem + PCIE_INTR_CLR_ADDRESS,
			       HOST_GROUP0_MASK);
	}

	mem = (void __iomem *)sc->mem;
	if (mem) {
		pci_disable_msi(pdev);
		hif_dump_pipe_debug_count(scn);
		hif_deinit_cdf_ctx(scn);
		if (scn->athdiag_procfs_inited) {
			athdiag_procfs_remove();
			scn->athdiag_procfs_inited = false;
		}
		pci_set_drvdata(pdev, NULL);
		pci_iounmap(pdev, mem);
		scn->mem = NULL;
		pci_release_region(pdev, BAR_NUM);
		pci_clear_master(pdev);
		pci_disable_device(pdev);
	}
	HIF_INFO("%s: X", __func__);
}

#define OL_ATH_PCI_PM_CONTROL 0x44

#ifdef CONFIG_CNSS

#ifdef RUNTIME_PM
/**
 * hif_runtime_prevent_linkdown() - prevent or allow a runtime pm from occuring
 * @scn: hif context
 * @flag: prevent linkdown if true otherwise allow
 *
 * this api should only be called as part of bus prevent linkdown
 */
static void hif_runtime_prevent_linkdown(struct ol_softc *scn, bool flag)
{
	struct hif_pci_softc *sc = scn->hif_sc;

	if (flag)
		hif_pm_runtime_prevent_suspend(scn, sc->prevent_linkdown_lock);
	else
		hif_pm_runtime_allow_suspend(scn, sc->prevent_linkdown_lock);
}
#else
static void hif_runtime_prevent_linkdown(struct ol_softc *scn, bool flag)
{
}
#endif

/**
 * hif_bus_prevent_linkdown(): allow or permit linkdown
 * @flag: true prevents linkdown, false allows
 *
 * Calls into the platform driver to vote against taking down the
 * pcie link.
 *
 * Return: n/a
 */
void hif_bus_prevent_linkdown(struct ol_softc *scn, bool flag)
{
	HIF_ERROR("wlan: %s pcie power collapse",
			(flag ? "disable" : "enable"));
	hif_runtime_prevent_linkdown(scn, flag);
	cnss_wlan_pm_control(flag);
}
#endif

/**
 * hif_drain_tasklets(): wait untill no tasklet is pending
 * @scn: hif context
 *
 * Let running tasklets clear pending trafic.
 *
 * Return: 0 if no bottom half is in progress when it returns.
 *   -EFAULT if it times out.
 */
static inline int hif_drain_tasklets(struct ol_softc *scn)
{
	uint32_t ce_drain_wait_cnt = 0;

	while (cdf_atomic_read(&scn->active_tasklet_cnt)) {
		if (++ce_drain_wait_cnt > HIF_CE_DRAIN_WAIT_CNT) {
			HIF_ERROR("%s: CE still not done with access",
			       __func__);

			return -EFAULT;
		}
		HIF_INFO("%s: Waiting for CE to finish access", __func__);
		msleep(10);
	}
	return 0;
}

/**
 * hif_bus_suspend_link_up() - suspend the bus
 *
 * Configures the pci irq line as a wakeup source.
 *
 * Return: 0 for success and non-zero for failure
 */
static int hif_bus_suspend_link_up(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct pci_dev *pdev;
	int status;

	if (!scn)
		return -EFAULT;

	pdev = scn->aps_osdev.bdev;

	status = hif_drain_tasklets(scn);
	if (status != 0)
		return status;

	if (unlikely(enable_irq_wake(pdev->irq))) {
		HIF_ERROR("%s: Fail to enable wake IRQ!", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * hif_bus_resume_link_up() - hif bus resume API
 *
 * This function disables the wakeup source.
 *
 * Return: 0 for success and non-zero for failure
 */
static int hif_bus_resume_link_up(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct pci_dev *pdev;

	if (!scn)
		return -EFAULT;

	pdev = scn->aps_osdev.bdev;
	if (!pdev) {
		HIF_ERROR("%s: pci_dev is null", __func__);
		return -EFAULT;
	}

	if (unlikely(disable_irq_wake(pdev->irq))) {
		HIF_ERROR("%s: Fail to disable wake IRQ!", __func__);
		return -EFAULT;
	}

	return 0;
}

/**
 * hif_bus_suspend_link_down() - suspend the bus
 *
 * Suspends the hif layer taking care of draining recieve queues and
 * shutting down copy engines if needed. Ensures opy engine interrupts
 * are disabled when it returns.  Prevents register access after it
 * returns.
 *
 * Return: 0 for success and non-zero for failure
 */
static int hif_bus_suspend_link_down(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct pci_dev *pdev;
	struct HIF_CE_state *hif_state;
	int status = 0;

	if (!scn)
		return -EFAULT;

	pdev = scn->aps_osdev.bdev;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	if (!hif_state) {
		HIF_ERROR("%s: hif_state is null", __func__);
		return -EFAULT;
	}

	disable_irq(pdev->irq);

	status = hif_drain_tasklets(scn);
	if (status != 0) {
		enable_irq(pdev->irq);
		return status;
	}

	/* Stop the HIF Sleep Timer */
	hif_cancel_deferred_target_sleep(scn);

	cdf_atomic_set(&scn->link_suspended, 1);

	return 0;
}

/**
 * hif_bus_resume_link_down() - hif bus resume API
 *
 * This function resumes the bus reenabling interupts.
 *
 * Return: 0 for success and non-zero for failure
 */
static int hif_bus_resume_link_down(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct pci_dev *pdev;

	if (!scn)
		return -EFAULT;

	pdev = scn->aps_osdev.bdev;
	if (!pdev) {
		HIF_ERROR("%s: pci_dev is null", __func__);
		return -EFAULT;
	}

	cdf_atomic_set(&scn->link_suspended, 0);

	enable_irq(pdev->irq);

	return 0;
}

/**
 * hif_bus_suspend(): prepare hif for suspend
 *
 * chose suspend type based on link suspend voting.
 *
 * Return: 0 for success and non-zero error code for failure
 */
int hif_bus_suspend(void)
{
	if (hif_can_suspend_link())
		return hif_bus_suspend_link_down();
	else
		return hif_bus_suspend_link_up();
}

/**
 * hif_bus_resume(): prepare hif for resume
 *
 * chose suspend type based on link suspend voting.
 *
 * Return: 0 for success and non-zero error code for failure
 */
int hif_bus_resume(void)
{
	if (hif_can_suspend_link())
		return hif_bus_resume_link_down();
	else
		return hif_bus_resume_link_up();
}

#ifdef FEATURE_RUNTIME_PM
/**
 * __hif_runtime_pm_set_state(): utility function
 * @state: state to set
 *
 * indexes into the runtime pm state and sets it.
 */
static void __hif_runtime_pm_set_state(enum hif_pm_runtime_state state)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct hif_pci_softc *sc;

	if (NULL == scn) {
		HIF_ERROR("%s: HIF_CTX not initialized",
		       __func__);
		return;
	}

	sc = scn->hif_sc;
	cdf_atomic_set(&sc->pm_state, state);

}
#endif

#ifdef FEATURE_RUNTIME_PM
/**
 * hif_runtime_pm_set_state_inprogress(): adjust runtime pm state
 *
 * Notify hif that a runtime pm opperation has started
 */
static void hif_runtime_pm_set_state_inprogress(void)
{
	__hif_runtime_pm_set_state(HIF_PM_RUNTIME_STATE_INPROGRESS);
}

/**
 * hif_runtime_pm_set_state_on():  adjust runtime pm state
 *
 * Notify hif that a the runtime pm state should be on
 */
static void hif_runtime_pm_set_state_on(void)
{
	__hif_runtime_pm_set_state(HIF_PM_RUNTIME_STATE_ON);
}

/**
 * hif_runtime_pm_set_state_suspended():  adjust runtime pm state
 *
 * Notify hif that a runtime suspend attempt has been completed successfully
 */
static void hif_runtime_pm_set_state_suspended(void)
{
	__hif_runtime_pm_set_state(HIF_PM_RUNTIME_STATE_SUSPENDED);
}

static inline struct hif_pci_softc *get_sc(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);

	if (NULL == scn) {
		HIF_ERROR("%s: Could not disable ASPM scn is null",
		       __func__);
		return NULL;
	}

	return scn->hif_sc;
}

/**
 * hif_log_runtime_suspend_success() - log a successful runtime suspend
 */
static void hif_log_runtime_suspend_success(void)
{
	struct hif_pci_softc *sc = get_sc();
	if (sc == NULL)
		return;

	sc->pm_stats.suspended++;
	sc->pm_stats.suspend_jiffies = jiffies;
}

/**
 * hif_log_runtime_suspend_failure() - log a failed runtime suspend
 *
 * log a failed runtime suspend
 * mark last busy to prevent immediate runtime suspend
 */
static void hif_log_runtime_suspend_failure(void)
{
	struct hif_pci_softc *sc = get_sc();
	if (sc == NULL)
		return;

	sc->pm_stats.suspend_err++;
}

/**
 * hif_log_runtime_resume_success() - log a successful runtime resume
 *
 * log a successfull runtime resume
 * mark last busy to prevent immediate runtime suspend
 */
static void hif_log_runtime_resume_success(void)
{
	struct hif_pci_softc *sc = get_sc();
	if (sc == NULL)
		return;

	sc->pm_stats.resumed++;
}

/**
 * hif_process_runtime_suspend_failure() - bookkeeping of suspend failure
 *
 * Record the failure.
 * mark last busy to delay a retry.
 * adjust the runtime_pm state.
 */
void hif_process_runtime_suspend_failure(void)
{
	struct hif_pci_softc *sc = get_sc();

	hif_log_runtime_suspend_failure();
	if (sc != NULL)
		hif_pm_runtime_mark_last_busy(sc->dev);
	hif_runtime_pm_set_state_on();
}

/**
 * hif_pre_runtime_suspend() - bookkeeping before beginning runtime suspend
 *
 * Makes sure that the pci link will be taken down by the suspend opperation.
 * If the hif layer is configured to leave the bus on, runtime suspend will
 * not save any power.
 *
 * Set the runtime suspend state to in progress.
 *
 * return -EINVAL if the bus won't go down.  otherwise return 0
 */
int hif_pre_runtime_suspend(void)
{
	if (!hif_can_suspend_link()) {
		HIF_ERROR("Runtime PM not supported for link up suspend");
		return -EINVAL;
	}

	hif_runtime_pm_set_state_inprogress();
	return 0;
}

/**
 * hif_process_runtime_suspend_success() - bookkeeping of suspend success
 *
 * Record the success.
 * adjust the runtime_pm state
 */
void hif_process_runtime_suspend_success(void)
{
	hif_runtime_pm_set_state_suspended();
	hif_log_runtime_suspend_success();
}

/**
 * hif_pre_runtime_resume() - bookkeeping before beginning runtime resume
 *
 * update the runtime pm state.
 */
void hif_pre_runtime_resume(void)
{
	hif_runtime_pm_set_state_inprogress();
}

/**
 * hif_process_runtime_resume_success() - bookkeeping after a runtime resume
 *
 * record the success.
 * adjust the runtime_pm state
 */
void hif_process_runtime_resume_success(void)
{
	struct hif_pci_softc *sc = get_sc();

	hif_log_runtime_resume_success();
	if (sc != NULL)
		hif_pm_runtime_mark_last_busy(sc->dev);
	hif_runtime_pm_set_state_on();
}
#endif

/**
 * hif_runtime_suspend() - do the bus suspend part of a runtime suspend
 *
 * Return: 0 for success and non-zero error code for failure
 */
int hif_runtime_suspend(void)
{
	return hif_bus_suspend();
}

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hif_fastpath_resume() - resume fastpath for runtimepm
 *
 * ensure that the fastpath write index register is up to date
 * since runtime pm may cause ce_send_fast to skip the register
 * write.
 */
static void hif_fastpath_resume(void)
{
	struct ol_softc *scn =
		(struct ol_softc *)cds_get_context(CDF_MODULE_ID_HIF);
	struct CE_state *ce_state;

	if (!scn)
		return;

	if (scn->fastpath_mode_on) {
		if (Q_TARGET_ACCESS_BEGIN(scn)) {
			ce_state = scn->ce_id_to_state[CE_HTT_H2T_MSG];
			cdf_spin_lock_bh(&ce_state->ce_index_lock);

			/*war_ce_src_ring_write_idx_set */
			CE_SRC_RING_WRITE_IDX_SET(scn, ce_state->ctrl_addr,
					ce_state->src_ring->write_index);
			cdf_spin_unlock_bh(&ce_state->ce_index_lock);
			Q_TARGET_ACCESS_END(scn);
		}
	}
}
#else
static void hif_fastpath_resume(void) {}
#endif


/**
 * hif_runtime_resume() - do the bus resume part of a runtime resume
 *
 *  Return: 0 for success and non-zero error code for failure
 */
int hif_runtime_resume(void)
{
	int status = hif_bus_resume();

	hif_fastpath_resume();

	return status;
}

void hif_disable_isr(void *ol_sc)
{
	struct ol_softc *scn = (struct ol_softc *)ol_sc;
	struct hif_pci_softc *sc = scn->hif_sc;

	hif_nointrs(ol_sc);
#if CONFIG_PCIE_64BIT_MSI
	OS_FREE_CONSISTENT(&scn->aps_osdev, 4,
			   scn->msi_magic, scn->msi_magic_dma,
			   OS_GET_DMA_MEM_CONTEXT(scn, MSI_dmacontext));
	scn->msi_magic = NULL;
	scn->msi_magic_dma = 0;
#endif
	/* Cancel the pending tasklet */
	ce_tasklet_kill(scn->hif_hdl);
	tasklet_kill(&sc->intr_tq);
	cdf_atomic_set(&scn->active_tasklet_cnt, 0);
}

/* Function to reset SoC */
void hif_reset_soc(void *ol_sc)
{
	struct ol_softc *scn = (struct ol_softc *)ol_sc;
	struct hif_pci_softc *sc = scn->hif_sc;

#if defined(CPU_WARM_RESET_WAR)
	/* Currently CPU warm reset sequence is tested only for AR9888_REV2
	 * Need to enable for AR9888_REV1 once CPU warm reset sequence is
	 * verified for AR9888_REV1
	 */
	if (scn->target_version == AR9888_REV2_VERSION) {
		hif_pci_device_warm_reset(sc);
	} else {
		hif_pci_device_reset(sc);
	}
#else
	hif_pci_device_reset(sc);
#endif
}

void hif_disable_aspm(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct hif_pci_softc *sc;

	if (NULL == scn) {
		HIF_ERROR("%s: Could not disable ASPM scn is null",
		       __func__);
		return;
	}

	sc = scn->hif_sc;

	/* Disable ASPM when pkt log is enabled */
	pci_read_config_dword(sc->pdev, 0x80, &scn->lcr_val);
	pci_write_config_dword(sc->pdev, 0x80, (scn->lcr_val & 0xffffff00));
}

/**
 * hif_enable_power_gating(): enable HW power gating
 *
 * This function enables HW gating
 *
 * Return: none
 */
void hif_enable_power_gating(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;
	struct hif_pci_softc *sc;

	if (NULL == scn) {
		HIF_ERROR("%s: Could not disable ASPM scn is null",
		       __func__);
		return;
	}
	sc = scn->hif_sc;

	/* Re-enable ASPM after firmware/OTP download is complete */
	pci_write_config_dword(sc->pdev, 0x80, scn->lcr_val);
	if (scn->pkt_log_init == false) {
		PKT_LOG_MOD_INIT(scn);
		scn->pkt_log_init = true;
	}
}

#ifdef CONFIG_PCI_MSM
static inline void hif_msm_pcie_debug_info(struct hif_pci_softc *sc)
{
	msm_pcie_debug_info(sc->pdev, 13, 1, 0, 0, 0);
	msm_pcie_debug_info(sc->pdev, 13, 2, 0, 0, 0);
}
#else
static inline void hif_msm_pcie_debug_info(struct hif_pci_softc *sc) {};
#endif

/*
 * For now, we use simple on-demand sleep/wake.
 * Some possible improvements:
 *  -Use the Host-destined A_INUM_PCIE_AWAKE interrupt rather than spin/delay
 *   (or perhaps spin/delay for a short while, then convert to sleep/interrupt)
 *   Careful, though, these functions may be used by
 *  interrupt handlers ("atomic")
 *  -Don't use host_reg_table for this code; instead use values directly
 *  -Use a separate timer to track activity and allow Target to sleep only
 *   if it hasn't done anything for a while; may even want to delay some
 *   processing for a short while in order to "batch" (e.g.) transmit
 *   requests with completion processing into "windows of up time".  Costs
 *   some performance, but improves power utilization.
 *  -On some platforms, it might be possible to eliminate explicit
 *   sleep/wakeup. Instead, take a chance that each access works OK. If not,
 *   recover from the failure by forcing the Target awake.
 *  -Change keep_awake_count to an atomic_t in order to avoid spin lock
 *   overhead in some cases. Perhaps this makes more sense when
 *   CONFIG_ATH_PCIE_ACCESS_LIKELY is used and less sense when LIKELY is
 *   disabled.
 *  -It is possible to compile this code out and simply force the Target
 *   to remain awake.  That would yield optimal performance at the cost of
 *   increased power. See CONFIG_ATH_PCIE_MAX_PERF.
 *
 * Note: parameter wait_for_it has meaning only when waking (when sleep_ok==0).
 */
/**
 * hif_target_sleep_state_adjust() - on-demand sleep/wake
 * @scn: ol_softc pointer.
 * @sleep_ok: bool
 * @wait_for_it: bool
 *
 * Output the pipe error counts of each pipe to log file
 *
 * Return: int
 */
#if ((CONFIG_ATH_PCIE_MAX_PERF == 0) && CONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD)
int
hif_target_sleep_state_adjust(struct ol_softc *scn,
			      bool sleep_ok, bool wait_for_it)
{
	struct HIF_CE_state *hif_state = scn->hif_hdl;
	A_target_id_t pci_addr = scn->mem;
	static int max_delay;
	struct hif_pci_softc *sc = scn->hif_sc;
	static int debug;

	if (scn->recovery)
		return -EACCES;

	if (cdf_atomic_read(&scn->link_suspended)) {
		HIF_ERROR("%s:invalid access, PCIe link is down", __func__);
		debug = true;
		CDF_ASSERT(0);
		return -EACCES;
	}

	if (debug) {
		wait_for_it = true;
		HIF_ERROR("%s: doing debug for invalid access, PCIe link is suspended",
				__func__);
		CDF_ASSERT(0);
	}

	if (sleep_ok) {
		cdf_spin_lock_irqsave(&hif_state->keep_awake_lock);
		hif_state->keep_awake_count--;
		if (hif_state->keep_awake_count == 0) {
			/* Allow sleep */
			hif_state->verified_awake = false;
			hif_state->sleep_ticks = cdf_system_ticks();
		}
		if (hif_state->fake_sleep == false) {
			/* Set the Fake Sleep */
			hif_state->fake_sleep = true;

			/* Start the Sleep Timer */
			cdf_softirq_timer_cancel(&hif_state->sleep_timer);
			cdf_softirq_timer_start(&hif_state->sleep_timer,
				HIF_SLEEP_INACTIVITY_TIMER_PERIOD_MS);
		}
		cdf_spin_unlock_irqrestore(&hif_state->keep_awake_lock);
	} else {
		cdf_spin_lock_irqsave(&hif_state->keep_awake_lock);

		if (hif_state->fake_sleep) {
			hif_state->verified_awake = true;
		} else {
			if (hif_state->keep_awake_count == 0) {
				/* Force AWAKE */
				hif_write32_mb(pci_addr +
					      PCIE_LOCAL_BASE_ADDRESS +
					      PCIE_SOC_WAKE_ADDRESS,
					      PCIE_SOC_WAKE_V_MASK);
			}
		}
		hif_state->keep_awake_count++;
		cdf_spin_unlock_irqrestore(&hif_state->keep_awake_lock);

		if (wait_for_it && !hif_state->verified_awake) {
#define PCIE_SLEEP_ADJUST_TIMEOUT 8000  /* 8Ms */
			int tot_delay = 0;
			int curr_delay = 5;

			for (;; ) {
				if (hif_targ_is_awake(scn, pci_addr)) {
					hif_state->verified_awake = true;
					break;
				} else
				if (!hif_pci_targ_is_present
					    (scn, pci_addr)) {
					break;
				}
				if (tot_delay > PCIE_SLEEP_ADJUST_TIMEOUT) {
					uint16_t val;
					uint32_t bar;

					HIF_ERROR("%s: keep_awake_count = %d",
					       __func__,
					       hif_state->keep_awake_count);

					pci_read_config_word(sc->pdev,
							     PCI_VENDOR_ID,
							     &val);
					HIF_ERROR("%s: PCI Vendor ID = 0x%04x",
					       __func__, val);

					pci_read_config_word(sc->pdev,
							     PCI_DEVICE_ID,
							     &val);
					HIF_ERROR("%s: PCI Device ID = 0x%04x",
					       __func__, val);

					pci_read_config_word(sc->pdev,
							     PCI_COMMAND, &val);
					HIF_ERROR("%s: PCI Command = 0x%04x",
					       __func__, val);

					pci_read_config_word(sc->pdev,
							     PCI_STATUS, &val);
					HIF_ERROR("%s: PCI Status = 0x%04x",
					       __func__, val);

					pci_read_config_dword(sc->pdev,
						PCI_BASE_ADDRESS_0, &bar);
					HIF_ERROR("%s: PCI BAR 0 = 0x%08x",
					       __func__, bar);

					HIF_ERROR("%s: SOC_WAKE_ADDR 0%08x",
						__func__,
						hif_read32_mb(pci_addr +
						PCIE_LOCAL_BASE_ADDRESS
						+ PCIE_SOC_WAKE_ADDRESS));
					HIF_ERROR("%s: RTC_STATE_ADDR 0x%08x",
						__func__,
						hif_read32_mb(pci_addr +
						PCIE_LOCAL_BASE_ADDRESS
						+ RTC_STATE_ADDRESS));

					HIF_ERROR("%s:error, wakeup target",
						__func__);
					hif_msm_pcie_debug_info(sc);
					if (!sc->ol_sc->enable_self_recovery)
						CDF_BUG(0);
					scn->recovery = true;
					cds_set_recovery_in_progress(true);
					cnss_wlan_pci_link_down();
					return -EACCES;
				}

				OS_DELAY(curr_delay);
				tot_delay += curr_delay;

				if (curr_delay < 50)
					curr_delay += 5;
			}

			/*
			 * NB: If Target has to come out of Deep Sleep,
			 * this may take a few Msecs. Typically, though
			 * this delay should be <30us.
			 */
			if (tot_delay > max_delay)
				max_delay = tot_delay;
		}
	}

	if (debug && hif_state->verified_awake) {
		debug = 0;
		HIF_ERROR("%s: INTR_ENABLE_REG = 0x%08x, INTR_CAUSE_REG = 0x%08x, CPU_INTR_REG = 0x%08x, INTR_CLR_REG = 0x%08x, CE_INTERRUPT_SUMMARY_REG = 0x%08x",
			__func__,
			hif_read32_mb(sc->mem + SOC_CORE_BASE_ADDRESS +
				PCIE_INTR_ENABLE_ADDRESS),
			hif_read32_mb(sc->mem + SOC_CORE_BASE_ADDRESS +
				PCIE_INTR_CAUSE_ADDRESS),
			hif_read32_mb(sc->mem + SOC_CORE_BASE_ADDRESS +
				CPU_INTR_ADDRESS),
			hif_read32_mb(sc->mem + SOC_CORE_BASE_ADDRESS +
				PCIE_INTR_CLR_ADDRESS),
			hif_read32_mb(sc->mem + CE_WRAPPER_BASE_ADDRESS +
				CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS));
	}

	return 0;
}
#else
inline int
hif_target_sleep_state_adjust(struct ol_softc *scn,
			      bool sleep_ok, bool wait_for_it)
{
	return 0;
}
#endif

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
uint32_t hif_target_read_checked(struct ol_softc *scn, uint32_t offset)
{
	uint32_t value;
	void *addr;

	if (!A_TARGET_ACCESS_OK(scn))
		hi_fdebug();

	addr = scn->mem + offset;
	value = A_PCI_READ32(addr);

	{
		unsigned long irq_flags;
		int idx = pcie_access_log_seqnum % PCIE_ACCESS_LOG_NUM;

		spin_lock_irqsave(&pcie_access_log_lock, irq_flags);
		pcie_access_log[idx].seqnum = pcie_access_log_seqnum;
		pcie_access_log[idx].is_write = false;
		pcie_access_log[idx].addr = addr;
		pcie_access_log[idx].value = value;
		pcie_access_log_seqnum++;
		spin_unlock_irqrestore(&pcie_access_log_lock, irq_flags);
	}

	return value;
}

void
hif_target_write_checked(struct ol_softc *scn, uint32_t offset, uint32_t value)
{
	void *addr;

	if (!A_TARGET_ACCESS_OK(scn))
		hi_fdebug();

	addr = scn->mem + (offset);
	hif_write32_mb(addr, value);

	{
		unsigned long irq_flags;
		int idx = pcie_access_log_seqnum % PCIE_ACCESS_LOG_NUM;

		spin_lock_irqsave(&pcie_access_log_lock, irq_flags);
		pcie_access_log[idx].seqnum = pcie_access_log_seqnum;
		pcie_access_log[idx].is_write = true;
		pcie_access_log[idx].addr = addr;
		pcie_access_log[idx].value = value;
		pcie_access_log_seqnum++;
		spin_unlock_irqrestore(&pcie_access_log_lock, irq_flags);
	}
}

/**
 * hi_fdebug() - not needed in PCI
 *
 *
 * Return: n/a
 */
void hi_fdebug(void)
{
	/* BUG_ON(1); */
}

/**
 * hif_target_dump_access_log() - dump access log
 *
 * dump access log
 *
 * Return: n/a
 */
void hif_target_dump_access_log(void)
{
	int idx, len, start_idx, cur_idx;
	unsigned long irq_flags;

	spin_lock_irqsave(&pcie_access_log_lock, irq_flags);
	if (pcie_access_log_seqnum > PCIE_ACCESS_LOG_NUM) {
		len = PCIE_ACCESS_LOG_NUM;
		start_idx = pcie_access_log_seqnum % PCIE_ACCESS_LOG_NUM;
	} else {
		len = pcie_access_log_seqnum;
		start_idx = 0;
	}

	for (idx = 0; idx < len; idx++) {
		cur_idx = (start_idx + idx) % PCIE_ACCESS_LOG_NUM;
		HIF_ERROR("%s: idx:%d sn:%u wr:%d addr:%p val:%u.",
		       __func__, idx,
		       pcie_access_log[cur_idx].seqnum,
		       pcie_access_log[cur_idx].is_write,
		       pcie_access_log[cur_idx].addr,
		       pcie_access_log[cur_idx].value);
	}

	pcie_access_log_seqnum = 0;
	spin_unlock_irqrestore(&pcie_access_log_lock, irq_flags);
}
#endif

/**
 * war_pci_write32() - PCIe io32 write workaround
 * @addr: addr
 * @offset: offset
 * @value: value
 *
 * iowrite32
 *
 * Return: int
 */
void war_pci_write32(char *addr, uint32_t offset, uint32_t value)
{
	if (hif_pci_war1) {
		unsigned long irq_flags;

		spin_lock_irqsave(&pciwar_lock, irq_flags);

		(void)ioread32((void __iomem *)(addr + offset + 4));
		(void)ioread32((void __iomem *)(addr + offset + 4));
		(void)ioread32((void __iomem *)(addr + offset + 4));
		iowrite32((uint32_t) (value), (void __iomem *)(addr + offset));

		spin_unlock_irqrestore(&pciwar_lock, irq_flags);
	} else {
		iowrite32((uint32_t) (value), (void __iomem *)(addr + offset));
	}
}

/**
 * hif_configure_irq(): configure interrupt
 *
 * This function configures interrupt(s)
 *
 * @sc: PCIe control struct
 * @hif_hdl: struct HIF_CE_state
 *
 * Return: 0 - for success
 */
int hif_configure_irq(struct hif_pci_softc *sc)
{
	int ret = 0;
	struct ol_softc *scn = sc->ol_sc;

	HIF_TRACE("%s: E", __func__);

	if (ENABLE_MSI) {
		ret = hif_configure_msi(sc);
		if (ret == 0)
			goto end;
	}
	/* MSI failed. Try legacy irq */
	ret = hif_pci_configure_legacy_irq(sc);
	if (ret < 0) {
		HIF_ERROR("%s: hif_pci_configure_legacy_irq error = %d",
			__func__, ret);
		return ret;
	}
end:
	scn->request_irq_done = true;
	return 0;
}

/**
 * hif_target_sync() : ensure the target is ready
 * @scn: hif controll structure
 *
 * Informs fw that we plan to use legacy interupts so that
 * it can begin booting. Ensures that the fw finishes booting
 * before continuing. Should be called before trying to write
 * to the targets other registers for the first time.
 *
 * Return: none
 */
void hif_target_sync(struct ol_softc *scn)
{
	hif_write32_mb(scn->mem+(SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
				PCIE_INTR_FIRMWARE_MASK);

	hif_write32_mb(scn->mem + PCIE_LOCAL_BASE_ADDRESS +
			PCIE_SOC_WAKE_ADDRESS,
			PCIE_SOC_WAKE_V_MASK);
	while (!hif_targ_is_awake(scn, scn->mem))
		;

	if (HAS_FW_INDICATOR) {
		int wait_limit = 500;
		int fw_ind = 0;
		HIF_TRACE("%s: Loop checking FW signal", __func__);
		while (1) {
			fw_ind = hif_read32_mb(scn->hif_sc->mem +
					FW_INDICATOR_ADDRESS);
			if (fw_ind & FW_IND_INITIALIZED)
				break;
			if (wait_limit-- < 0)
				break;
			hif_write32_mb(scn->mem+(SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
				PCIE_INTR_FIRMWARE_MASK);

			cdf_mdelay(10);
		}
		if (wait_limit < 0)
			HIF_TRACE("%s: FW signal timed out",
					__func__);
		else
			HIF_TRACE("%s: Got FW signal, retries = %x",
					__func__, 500-wait_limit);
	}
	hif_write32_mb(scn->mem + PCIE_LOCAL_BASE_ADDRESS +
			PCIE_SOC_WAKE_ADDRESS, PCIE_SOC_WAKE_RESET);
}

/**
 * hif_enable_bus(): enable bus
 *
 * This function enables the bus
 *
 * @ol_sc: soft_sc struct
 * @dev: device pointer
 * @bdev: bus dev pointer
 * bid: bus id pointer
 * type: enum hif_enable_type such as HIF_ENABLE_TYPE_PROBE
 * Return: CDF_STATUS
 */
CDF_STATUS hif_enable_bus(struct ol_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const hif_bus_id *bid,
			  enum hif_enable_type type)
{
	int ret = 0;
	uint32_t hif_type, target_type;
	struct hif_pci_softc *sc;
	uint16_t revision_id;
	uint32_t lcr_val;
	int probe_again = 0;
	struct pci_dev *pdev = bdev;
	const struct pci_device_id *id = bid;

	HIF_TRACE("%s: con_mode = 0x%x, device_id = 0x%x",
		  __func__, cds_get_conparam(), id->device);

	ol_sc = cds_get_context(CDF_MODULE_ID_HIF);
	if (!ol_sc) {
		HIF_ERROR("%s: hif_ctx is NULL", __func__);
		return CDF_STATUS_E_NOMEM;
	}
	sc = ol_sc->hif_sc;
	ol_sc->aps_osdev.bdev = pdev;

	sc->pdev = pdev;
	sc->dev = &pdev->dev;
	ol_sc->aps_osdev.bdev = pdev;
	ol_sc->aps_osdev.device = &pdev->dev;
	ol_sc->aps_osdev.bc.bc_handle = (void *)ol_sc->mem;
	ol_sc->aps_osdev.bc.bc_bustype = type;
	sc->devid = id->device;
	sc->cacheline_sz = dma_get_cache_alignment();
again:
	ret = hif_enable_pci(sc, pdev, id);
	if (ret < 0) {
		HIF_ERROR("%s: ERROR - hif_enable_pci error = %d",
		       __func__, ret);
		goto err_enable_pci;
	}
	HIF_TRACE("%s: hif_enable_pci done", __func__);

	/* Temporary FIX: disable ASPM on peregrine.
	 * Will be removed after the OTP is programmed
	 */
	pci_read_config_dword(pdev, 0x80, &lcr_val);
	pci_write_config_dword(pdev, 0x80, (lcr_val & 0xffffff00));

	device_disable_async_suspend(&pdev->dev);
	pci_read_config_word(pdev, 0x08, &revision_id);

	ret = hif_get_device_type(id->device, revision_id,
						&hif_type, &target_type);
	if (ret < 0) {
		HIF_ERROR("%s: invalid device id/revision_id", __func__);
		goto err_tgtstate;
	}
	HIF_TRACE("%s: hif_type = 0x%x, target_type = 0x%x",
		  __func__, hif_type, target_type);

	hif_register_tbl_attach(sc->ol_sc, hif_type);
	target_register_tbl_attach(sc->ol_sc, target_type);

	ret = hif_pci_probe_tgt_wakeup(sc);
	if (ret < 0) {
		HIF_ERROR("%s: ERROR - hif_pci_prob_wakeup error = %d",
			   __func__, ret);
		if (ret == -EAGAIN)
			probe_again++;
		goto err_tgtstate;
	}
	HIF_TRACE("%s: hif_pci_probe_tgt_wakeup done", __func__);

	ol_sc->target_type = target_type;
	sc->soc_pcie_bar0 = pci_resource_start(pdev, BAR_NUM);
	if (!sc->soc_pcie_bar0) {
		HIF_ERROR("%s: ERROR - cannot get CE BAR0", __func__);
		ret = -EIO;
		goto err_tgtstate;
	}
	ol_sc->mem_pa = sc->soc_pcie_bar0;

	BUG_ON(pci_get_drvdata(sc->pdev) != NULL);
	pci_set_drvdata(sc->pdev, sc);

	ret = hif_init_cdf_ctx(ol_sc);
	if (ret != 0) {
		HIF_ERROR("%s: cannot init CDF", __func__);
		goto err_tgtstate;
	}

	hif_target_sync(ol_sc);
	return 0;

err_tgtstate:
	hif_deinit_cdf_ctx(ol_sc);
	hif_disable_pci(sc);
	sc->pci_enabled = false;
	HIF_ERROR("%s: error, hif_disable_pci done", __func__);
	return CDF_STATUS_E_ABORTED;

err_enable_pci:
	if (probe_again && (probe_again <= ATH_PCI_PROBE_RETRY_MAX)) {
		int delay_time;

		HIF_INFO("%s: pci reprobe", __func__);
		/* 10, 40, 90, 100, 100, ... */
		delay_time = max(100, 10 * (probe_again * probe_again));
		cdf_mdelay(delay_time);
		goto again;
	}
	return ret;
}

/**
 * hif_get_target_type(): Get the target type
 *
 * This function is used to query the target type.
 *
 * @ol_sc: ol_softc struct pointer
 * @dev: device pointer
 * @bdev: bus dev pointer
 * @bid: bus id pointer
 * @hif_type: HIF type such as HIF_TYPE_QCA6180
 * @target_type: target type such as TARGET_TYPE_QCA6180
 *
 * Return: 0 for success
 */
int hif_get_target_type(struct ol_softc *ol_sc, struct device *dev,
	void *bdev, const hif_bus_id *bid, uint32_t *hif_type,
	uint32_t *target_type)
{
	uint16_t revision_id;
	struct pci_dev *pdev = bdev;
	const struct pci_device_id *id = bid;

	pci_read_config_word(pdev, 0x08, &revision_id);
	return hif_get_device_type(id->device, revision_id,
			hif_type, target_type);
}

#ifdef FEATURE_RUNTIME_PM

void hif_pm_runtime_get_noresume(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;
	struct hif_pci_softc *sc;

	if (NULL == scn)
		return;

	sc = scn->hif_sc;
	if (NULL == sc)
		return;

	sc->pm_stats.runtime_get++;
	pm_runtime_get_noresume(sc->dev);
}

/**
 * hif_pm_runtime_get() - do a get opperation on the device
 *
 * A get opperation will prevent a runtime suspend untill a
 * corresponding put is done.  This api should be used when sending
 * data.
 *
 * CONTRARY TO THE REGULAR RUNTIME PM, WHEN THE BUS IS SUSPENDED,
 * THIS API WILL ONLY REQUEST THE RESUME AND NOT TO A GET!!!
 *
 * return: success if the bus is up and a get has been issued
 *   otherwise an error code.
 */
int hif_pm_runtime_get(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;
	struct hif_pci_softc *sc;
	int ret;
	int pm_state;

	if (NULL == scn) {
		HIF_ERROR("%s: Could not do runtime get, scn is null",
				__func__);
		return -EFAULT;
	}
	sc = scn->hif_sc;

	pm_state = cdf_atomic_read(&sc->pm_state);

	if (pm_state  == HIF_PM_RUNTIME_STATE_ON ||
			pm_state == HIF_PM_RUNTIME_STATE_NONE) {
		sc->pm_stats.runtime_get++;
		ret = __hif_pm_runtime_get(sc->dev);

		/* Get can return 1 if the device is already active, just return
		 * success in that case
		 */
		if (ret > 0)
			ret = 0;

		if (ret)
			hif_pm_runtime_put(hif_ctx);

		if (ret && ret != -EINPROGRESS) {
			sc->pm_stats.runtime_get_err++;
			HIF_ERROR("%s: Runtime Get PM Error in pm_state:%d ret: %d",
				__func__, cdf_atomic_read(&sc->pm_state), ret);
		}

		return ret;
	}

	sc->pm_stats.request_resume++;
	sc->pm_stats.last_resume_caller = (void *)_RET_IP_;
	ret = hif_pm_request_resume(sc->dev);

	return -EAGAIN;
}

/**
 * hif_pm_runtime_put() - do a put opperation on the device
 *
 * A put opperation will allow a runtime suspend after a corresponding
 * get was done.  This api should be used when sending data.
 *
 * This api will return a failure if runtime pm is stopped
 * This api will return failure if it would decrement the usage count below 0.
 *
 * return: CDF_STATUS_SUCCESS if the put is performed
 */
int hif_pm_runtime_put(void *hif_ctx)
{
	struct ol_softc *scn = (struct ol_softc *)hif_ctx;
	struct hif_pci_softc *sc;
	int pm_state, usage_count;
	unsigned long flags;
	char *error = NULL;

	if (NULL == scn) {
		HIF_ERROR("%s: Could not do runtime put, scn is null",
				__func__);
		return -EFAULT;
	}
	sc = scn->hif_sc;

	usage_count = atomic_read(&sc->dev->power.usage_count);

	if (usage_count == 1) {
		pm_state = cdf_atomic_read(&sc->pm_state);

		if (pm_state == HIF_PM_RUNTIME_STATE_NONE)
			error = "Ignoring unexpected put when runtime pm is disabled";

	} else if (usage_count == 0) {
		error = "PUT Without a Get Operation";
	}

	if (error) {
		spin_lock_irqsave(&sc->runtime_lock, flags);
		hif_pci_runtime_pm_warn(sc, error);
		spin_unlock_irqrestore(&sc->runtime_lock, flags);
		return -EINVAL;
	}

	sc->pm_stats.runtime_put++;

	hif_pm_runtime_mark_last_busy(sc->dev);
	hif_pm_runtime_put_auto(sc->dev);

	return 0;
}


/**
 * __hif_pm_runtime_prevent_suspend() - prevent runtime suspend for a protocol reason
 * @hif_sc: pci context
 * @lock: runtime_pm lock being acquired
 *
 * Return 0 if successful.
 */
static int __hif_pm_runtime_prevent_suspend(struct hif_pci_softc
		*hif_sc, struct hif_pm_runtime_lock *lock)
{
	int ret = 0;

	/*
	 * We shouldn't be setting context->timeout to zero here when
	 * context is active as we will have a case where Timeout API's
	 * for the same context called back to back.
	 * eg: echo "1=T:10:T:20" > /d/cnss_runtime_pm
	 * Set context->timeout to zero in hif_pm_runtime_prevent_suspend
	 * API to ensure the timeout version is no more active and
	 * list entry of this context will be deleted during allow suspend.
	 */
	if (lock->active)
		return 0;

	ret = __hif_pm_runtime_get(hif_sc->dev);

	/**
	 * The ret can be -EINPROGRESS, if Runtime status is RPM_RESUMING or
	 * RPM_SUSPENDING. Any other negative value is an error.
	 * We shouldn't be do runtime_put here as in later point allow
	 * suspend gets called with the the context and there the usage count
	 * is decremented, so suspend will be prevented.
	 */

	if (ret < 0 && ret != -EINPROGRESS) {
		hif_sc->pm_stats.runtime_get_err++;
		hif_pci_runtime_pm_warn(hif_sc,
				"Prevent Suspend Runtime PM Error");
	}

	hif_sc->prevent_suspend_cnt++;

	lock->active = true;

	list_add_tail(&lock->list, &hif_sc->prevent_suspend_list);

	hif_sc->pm_stats.prevent_suspend++;

	HIF_ERROR("%s: in pm_state:%d ret: %d", __func__,
			cdf_atomic_read(&hif_sc->pm_state), ret);

	return ret;
}

static int __hif_pm_runtime_allow_suspend(struct hif_pci_softc *hif_sc,
		struct hif_pm_runtime_lock *lock)
{
	int ret = 0;
	int usage_count;

	if (hif_sc->prevent_suspend_cnt == 0)
		return ret;

	if (!lock->active)
		return ret;

	usage_count = atomic_read(&hif_sc->dev->power.usage_count);

	/*
	 * During Driver unload, platform driver increments the usage
	 * count to prevent any runtime suspend getting called.
	 * So during driver load in HIF_PM_RUNTIME_STATE_NONE state the
	 * usage_count should be one. Ideally this shouldn't happen as
	 * context->active should be active for allow suspend to happen
	 * Handling this case here to prevent any failures.
	 */
	if ((cdf_atomic_read(&hif_sc->pm_state) == HIF_PM_RUNTIME_STATE_NONE
				&& usage_count == 1) || usage_count == 0) {
		hif_pci_runtime_pm_warn(hif_sc,
				"Allow without a prevent suspend");
		return -EINVAL;
	}

	list_del(&lock->list);

	hif_sc->prevent_suspend_cnt--;

	lock->active = false;
	lock->timeout = 0;

	hif_pm_runtime_mark_last_busy(hif_sc->dev);
	ret = hif_pm_runtime_put_auto(hif_sc->dev);

	HIF_ERROR("%s: in pm_state:%d ret: %d", __func__,
			cdf_atomic_read(&hif_sc->pm_state), ret);

	hif_sc->pm_stats.allow_suspend++;
	return ret;
}

/**
 * hif_pm_runtime_lock_timeout_fn() - callback the runtime lock timeout
 * @data: calback data that is the pci context
 *
 * if runtime locks are aquired with a timeout, this function releases
 * the locks when the last runtime lock expires.
 *
 * dummy implementation until lock acquisition is implemented.
 */
static void hif_pm_runtime_lock_timeout_fn(unsigned long data)
{
	struct hif_pci_softc *hif_sc = (struct hif_pci_softc *)data;
	unsigned long flags;
	unsigned long timer_expires;
	struct hif_pm_runtime_lock *context, *temp;

	spin_lock_irqsave(&hif_sc->runtime_lock, flags);

	timer_expires = hif_sc->runtime_timer_expires;

	/* Make sure we are not called too early, this should take care of
	 * following case
	 *
	 * CPU0                         CPU1 (timeout function)
	 * ----                         ----------------------
	 * spin_lock_irq
	 *                              timeout function called
	 *
	 * mod_timer()
	 *
	 * spin_unlock_irq
	 *                              spin_lock_irq
	 */
	if (timer_expires > 0 && !time_after(timer_expires, jiffies)) {
		hif_sc->runtime_timer_expires = 0;
		list_for_each_entry_safe(context, temp,
				&hif_sc->prevent_suspend_list, list) {
			if (context->timeout) {
				__hif_pm_runtime_allow_suspend(hif_sc, context);
				hif_sc->pm_stats.allow_suspend_timeout++;
			}
		}
	}

	spin_unlock_irqrestore(&hif_sc->runtime_lock, flags);
}

int hif_pm_runtime_prevent_suspend(void *ol_sc,
		struct hif_pm_runtime_lock *data)
{
	struct ol_softc *sc = (struct ol_softc *)ol_sc;
	struct hif_pci_softc *hif_sc = sc->hif_sc;
	struct hif_pm_runtime_lock *context = data;
	unsigned long flags;

	if (!sc->enable_runtime_pm)
		return 0;

	if (!context)
		return -EINVAL;

	spin_lock_irqsave(&hif_sc->runtime_lock, flags);
	context->timeout = 0;
	__hif_pm_runtime_prevent_suspend(hif_sc, context);
	spin_unlock_irqrestore(&hif_sc->runtime_lock, flags);

	return 0;
}

int hif_pm_runtime_allow_suspend(void *ol_sc, struct hif_pm_runtime_lock *data)
{
	struct ol_softc *sc = (struct ol_softc *)ol_sc;
	struct hif_pci_softc *hif_sc = sc->hif_sc;
	struct hif_pm_runtime_lock *context = data;

	unsigned long flags;

	if (!sc->enable_runtime_pm)
		return 0;

	if (!context)
		return -EINVAL;

	spin_lock_irqsave(&hif_sc->runtime_lock, flags);

	__hif_pm_runtime_allow_suspend(hif_sc, context);

	/* The list can be empty as well in cases where
	 * we have one context in the list and the allow
	 * suspend came before the timer expires and we delete
	 * context above from the list.
	 * When list is empty prevent_suspend count will be zero.
	 */
	if (hif_sc->prevent_suspend_cnt == 0 &&
			hif_sc->runtime_timer_expires > 0) {
		del_timer(&hif_sc->runtime_timer);
		hif_sc->runtime_timer_expires = 0;
	}

	spin_unlock_irqrestore(&hif_sc->runtime_lock, flags);

	return 0;
}

/**
 * hif_pm_runtime_prevent_suspend_timeout() - Prevent runtime suspend timeout
 * @ol_sc: HIF context
 * @lock: which lock is being acquired
 * @delay: Timeout in milliseconds
 *
 * Prevent runtime suspend with a timeout after which runtime suspend would be
 * allowed. This API uses a single timer to allow the suspend and timer is
 * modified if the timeout is changed before timer fires.
 * If the timeout is less than autosuspend_delay then use mark_last_busy instead
 * of starting the timer.
 *
 * It is wise to try not to use this API and correct the design if possible.
 *
 * Return: 0 on success and negative error code on failure
 */
int hif_pm_runtime_prevent_suspend_timeout(void *ol_sc,
		struct hif_pm_runtime_lock *lock, unsigned int delay)
{
	struct ol_softc *sc = (struct ol_softc *)ol_sc;
	struct hif_pci_softc *hif_sc = sc->hif_sc;
	int ret = 0;
	unsigned long expires;
	unsigned long flags;
	struct hif_pm_runtime_lock *context = lock;

	if (cds_is_load_unload_in_progress()) {
		HIF_ERROR("%s: Load/unload in progress, ignore!",
				__func__);
		return -EINVAL;
	}

	if (cds_is_logp_in_progress()) {
		HIF_ERROR("%s: LOGP in progress, ignore!", __func__);
		return -EINVAL;
	}

	if (!sc->enable_runtime_pm)
		return 0;

	if (!context)
		return -EINVAL;

	/*
	 * Don't use internal timer if the timeout is less than auto suspend
	 * delay.
	 */
	if (delay <= hif_sc->dev->power.autosuspend_delay) {
		hif_pm_request_resume(hif_sc->dev);
		hif_pm_runtime_mark_last_busy(hif_sc->dev);
		return ret;
	}

	expires = jiffies + msecs_to_jiffies(delay);
	expires += !expires;

	spin_lock_irqsave(&hif_sc->runtime_lock, flags);

	context->timeout = delay;
	ret = __hif_pm_runtime_prevent_suspend(hif_sc, context);
	hif_sc->pm_stats.prevent_suspend_timeout++;

	/* Modify the timer only if new timeout is after already configured
	 * timeout
	 */
	if (time_after(expires, hif_sc->runtime_timer_expires)) {
		mod_timer(&hif_sc->runtime_timer, expires);
		hif_sc->runtime_timer_expires = expires;
	}

	spin_unlock_irqrestore(&hif_sc->runtime_lock, flags);

	HIF_ERROR("%s: pm_state: %d delay: %dms ret: %d\n", __func__,
			cdf_atomic_read(&hif_sc->pm_state), delay, ret);

	return ret;
}

/**
 * hif_runtime_lock_init() - API to initialize Runtime PM context
 * @name: Context name
 *
 * This API initalizes the Runtime PM context of the caller and
 * return the pointer.
 *
 * Return: void *
 */
struct hif_pm_runtime_lock *hif_runtime_lock_init(const char *name)
{
	struct hif_pm_runtime_lock *context;

	context = cdf_mem_malloc(sizeof(*context));
	if (!context) {
		HIF_ERROR("%s: No memory for Runtime PM wakelock context\n",
				__func__);
		return NULL;
	}

	context->name = name ? name : "Default";
	return context;
}

/**
 * hif_runtime_lock_deinit() - This API frees the runtime pm ctx
 * @data: Runtime PM context
 *
 * Return: void
 */
void hif_runtime_lock_deinit(struct hif_pm_runtime_lock *data)
{
	unsigned long flags;
	struct hif_pm_runtime_lock *context = data;
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct hif_pci_softc *sc;

	if (!scn)
		return;

	sc = scn->hif_sc;

	if (!sc)
		return;

	if (!context)
		return;

	/*
	 * Ensure to delete the context list entry and reduce the usage count
	 * before freeing the context if context is active.
	 */
	spin_lock_irqsave(&sc->runtime_lock, flags);
	__hif_pm_runtime_allow_suspend(sc, context);
	spin_unlock_irqrestore(&sc->runtime_lock, flags);

	cdf_mem_free(context);
}

#endif /* FEATURE_RUNTIME_PM */

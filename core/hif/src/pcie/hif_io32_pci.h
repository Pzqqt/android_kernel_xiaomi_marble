/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_IO32_PCI_H__
#define __HIF_IO32_PCI_H__

#ifdef HIF_PCI

#include "hif.h"
#include "regtable.h"
#include "ce_reg.h"
#include "cdf_atomic.h"
#include "if_pci.h"
/*
 * For maximum performance and no power management, set this to 1.
 * For power management at the cost of performance, set this to 0.
 */
#define CONFIG_ATH_PCIE_MAX_PERF 0

/*
 * For keeping the target awake till the driver is
 * loaded, set this to 1
 */
#define CONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD 1

/*
 * When CONFIG_ATH_PCIE_MAX_PERF is 0:
 * To use LIKELY hints, set this to 1 (slightly better performance, more power)
 * To ignore "LIKELY" hints, set this to 0 (slightly worse performance,
 * less power)
 */
#if defined(CONFIG_ATH_PCIE_MAX_PERF)
#define CONFIG_ATH_PCIE_ACCESS_LIKELY 0
#else
#define CONFIG_ATH_PCIE_ACCESS_LIKELY 1
#endif

/*
 * PCI-E L1 ASPPM sub-states
 * To enable clock gating in L1 state, set this to 1.
 * (less power, slightly more wakeup latency)
 * To disable clock gating in L1 state, set this to 0. (slighly more power)
 */
#define CONFIG_PCIE_ENABLE_L1_CLOCK_GATE 1

/*
 * PCIE_ACCESS_LOG_NUM specifies the number of
 * read/write records to store
 */
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
#define PCIE_ACCESS_LOG_NUM 500
#endif

/* 64-bit MSI support */
#define CONFIG_PCIE_64BIT_MSI 0

/* BAR0 ready checking for AR6320v2 */
#define PCIE_BAR0_READY_CHECKING 0

/* AXI gating when L1, L2 to reduce power consumption */
#define CONFIG_PCIE_ENABLE_AXI_CLK_GATE 0

#define hif_read32_mb(addr)         ioread32((void __iomem *)addr)
#define hif_write32_mb(addr, value) \
	iowrite32((u32)(value), (void __iomem *)(addr))

extern int hif_target_sleep_state_adjust(struct ol_softc *scn,
					 bool sleep_ok,
					 bool wait_for_it);

#if CONFIG_ATH_PCIE_MAX_PERF
#define A_TARGET_ACCESS_BEGIN(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_END(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_OK(scn) 1

#define A_TARGET_ACCESS_LIKELY(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_UNLIKELY(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_READ(scn, offset) \
	hif_read32_mb(scn->mem + (offset))

void war_pci_write32(char *addr, u32 offset, u32 value);
#define A_TARGET_WRITE(scn, offset, value) \
	war_pci_write32(scn->mem, (offset), (value))

#define A_TARGET_ACCESS_BEGIN_RET(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_BEGIN_RET_EXT(scn, val) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_BEGIN_RET_PTR(scn) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_END_RET(scn)	\
	do {struct ol_softc *unused = scn; \
		unused = unused; } while (0)

#define A_TARGET_ACCESS_END_RET_EXT(scn, val) \
	do {struct ol_softc *unused = scn; \
	    unused = unused; } while (0)

#define A_TARGET_ACCESS_END_RET_PTR(scn) \
	do {struct ol_softc *unused = scn; \
		unused = unused; } while (0)

#else                           /* CONFIG_ATH_PCIE_MAX_PERF */

void war_pci_write32(char *addr, u32 offset, u32 value);

#define A_TARGET_ACCESS_BEGIN_RET_EXT(scn, val) \
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
		Q_TARGET_ACCESS_BEGIN(scn) < 0) \
		val = -1; \
} while (0)

#define A_TARGET_ACCESS_BEGIN_RET(scn) \
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
	    Q_TARGET_ACCESS_BEGIN(scn) < 0) \
		return ATH_ISR_NOSCHED; \
} while (0)

#define A_TARGET_ACCESS_BEGIN_RET_PTR(scn) \
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
	    Q_TARGET_ACCESS_BEGIN(scn) < 0) \
		return NULL; \
} while (0)

#define A_TARGET_ACCESS_BEGIN(scn) \
do { \
	if (Q_TARGET_ACCESS_BEGIN(scn) < 0) \
		return; \
} while (0)

#define Q_TARGET_ACCESS_BEGIN(scn) \
	hif_target_sleep_state_adjust(scn, false, true)

#define A_TARGET_ACCESS_END_RET(scn)	\
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
	    Q_TARGET_ACCESS_END(scn) < 0) \
		return ATH_ISR_NOSCHED; \
} while (0)

#define A_TARGET_ACCESS_END_RET_EXT(scn, val) \
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
	    Q_TARGET_ACCESS_END(scn) < 0) \
		val = -1; \
} while (0)

#define A_TARGET_ACCESS_END_RET_PTR(scn) \
do { \
	if (!WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && \
	    Q_TARGET_ACCESS_END(scn) < 0) \
		return NULL; \
} while (0)
#define A_TARGET_ACCESS_END(scn) \
do { \
	if (Q_TARGET_ACCESS_END(scn) < 0) \
		return; \
} while (0)

#define Q_TARGET_ACCESS_END(scn) \
	hif_target_sleep_state_adjust(scn, true, false)

#define A_TARGET_ACCESS_OK(scn) hif_target_forced_awake(scn)

#if CONFIG_ATH_PCIE_ACCESS_LIKELY
#define A_TARGET_ACCESS_LIKELY(scn) \
	hif_target_sleep_state_adjust(scn, false, false)
#define A_TARGET_ACCESS_UNLIKELY(scn) \
	hif_target_sleep_state_adjust(scn, true, false)
#else                           /* CONFIG_ATH_PCIE_ACCESS_LIKELY */
#define A_TARGET_ACCESS_LIKELY(scn) \
	do { \
		unsigned long unused = (unsigned long)(scn); \
		unused = unused; \
	} while (0)

#define A_TARGET_ACCESS_UNLIKELY(scn) \
	do { \
		unsigned long unused = (unsigned long)(scn); \
		unused = unused; \
	} while (0)
#endif /* CONFIG_ATH_PCIE_ACCESS_LIKELY */

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
extern uint32_t hif_target_read_checked(struct ol_softc *scn,
					uint32_t offset);
extern void hif_target_write_checked(struct ol_softc *scn, uint32_t offset,
				     uint32_t value);
#define A_TARGET_READ(scn, offset) \
	hif_target_read_checked(scn, (offset))
#define A_TARGET_WRITE(scn, offset, value) \
	hif_target_write_checked(scn, (offset), (value))
#else                           /* CONFIG_ATH_PCIE_ACCESS_DEBUG */
#define A_TARGET_READ(scn, offset) \
	hif_read32_mb(scn->mem + (offset))
#define A_TARGET_WRITE(scn, offset, value) \
	war_pci_write32(scn->mem, (offset), (value))
#endif
#endif /* CONFIG_ATH_PCIE_MAX_PERF */


irqreturn_t hif_fw_interrupt_handler(int irq, void *arg);

/**
 * ce_irq_enable() - ce_irq_enable
 * @scn: ol_softc
 * @ce_id: ce_id
 *
 * Return: void
 */
static inline void ce_irq_enable(struct ol_softc *scn, int ce_id)
{
	uint32_t tmp = 1 << ce_id;
	struct hif_pci_softc *sc = scn->hif_sc;

	cdf_spin_lock_irqsave(&scn->irq_lock);
	scn->ce_irq_summary &= ~tmp;
	if (scn->ce_irq_summary == 0) {
		/* Enable Legacy PCI line interrupts */
		if (LEGACY_INTERRUPTS(sc) &&
			(scn->target_status != OL_TRGET_STATUS_RESET) &&
			(!cdf_atomic_read(&scn->link_suspended))) {

			hif_write32_mb(scn->mem +
				(SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
				HOST_GROUP0_MASK);

			hif_read32_mb(scn->mem +
					(SOC_CORE_BASE_ADDRESS |
					PCIE_INTR_ENABLE_ADDRESS));
		}
	}
	if (scn->hif_init_done == true)
		A_TARGET_ACCESS_END(scn);
	cdf_spin_unlock_irqrestore(&scn->irq_lock);

	/* check for missed firmware crash */
	hif_fw_interrupt_handler(0, scn);
}
/**
 * ce_irq_disable() - ce_irq_disable
 * @scn: ol_softc
 * @ce_id: ce_id
 *
 * Return: void
 */
static inline void ce_irq_disable(struct ol_softc *scn, int ce_id)
{
	/* For Rome only need to wake up target */
	A_TARGET_ACCESS_BEGIN(scn);
}
/**
 * soc_wake_reset() - soc_wake_reset
 * @scn: ol_softc
 *
 * Return: void
 */
static inline void soc_wake_reset(struct ol_softc *scn)
{
	hif_write32_mb(scn->mem +
		PCIE_LOCAL_BASE_ADDRESS +
		PCIE_SOC_WAKE_ADDRESS,
		PCIE_SOC_WAKE_RESET);
}
#endif /* HIF_PCI */
#endif /* __HIF_IO32_PCI_H__ */

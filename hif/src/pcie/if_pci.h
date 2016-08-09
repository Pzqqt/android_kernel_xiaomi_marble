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

#ifndef __ATH_PCI_H__
#define __ATH_PCI_H__

#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#define ATH_DBG_DEFAULT   0
#include "hif.h"
#include "cepci.h"
#include "ce_main.h"

/* An address (e.g. of a buffer) in Copy Engine space. */

#define HIF_MAX_TASKLET_NUM 11
struct hif_tasklet_entry {
	uint8_t id;        /* 0 - 9: maps to CE, 10: fw */
	void *hif_handler; /* struct hif_pci_softc */
};

/**
 * enum hif_pm_runtime_state - Driver States for Runtime Power Management
 * HIF_PM_RUNTIME_STATE_NONE: runtime pm is off
 * HIF_PM_RUNTIME_STATE_ON: runtime pm is active and link is active
 * HIF_PM_RUNTIME_STATE_INPROGRESS: a runtime suspend or resume is in progress
 * HIF_PM_RUNTIME_STATE_SUSPENDED: the driver is runtime suspended
 */
enum hif_pm_runtime_state {
	HIF_PM_RUNTIME_STATE_NONE,
	HIF_PM_RUNTIME_STATE_ON,
	HIF_PM_RUNTIME_STATE_INPROGRESS,
	HIF_PM_RUNTIME_STATE_SUSPENDED,
};

#ifdef FEATURE_RUNTIME_PM

/**
 * struct hif_pm_runtime_lock - data structure for preventing runtime suspend
 * @list - global list of runtime locks
 * @active - true if this lock is preventing suspend
 * @name - character string for tracking this lock
 */
struct hif_pm_runtime_lock {
	struct list_head list;
	bool active;
	uint32_t timeout;
	const char *name;
};

/* Debugging stats for Runtime PM */
struct hif_pci_pm_stats {
	u32 suspended;
	u32 suspend_err;
	u32 resumed;
	u32 runtime_get;
	u32 runtime_put;
	u32 request_resume;
	u32 allow_suspend;
	u32 prevent_suspend;
	u32 prevent_suspend_timeout;
	u32 allow_suspend_timeout;
	u32 runtime_get_err;
	void *last_resume_caller;
	unsigned long suspend_jiffies;
};
#endif

/**
 * struct hif_msi_info - Structure to hold msi info
 * @magic: cookie
 * @magic_da: dma address
 * @dmaContext: dma address
 *
 * Structure to hold MSI information for PCIe interrupts
 */
struct hif_msi_info {
	void *magic;
	dma_addr_t magic_da;
	OS_DMA_MEM_CONTEXT(dmacontext)
};

struct hif_pci_softc {
	struct HIF_CE_state ce_sc;
	void __iomem *mem;      /* PCI address. */
	/* For efficiency, should be first in struct */

	struct device *dev;
	struct pci_dev *pdev;
	int num_msi_intrs;      /* number of MSI interrupts granted */
	/* 0 --> using legacy PCI line interrupts */
	struct tasklet_struct intr_tq;  /* tasklet */

	struct hif_msi_info msi_info;
	int irq;
	int irq_event;
	int cacheline_sz;
	u16 devid;
	qdf_dma_addr_t soc_pcie_bar0;
	struct hif_tasklet_entry tasklet_entries[HIF_MAX_TASKLET_NUM];
	bool pci_enabled;
	qdf_spinlock_t irq_lock;
	qdf_work_t reschedule_tasklet_work;
	uint32_t lcr_val;
#ifdef FEATURE_RUNTIME_PM
	atomic_t pm_state;
	uint32_t prevent_suspend_cnt;
	struct hif_pci_pm_stats pm_stats;
	struct work_struct pm_work;
	spinlock_t runtime_lock;
	struct timer_list runtime_timer;
	struct list_head prevent_suspend_list;
	unsigned long runtime_timer_expires;
	struct hif_pm_runtime_lock *prevent_linkdown_lock;
#ifdef WLAN_OPEN_SOURCE
	struct dentry *pm_dentry;
#endif
#endif
};

bool hif_pci_targ_is_present(struct hif_softc *scn, void *__iomem *mem);
int hif_configure_irq(struct hif_softc *sc);
void hif_pci_cancel_deferred_target_sleep(struct hif_softc *scn);
void wlan_tasklet(unsigned long data);
irqreturn_t hif_pci_interrupt_handler(int irq, void *arg);

/*
 * A firmware interrupt to the Host is indicated by the
 * low bit of SCRATCH_3_ADDRESS being set.
 */
#define FW_EVENT_PENDING_REG_ADDRESS SCRATCH_3_ADDRESS

/*
 * Typically, MSI Interrupts are used with PCIe. To force use of legacy
 * "ABCD" PCI line interrupts rather than MSI, define
 * FORCE_LEGACY_PCI_INTERRUPTS.
 * Even when NOT forced, the driver may attempt to use legacy PCI interrupts
 * MSI allocation fails
 */
#define LEGACY_INTERRUPTS(sc) ((sc)->num_msi_intrs == 0)

/*
 * There may be some pending tx frames during platform suspend.
 * Suspend operation should be delayed until those tx frames are
 * transfered from the host to target. This macro specifies how
 * long suspend thread has to sleep before checking pending tx
 * frame count.
 */
#define OL_ATH_TX_DRAIN_WAIT_DELAY     50       /* ms */

#define HIF_CE_DRAIN_WAIT_DELAY        10       /* ms */
/*
 * Wait time (in unit of OL_ATH_TX_DRAIN_WAIT_DELAY) for pending
 * tx frame completion before suspend. Refer: hif_pci_suspend()
 */
#ifndef QCA_WIFI_3_0_EMU
#define OL_ATH_TX_DRAIN_WAIT_CNT       10
#else
#define OL_ATH_TX_DRAIN_WAIT_CNT       60
#endif

#ifdef FEATURE_RUNTIME_PM
#include <linux/pm_runtime.h>

static inline int hif_pm_request_resume(struct device *dev)
{
	return pm_request_resume(dev);
}
static inline int __hif_pm_runtime_get(struct device *dev)
{
	return pm_runtime_get(dev);
}

static inline int hif_pm_runtime_put_auto(struct device *dev)
{
	return pm_runtime_put_autosuspend(dev);
}

static inline void hif_pm_runtime_mark_last_busy(struct device *dev)
{
	pm_runtime_mark_last_busy(dev);
}

static inline int hif_pm_runtime_resume(struct device *dev)
{
	return pm_runtime_resume(dev);
}
#else
static inline void hif_pm_runtime_mark_last_busy(struct device *dev) { }
#endif /* FEATURE_RUNTIME_PM */
#endif /* __ATH_PCI_H__ */

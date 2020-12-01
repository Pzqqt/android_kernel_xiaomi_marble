/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

#ifndef __ATH_PCI_H__
#define __ATH_PCI_H__

#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#define ATH_DBG_DEFAULT   0
#define DRAM_SIZE               0x000a8000
#include "hif.h"
#include "hif_runtime_pm.h"
#include "cepci.h"
#include "ce_main.h"

#ifdef FORCE_WAKE
/* Register to wake the UMAC from power collapse */
#define PCIE_SOC_PCIE_REG_PCIE_SCRATCH_0_SOC_PCIE_REG (0x01E04000 + 0x40)
/* Register used for handshake mechanism to validate UMAC is awake */
#define PCIE_PCIE_LOCAL_REG_PCIE_SOC_WAKE_PCIE_LOCAL_REG (0x01E00000 + 0x3004)
/* Timeout duration to validate UMAC wake status */
#ifdef HAL_CONFIG_SLUB_DEBUG_ON
#define FORCE_WAKE_DELAY_TIMEOUT_MS 500
#else
#define FORCE_WAKE_DELAY_TIMEOUT_MS 50
#endif /* HAL_CONFIG_SLUB_DEBUG_ON */
/* Validate UMAC status every 5ms */
#define FORCE_WAKE_DELAY_MS 5
#endif /* FORCE_WAKE */

#ifdef QCA_HIF_HIA_EXTND
extern int32_t frac, intval, ar900b_20_targ_clk, qca9888_20_targ_clk;
#endif

/* An address (e.g. of a buffer) in Copy Engine space. */

#define HIF_MAX_TASKLET_NUM 11
struct hif_tasklet_entry {
	uint8_t id;        /* 0 - 9: maps to CE, 10: fw */
	void *hif_handler; /* struct hif_pci_softc */
};

struct hang_event_bus_info {
	uint16_t tlv_header;
	uint16_t dev_id;
} qdf_packed;

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
	OS_DMA_MEM_CONTEXT(dmacontext);
};

/**
 * struct hif_pci_stats - Account for hif pci based statistics
 * @mhi_force_wake_request_vote: vote for mhi
 * @mhi_force_wake_failure: mhi force wake failure
 * @mhi_force_wake_success: mhi force wake success
 * @soc_force_wake_register_write_success: write to soc wake
 * @soc_force_wake_failure: soc force wake failure
 * @soc_force_wake_success: soc force wake success
 * @mhi_force_wake_release_success: mhi force wake release success
 * @soc_force_wake_release_success: soc force wake release
 */
struct hif_pci_stats {
	uint32_t mhi_force_wake_request_vote;
	uint32_t mhi_force_wake_failure;
	uint32_t mhi_force_wake_success;
	uint32_t soc_force_wake_register_write_success;
	uint32_t soc_force_wake_failure;
	uint32_t soc_force_wake_success;
	uint32_t mhi_force_wake_release_failure;
	uint32_t mhi_force_wake_release_success;
	uint32_t soc_force_wake_release_success;
};

struct hif_pci_softc {
	struct HIF_CE_state ce_sc;
	void __iomem *mem;      /* PCI address. */
	void __iomem *mem_ce;   /* PCI address for CE. */
	size_t mem_len;

	struct device *dev;	/* For efficiency, should be first in struct */
	struct pci_dev *pdev;
	int num_msi_intrs;      /* number of MSI interrupts granted */
	/* 0 --> using legacy PCI line interrupts */
	struct tasklet_struct intr_tq;  /* tasklet */
	struct hif_msi_info msi_info;
	int ce_msi_irq_num[CE_COUNT_MAX];
	int irq;
	int irq_event;
	int cacheline_sz;
	u16 devid;
	struct hif_tasklet_entry tasklet_entries[HIF_MAX_TASKLET_NUM];
	bool pci_enabled;
	bool use_register_windowing;
	uint32_t register_window;
	qdf_spinlock_t register_access_lock;
	qdf_spinlock_t irq_lock;
	qdf_work_t reschedule_tasklet_work;
	uint32_t lcr_val;
#ifdef FEATURE_RUNTIME_PM
	struct hif_runtime_pm_ctx rpm_ctx;
#endif
	int (*hif_enable_pci)(struct hif_pci_softc *sc, struct pci_dev *pdev,
			      const struct pci_device_id *id);
	void (*hif_pci_deinit)(struct hif_pci_softc *sc);
	void (*hif_pci_get_soc_info)(struct hif_pci_softc *sc,
				     struct device *dev);
	struct hif_pci_stats stats;
#ifdef HIF_CPU_PERF_AFFINE_MASK
	/* Stores the affinity hint mask for each CE IRQ */
	qdf_cpu_mask ce_irq_cpu_mask[CE_COUNT_MAX];
#endif
};

bool hif_pci_targ_is_present(struct hif_softc *scn, void *__iomem *mem);
int hif_configure_irq(struct hif_softc *sc);
void hif_pci_cancel_deferred_target_sleep(struct hif_softc *scn);
void wlan_tasklet(unsigned long data);
irqreturn_t hif_pci_legacy_ce_interrupt_handler(int irq, void *arg);
int hif_pci_addr_in_boundary(struct hif_softc *scn, uint32_t offset);

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
 * transferred from the host to target. This macro specifies how
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

#ifdef FORCE_WAKE
/**
 * hif_print_pci_stats() - Display HIF PCI stats
 * @hif_ctx - HIF pci handle
 *
 * Return: None
 */
void hif_print_pci_stats(struct hif_pci_softc *pci_scn);
#else
static inline
void hif_print_pci_stats(struct hif_pci_softc *pci_scn)
{
}
#endif /* FORCE_WAKE */
#ifdef HIF_BUS_LOG_INFO
bool hif_log_pcie_info(struct hif_softc *scn, uint8_t *data,
		       unsigned int *offset);
#else
static inline
bool hif_log_pcie_info(struct hif_softc *scn, uint8_t *data,
		       unsigned int *offset)
{
	return false;
}
#endif
#endif /* __ATH_PCI_H__ */

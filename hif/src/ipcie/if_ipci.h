/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __ATH_IPCI_H__
#define __ATH_IPCI_H__

#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#define ATH_DBG_DEFAULT   0
#define DRAM_SIZE               0x000a8000
#include "hif.h"
#include "cepci.h"
#include "ce_main.h"
#include "hif_runtime_pm.h"

#ifdef FORCE_WAKE
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
struct hif_ipci_stats {
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

/* Register offset to wake the UMAC from power collapse */
#define PCIE_REG_WAKE_UMAC_OFFSET 0x3004
/* Register to wake the UMAC from power collapse */
#define PCIE_SOC_PCIE_REG_PCIE_SCRATCH_0_SOC_PCIE_REG (0x01E04000 + 0x40)

/* Timeout duration to validate UMAC wake status */
#define FORCE_WAKE_DELAY_TIMEOUT_MS 500

/* Validate UMAC status every 5ms */
#define FORCE_WAKE_DELAY_MS 5
#endif /* FORCE_WAKE */

#ifdef FEATURE_HAL_DELAYED_REG_WRITE
#define EP_VOTE_POLL_TIME_US  50
#define EP_VOTE_POLL_TIME_CNT 2
#ifdef HAL_CONFIG_SLUB_DEBUG_ON
#define EP_WAKE_RESET_DELAY_TIMEOUT_MS 3
#define EP_WAKE_DELAY_TIMEOUT_MS 7
#else
#define EP_WAKE_RESET_DELAY_TIMEOUT_MS 10
#define EP_WAKE_DELAY_TIMEOUT_MS 10
#endif
#define EP_WAKE_RESET_DELAY_US 50
#define EP_WAKE_DELAY_US 200
#endif

struct hif_ipci_softc {
	struct HIF_CE_state ce_sc;
	void __iomem *mem;      /* PCI address. */

	struct device *dev;	/* For efficiency, should be first in struct */
	struct tasklet_struct intr_tq;  /* tasklet */
	int ce_msi_irq_num[CE_COUNT_MAX];
	bool use_register_windowing;
	uint32_t register_window;
	qdf_spinlock_t register_access_lock;
	qdf_spinlock_t irq_lock;
	bool grp_irqs_disabled;
#ifdef FEATURE_RUNTIME_PM
	struct hif_runtime_pm_ctx rpm_ctx;
#endif

	void (*hif_ipci_get_soc_info)(struct hif_ipci_softc *sc,
				      struct device *dev);
#ifdef FEATURE_HAL_DELAYED_REG_WRITE
	uint32_t ep_awake_reset_fail;
	uint32_t prevent_l1_fail;
	uint32_t ep_awake_set_fail;
	bool prevent_l1;
#endif
#ifdef FORCE_WAKE
	struct hif_ipci_stats stats;
#endif
#ifdef HIF_CPU_PERF_AFFINE_MASK
	/* Stores the affinity hint mask for each CE IRQ */
	qdf_cpu_mask ce_irq_cpu_mask[CE_COUNT_MAX];
#endif
};

int hif_configure_irq(struct hif_softc *sc);

/*
 * There may be some pending tx frames during platform suspend.
 * Suspend operation should be delayed until those tx frames are
 * transferred from the host to target. This macro specifies how
 * long suspend thread has to sleep before checking pending tx
 * frame count.
 */
#define OL_ATH_TX_DRAIN_WAIT_DELAY     50       /* ms */

#ifdef FORCE_WAKE
/**
 * hif_print_ipci_stats() - Display HIF IPCI stats
 * @ipci_scn - HIF ipci handle
 *
 * Return: None
 */
void hif_print_ipci_stats(struct hif_ipci_softc *ipci_scn);
#else
static inline
void hif_print_ipci_stats(struct hif_ipci_softc *ipci_scn)
{
}
#endif /* FORCE_WAKE */

#endif /* __IATH_PCI_H__ */

/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include <osdep.h>
#include <ol_if_athvar.h>
#include <athdefs.h>
#include "osapi_linux.h"
#include "hif.h"
#include "cepci.h"

struct CE_state;
struct ol_softc;

/* An address (e.g. of a buffer) in Copy Engine space. */

#define HIF_MAX_TASKLET_NUM 11
struct hif_tasklet_entry {
	uint8_t id;        /* 0 - 9: maps to CE, 10: fw */
	void *hif_handler; /* struct hif_pci_softc */
};
struct hif_pci_softc {
	void __iomem *mem;      /* PCI address. */
	/* For efficiency, should be first in struct */

	struct device *dev;
	struct pci_dev *pdev;
	struct ol_softc *ol_sc;
	int num_msi_intrs;      /* number of MSI interrupts granted */
	/* 0 --> using legacy PCI line interrupts */
	struct tasklet_struct intr_tq;  /* tasklet */


	int irq;
	int irq_event;
	int cacheline_sz;
	u16 devid;
	cdf_dma_addr_t soc_pcie_bar0;
	struct hif_tasklet_entry tasklet_entries[HIF_MAX_TASKLET_NUM];
	bool pci_enabled;
};

bool hif_pci_targ_is_present(struct ol_softc *scn, void *__iomem *mem);
void icnss_dispatch_ce_irq(struct ol_softc *scn);
int hif_configure_irq(struct hif_pci_softc *sc);
void hif_pci_cancel_deferred_target_sleep(struct ol_softc *scn);

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

#define HIF_CE_DRAIN_WAIT_CNT          20
#endif /* __ATH_PCI_H__ */

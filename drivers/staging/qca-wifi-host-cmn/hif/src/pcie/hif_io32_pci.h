/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_IO32_PCI_H__
#define __HIF_IO32_PCI_H__

#ifdef HIF_PCI

#include "hif_main.h"
#include "regtable.h"
#include "ce_reg.h"
#include "qdf_atomic.h"
#include "if_pci.h"
/*
 * For maximum performance and no power management, set this to 1.
 * For power management at the cost of performance, set this to 0.
 */
#ifndef CONFIG_ATH_PCIE_MAX_PERF
#define CONFIG_ATH_PCIE_MAX_PERF 0
#endif

/*
 * For keeping the target awake till the driver is
 * loaded, set this to 1
 */
#ifndef CONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD
#define CONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD 1
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

irqreturn_t hif_fw_interrupt_handler(int irq, void *arg);
#endif /* HIF_PCI */
#endif /* __HIF_IO32_PCI_H__ */

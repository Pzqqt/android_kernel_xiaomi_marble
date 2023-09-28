/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_IO32_IPCI_H__
#define __HIF_IO32_IPCI_H__

#ifdef HIF_IPCI

#include "hif_main.h"
#include "regtable.h"
#include "ce_reg.h"
#include "qdf_atomic.h"
#include "if_ipci.h"
/*
 * For maximum performance and no power management, set this to 1.
 * For power management at the cost of performance, set this to 0.
 */
#ifndef CONFIG_ATH_IPCIE_MAX_PERF
#define CONFIG_ATH_IPCIE_MAX_PERF 0
#endif

/*
 * PCIE_ACCESS_LOG_NUM specifies the number of
 * read/write records to store
 */
#ifdef CONFIG_ATH_IPCIE_ACCESS_DEBUG
#define IPCIE_ACCESS_LOG_NUM 500
#endif

/* 64-bit MSI support */
#define CONFIG_IPCIE_64BIT_MSI 0

/* AXI gating when L1, L2 to reduce power consumption */
#define CONFIG_IPCIE_ENABLE_AXI_CLK_GATE 0

irqreturn_t hif_fw_interrupt_handler(int irq, void *arg);
#endif /* HIF_IPCI */
#endif /* __HIF_IO32_IPCI_H__ */

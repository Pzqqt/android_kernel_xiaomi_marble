/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: if_ahb.h
 *
 * h file for ahb specific implementations.
 */

#ifndef __IF_AHB_H
#define __IF_AHB_H

#define GCC_BASE 0x1800000
#define GCC_SIZE 0x60000
#define GCC_FEPLL_PLL_DIV 0x2f020
#define GCC_FEPLL_PLL_CLK_WIFI_0_SEL_MASK 0x00000300
#define GCC_FEPLL_PLL_CLK_WIFI_0_SEL_SHIFT 8
#define GCC_FEPLL_PLL_CLK_WIFI_1_SEL_MASK 0x00003000
#define GCC_FEPLL_PLL_CLK_WIFI_1_SEL_SHIFT 12


/* These registers are outsize Wifi space. */
/* TBD: Should we add these offsets as device tree properties? */
#define TCSR_BASE 0x1900000
#define TCSR_SIZE 0x80000
#define TCSR_WIFI0_GLB_CFG 0x49000
#define TCSR_WIFI1_GLB_CFG 0x49004
#define TCSR_WCSS0_HALTREQ 0x52000
#define TCSR_WCSS1_HALTREQ 0x52004
#define TCSR_WCSS0_HALTACK 0x52010
#define TCSR_WCSS1_HALTACK 0x52014
#define ATH_AHB_RESET_WAIT_MAX 10 /* Ms */

irqreturn_t hif_ahb_interrupt_handler(int irq, void *context);

#endif


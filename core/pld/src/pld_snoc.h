/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef __PLD_SNOC_H__
#define __PLD_SNOC_H__

#include "pld_internal.h"

#ifndef CONFIG_PLD_SNOC_ICNSS
static inline int pld_snoc_register_driver(void)
{
	return 0;
}

static inline void pld_snoc_unregister_driver(void)
{
	return;
}
static inline int pld_snoc_wlan_enable(struct pld_wlan_enable_cfg *config,
		    enum pld_driver_mode mode, const char *host_version)
{
	return 0;
}
static inline int pld_snoc_wlan_disable(enum pld_driver_mode mode)
{
	return 0;
}
static inline int pld_snoc_ce_request_irq(unsigned int ce_id,
		       irqreturn_t (*handler)(int, void *),
		       unsigned long flags, const char *name, void *ctx)
{
	return 0;
}
static inline int pld_snoc_ce_free_irq(unsigned int ce_id, void *ctx)
{
	return 0;
}
static inline void pld_snoc_enable_irq(unsigned int ce_id)
{
	return;
}
static inline void pld_snoc_disable_irq(unsigned int ce_id)
{
	return;
}
static inline int pld_snoc_get_soc_info(struct pld_soc_info *info)
{
	return 0;
}
static inline int pld_snoc_get_ce_id(int irq)
{
	return 0;
}
static inline int pld_snoc_power_on(struct device *dev)
{
	return 0;
}
static inline int pld_snoc_power_off(struct device *dev)
{
	return 0;
}
static inline int pld_snoc_get_irq(int ce_id)
{
	return 0;
}
#else
int pld_snoc_register_driver(void);
void pld_snoc_unregister_driver(void);
int pld_snoc_wlan_enable(struct pld_wlan_enable_cfg *config,
			 enum pld_driver_mode mode, const char *host_version);
int pld_snoc_wlan_disable(enum pld_driver_mode mode);
int pld_snoc_get_soc_info(struct pld_soc_info *info);
int pld_snoc_ce_request_irq(unsigned int ce_id,
			    irqreturn_t (*handler)(int, void *),
			    unsigned long flags, const char *name, void *ctx);
int pld_snoc_ce_free_irq(unsigned int ce_id, void *ctx);
void pld_snoc_enable_irq(unsigned int ce_id);
void pld_snoc_disable_irq(unsigned int ce_id);
int pld_snoc_get_ce_id(int irq);
int pld_snoc_power_on(struct device *dev);
int pld_snoc_power_off(struct device *dev);
int pld_snoc_get_irq(int ce_id);
#endif

#endif

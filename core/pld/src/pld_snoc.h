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

#ifdef CONFIG_PLD_SNOC_ICNSS
#include <soc/qcom/icnss.h>
#endif
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
static inline int pld_snoc_set_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 ch_count)
{
	return 0;
}
static inline int pld_snoc_get_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 *ch_count,
						   u16 buf_len)
{
	return 0;
}
static inline int pld_snoc_wlan_set_dfs_nol(const void *info, u16 info_len)
{
	return 0;
}
static inline int pld_snoc_wlan_get_dfs_nol(void *info, u16 info_len)
{
	return 0;
}
static inline int pld_snoc_athdiag_read(struct device *dev, uint32_t offset,
					uint32_t memtype, uint32_t datalen,
					uint8_t *output)
{
	return 0;
}
static inline int pld_snoc_athdiag_write(struct device *dev, uint32_t offset,
					 uint32_t memtype, uint32_t datalen,
					 uint8_t *input)
{
	return 0;
}
static inline void *pld_snoc_smmu_get_mapping(struct device *dev)
{
	return NULL;
}
static inline int pld_snoc_smmu_map(struct device *dev, phys_addr_t paddr,
				    uint32_t *iova_addr, size_t size)
{
	return 0;
}
static inline
unsigned int pld_snoc_socinfo_get_serial_number(struct device *dev)
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
static inline int pld_snoc_ce_request_irq(unsigned int ce_id,
					  irqreturn_t (*handler)(int, void *),
					  unsigned long flags,
					  const char *name, void *ctx)
{
	return icnss_ce_request_irq(ce_id, handler, flags, name, ctx);
}
static inline int pld_snoc_ce_free_irq(unsigned int ce_id, void *ctx)
{
	return icnss_ce_free_irq(ce_id, ctx);
}
static inline void pld_snoc_enable_irq(unsigned int ce_id)
{
	icnss_enable_irq(ce_id);
}
static inline void pld_snoc_disable_irq(unsigned int ce_id)
{
	icnss_disable_irq(ce_id);
}
static inline int pld_snoc_get_ce_id(int irq)
{
	return icnss_get_ce_id(irq);
}
static inline int pld_snoc_power_on(struct device *dev)
{
	return icnss_power_on(dev);
}
static inline int pld_snoc_power_off(struct device *dev)
{
	return icnss_power_off(dev);
}
static inline int pld_snoc_get_irq(int ce_id)
{
	return icnss_get_irq(ce_id);
}
static inline int pld_snoc_set_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 ch_count)
{
	return icnss_set_wlan_unsafe_channel(unsafe_ch_list, ch_count);
}
static inline int pld_snoc_get_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 *ch_count,
						   u16 buf_len)
{
	return icnss_get_wlan_unsafe_channel(unsafe_ch_list, ch_count,
					     buf_len);
}
static inline int pld_snoc_wlan_set_dfs_nol(const void *info, u16 info_len)
{
	return icnss_wlan_set_dfs_nol(info, info_len);
}
static inline int pld_snoc_wlan_get_dfs_nol(void *info, u16 info_len)
{
	return icnss_wlan_get_dfs_nol(info, info_len);
}
static inline int pld_snoc_athdiag_read(struct device *dev, uint32_t offset,
					uint32_t memtype, uint32_t datalen,
					uint8_t *output)
{
	return icnss_athdiag_read(dev, offset, memtype, datalen, output);
}
static inline int pld_snoc_athdiag_write(struct device *dev, uint32_t offset,
					 uint32_t memtype, uint32_t datalen,
					 uint8_t *input)
{
	return icnss_athdiag_write(dev, offset, memtype, datalen, input);
}
static inline void *pld_snoc_smmu_get_mapping(struct device *dev)
{
	return icnss_smmu_get_mapping(dev);
}
static inline int pld_snoc_smmu_map(struct device *dev, phys_addr_t paddr,
				    uint32_t *iova_addr, size_t size)
{
	return icnss_smmu_map(dev, paddr, iova_addr, size);
}
static inline
unsigned int pld_snoc_socinfo_get_serial_number(struct device *dev)
{
	return icnss_socinfo_get_serial_number(dev);
}
#endif
#endif

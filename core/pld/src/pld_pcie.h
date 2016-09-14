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

#ifndef __PLD_PCIE_H__
#define __PLD_PCIE_H__

#ifdef CONFIG_PLD_PCIE_CNSS
#include <net/cnss.h>
#endif
#include "pld_internal.h"

#ifndef CONFIG_PCI
static inline int pld_pcie_register_driver(void)
{
	return 0;
}

static inline void pld_pcie_unregister_driver(void)
{
	return;
}

static inline int pld_pcie_get_ce_id(int irq)
{
	return 0;
}
#else
int pld_pcie_register_driver(void);
void pld_pcie_unregister_driver(void);
int pld_pcie_get_ce_id(int irq);
#endif

#if (!defined(CONFIG_PLD_PCIE_CNSS)) || (!defined(QCA_WIFI_3_0_ADRASTEA))
static inline int pld_pcie_wlan_enable(struct pld_wlan_enable_cfg *config,
		    enum pld_driver_mode mode, const char *host_version)
{
	return 0;
}
static inline int pld_pcie_wlan_disable(enum pld_driver_mode mode)
{
	return 0;
}
static inline int pld_pcie_set_fw_debug_mode(bool enablefwlog)
{
	return 0;
}
static inline void pld_pcie_intr_notify_q6(void)
{
	return;
}
#else
int pld_pcie_wlan_enable(struct pld_wlan_enable_cfg *config,
			 enum pld_driver_mode mode, const char *host_version);
int pld_pcie_wlan_disable(enum pld_driver_mode mode);
static inline int pld_pcie_set_fw_debug_mode(bool enablefwlog)
{
	return cnss_set_fw_debug_mode(mode);
}
static inline void pld_pcie_intr_notify_q6(void)
{
	cnss_intr_notify_q6();
}
#endif

#if (!defined(CONFIG_PLD_PCIE_CNSS)) || (!defined(CONFIG_CNSS_SECURE_FW))
static inline int pld_pcie_get_sha_hash(const u8 *data,
					u32 data_len, u8 *hash_idx, u8 *out)
{
	return 0;
}
static inline void *pld_pcie_get_fw_ptr(void)
{
	return NULL;
}
#else
static inline int pld_pcie_get_sha_hash(const u8 *data,
					u32 data_len, u8 *hash_idx, u8 *out)
{
	return cnss_get_sha_hash(data, data_len, hash_idx, out);
}
static inline void *pld_pcie_get_fw_ptr(void)
{
	return cnss_get_fw_ptr();
}
#endif

#if (!defined(CONFIG_PLD_PCIE_CNSS)) || (!defined(CONFIG_PCI_MSM))
static inline int pld_pcie_wlan_pm_control(bool vote)
{
	return 0;
}
#else
static inline int pld_pcie_wlan_pm_control(bool vote)
{
	return cnss_wlan_pm_control(vote);
}
#endif

#ifndef CONFIG_PLD_PCIE_CNSS
static inline int
pld_pcie_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				 u32 target_type, u32 target_version)
{
	pld_get_default_fw_files(pfw_files);
	return 0;
}
static inline void pld_pcie_link_down(void)
{
	return;
}
static inline int pld_pcie_shadow_control(bool enable)
{
	return 0;
}
static inline int
pld_pcie_get_codeswap_struct(struct pld_codeswap_codeseg_info *swap_seg)
{
	return 0;
}
static inline int
pld_pcie_set_wlan_unsafe_channel(u16 *unsafe_ch_list, u16 ch_count)
{
	return 0;
}
static inline int
pld_pcie_get_wlan_unsafe_channel(u16 *unsafe_ch_list,
				 u16 *ch_count, u16 buf_len)
{
	return 0;
}
static inline int pld_pcie_wlan_set_dfs_nol(void *info, u16 info_len)
{
	return 0;
}
static inline int pld_pcie_wlan_get_dfs_nol(void *info, u16 info_len)
{
	return 0;
}
static void pld_pcie_schedule_recovery_work(void)
{
	return;
}
static inline void *pld_pcie_get_virt_ramdump_mem(unsigned long *size)
{
	return NULL;
}
static inline void pld_pcie_device_crashed(void)
{
	return;
}
static inline void pld_pcie_device_self_recovery(void)
{
	return;
}
static inline void pld_pcie_request_pm_qos(u32 qos_val)
{
	return;
}
static inline void pld_pcie_remove_pm_qos(void)
{
	return;
}
static inline int pld_pcie_request_bus_bandwidth(int bandwidth)
{
	return 0;
}
static inline int pld_pcie_get_platform_cap(struct pld_platform_cap *cap)
{
	return 0;
}
static inline void pld_pcie_set_driver_status(enum pld_driver_status status)
{
	return;
}
static inline int pld_pcie_auto_suspend(void)
{
	return 0;
}
static inline int pld_pcie_auto_resume(void)
{
	return 0;
}
static void pld_pcie_lock_pm_sem(void)
{
	return;
}
static void pld_pcie_release_pm_sem(void)
{
	return;
}
static inline int pld_pcie_power_on(struct device *dev)
{
	return 0;
}
static inline int pld_pcie_power_off(struct device *dev)
{
	return 0;
}
#else
int pld_pcie_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version);
int pld_pcie_get_codeswap_struct(struct pld_codeswap_codeseg_info *swap_seg);
int pld_pcie_get_platform_cap(struct pld_platform_cap *cap);
void pld_pcie_set_driver_status(enum pld_driver_status status);
static inline void pld_pcie_link_down(void)
{
	cnss_wlan_pci_link_down();
}
static inline int pld_pcie_shadow_control(bool enable)
{
	return 0;
}
static inline int pld_pcie_set_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 ch_count)
{
	return cnss_set_wlan_unsafe_channel(unsafe_ch_list, ch_count);
}
static inline int pld_pcie_get_wlan_unsafe_channel(u16 *unsafe_ch_list,
						   u16 *ch_count, u16 buf_len)
{
	return cnss_get_wlan_unsafe_channel(unsafe_ch_list, ch_count, buf_len);
}
static inline int pld_pcie_wlan_set_dfs_nol(void *info, u16 info_len)
{
	return cnss_wlan_set_dfs_nol(info, info_len);
}
static inline int pld_pcie_wlan_get_dfs_nol(void *info, u16 info_len)
{
	return cnss_wlan_get_dfs_nol(info, info_len);
}
static inline void pld_pcie_schedule_recovery_work(void)
{
	cnss_schedule_recovery_work();
}
static inline void *pld_pcie_get_virt_ramdump_mem(unsigned long *size)
{
	return cnss_get_virt_ramdump_mem(size);
}
static inline void pld_pcie_device_crashed(void)
{
	cnss_device_crashed();
}
static inline void pld_pcie_device_self_recovery(void)
{
	cnss_device_self_recovery();
}
static inline void pld_pcie_request_pm_qos(u32 qos_val)
{
	cnss_request_pm_qos(qos_val);
}
static inline void pld_pcie_remove_pm_qos(void)
{
	cnss_remove_pm_qos();
}
static inline int pld_pcie_request_bus_bandwidth(int bandwidth)
{
	return cnss_request_bus_bandwidth(bandwidth);
}
static inline int pld_pcie_auto_suspend(void)
{
	return cnss_auto_suspend();
}
static inline int pld_pcie_auto_resume(void)
{
	return cnss_auto_resume();
}
static inline void pld_pcie_lock_pm_sem(void)
{
	cnss_lock_pm_sem();
}
static inline void pld_pcie_release_pm_sem(void)
{
	cnss_release_pm_sem();
}
static inline int pld_pcie_power_on(struct device *dev)
{
	return cnss_power_up(dev);
}
static inline int pld_pcie_power_off(struct device *dev)
{
	return cnss_power_down(dev);
}
#endif

#endif

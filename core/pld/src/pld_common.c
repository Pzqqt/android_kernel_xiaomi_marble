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

#define pr_fmt(fmt) "wlan_pld:%s:%d:: " fmt, __func__, __LINE__

#include <linux/printk.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/pm.h>

#if defined(CONFIG_PLD_PCIE_CNSS) || defined(CONFIG_PLD_SDIO_CNSS)
#include <net/cnss.h>
#endif
#ifdef CONFIG_PLD_SNOC_ICNSS
#include <soc/qcom/icnss.h>
#endif

#include "pld_pcie.h"
#include "pld_snoc.h"
#include "pld_sdio.h"

#define PLD_PCIE_REGISTERED BIT(0)
#define PLD_SNOC_REGISTERED BIT(1)
#define PLD_SDIO_REGISTERED BIT(2)

static struct pld_context *pld_ctx;

/**
 * pld_init() - Initialize PLD module
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_init(void)
{
	struct pld_context *pld_context;

	pld_context = kzalloc(sizeof(*pld_context), GFP_KERNEL);
	if (!pld_context)
		return -ENOMEM;

	spin_lock_init(&pld_context->pld_lock);

	INIT_LIST_HEAD(&pld_context->dev_list);

	pld_ctx = pld_context;

	return 0;
}

/**
 * pld_deinit() - Uninitialize PLD module
 *
 * Return: void
 */
void pld_deinit(void)
{
	struct dev_node *dev_node;
	struct pld_context *pld_context;
	unsigned long flags;

	pld_context = pld_ctx;
	if (!pld_context) {
		pld_ctx = NULL;
		return;
	}

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	while (!list_empty(&pld_context->dev_list)) {
		dev_node = list_first_entry(&pld_context->dev_list,
					    struct dev_node, list);
		list_del(&dev_node->list);
		kfree(dev_node);
	}
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);

	kfree(pld_context);

	pld_ctx = NULL;
}

/**
 * pld_get_global_context() - Get global context of PLD
 *
 * Return: PLD global context
 */
struct pld_context *pld_get_global_context(void)
{
	return pld_ctx;
}

/**
 * pld_add_dev() - Add dev node to global context
 * @pld_context: PLD global context
 * @dev: device
 * @type: Bus type
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_add_dev(struct pld_context *pld_context,
		struct device *dev, enum pld_bus_type type)
{
	unsigned long flags;
	struct dev_node *dev_node;

	dev_node = kzalloc(sizeof(*dev_node), GFP_KERNEL);
	if (dev_node == NULL)
		return -ENOMEM;

	dev_node->dev = dev;
	dev_node->bus_type = type;

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	list_add_tail(&dev_node->list, &pld_context->dev_list);
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);

	return 0;
}

/**
 * pld_del_dev() - Delete dev node from global context
 * @pld_context: PLD global context
 * @dev: device
 *
 * Return: void
 */
void pld_del_dev(struct pld_context *pld_context,
		 struct device *dev)
{
	unsigned long flags;
	struct dev_node *dev_node, *tmp;

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	list_for_each_entry_safe(dev_node, tmp, &pld_context->dev_list, list) {
		if (dev_node->dev == dev) {
			list_del(&dev_node->list);
			kfree(dev_node);
		}
	}
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);
}

/**
 * pld_get_bus_type() - Bus type of the device
 * @dev: device
 *
 * Return: PLD bus type
 */
enum pld_bus_type pld_get_bus_type(struct device *dev)
{
	struct pld_context *pld_context;
	struct dev_node *dev_node;
	unsigned long flags;

	pld_context = pld_get_global_context();

	if (dev == NULL || pld_context == NULL) {
		pr_err("Invalid info: dev %p, context %p\n",
		       dev, pld_context);
		return PLD_BUS_TYPE_NONE;
	}

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	list_for_each_entry(dev_node, &pld_context->dev_list, list) {
		if (dev_node->dev == dev) {
			spin_unlock_irqrestore(&pld_context->pld_lock, flags);
			return dev_node->bus_type;
		}
	}
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);

	return PLD_BUS_TYPE_NONE;
}

/**
 * pld_register_driver() - Register driver to kernel
 * @ops: Callback functions that will be registered to kernel
 *
 * This function should be called when other modules want to
 * register platform driver callback functions to kernel. The
 * probe() is expected to be called after registration if the
 * device is online.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_register_driver(struct pld_driver_ops *ops)
{
	int ret = 0;
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();

	if (pld_context == NULL) {
		pr_err("global context is NULL\n");
		ret = -ENODEV;
		goto out;
	}

	if (pld_context->ops) {
		pr_err("driver already registered\n");
		ret = -EEXIST;
		goto out;
	}

	if (!ops || !ops->probe || !ops->remove ||
	    !ops->suspend || !ops->resume) {
		pr_err("Required callback functions are missing\n");
		ret = -EINVAL;
		goto out;
	}

	pld_context->ops = ops;

	ret = pld_pcie_register_driver();
	if (ret) {
		pr_err("Fail to register pcie driver\n");
		goto fail_pcie;
	}
	pld_context->pld_driver_state |= PLD_PCIE_REGISTERED;

	ret = pld_snoc_register_driver();
	if (ret) {
		pr_err("Fail to register snoc driver\n");
		goto fail_snoc;
	}
	pld_context->pld_driver_state |= PLD_SNOC_REGISTERED;

	ret = pld_sdio_register_driver();
	if (ret) {
		pr_err("Fail to register sdio driver\n");
		goto fail_sdio;
	}
	pld_context->pld_driver_state |= PLD_SDIO_REGISTERED;

	return ret;

fail_sdio:
	pld_snoc_unregister_driver();
fail_snoc:
	pld_pcie_unregister_driver();
fail_pcie:
	pld_context->pld_driver_state = 0;
	pld_context->ops = NULL;
out:
	return ret;
}

/**
 * pld_unregister_driver() - Unregister driver to kernel
 *
 * This function should be called when other modules want to
 * unregister callback functions from kernel. The remove() is
 * expected to be called after registration.
 *
 * Return: void
 */
void pld_unregister_driver(void)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();

	if (pld_context == NULL) {
		pr_err("global context is NULL\n");
		return;
	}

	if (pld_context->ops == NULL) {
		pr_err("driver not registered\n");
		return;
	}

	pld_pcie_unregister_driver();
	pld_snoc_unregister_driver();
	pld_sdio_unregister_driver();

	pld_context->pld_driver_state = 0;

	pld_context->ops = NULL;
}

/**
 * pld_wlan_enable() - Enable WLAN
 * @dev: device
 * @config: WLAN configuration data
 * @mode: WLAN mode
 * @host_version: host software version
 *
 * This function enables WLAN FW. It passed WLAN configuration data,
 * WLAN mode and host software version to FW.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_wlan_enable(struct device *dev, struct pld_wlan_enable_cfg *config,
		    enum pld_driver_mode mode, const char *host_version)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_wlan_enable(config, mode, host_version);
		break;
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_wlan_enable(config, mode, host_version);
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_wlan_disable() - Disable WLAN
 * @dev: device
 * @mode: WLAN mode
 *
 * This function disables WLAN FW. It passes WLAN mode to FW.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_wlan_disable(struct device *dev, enum pld_driver_mode mode)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_wlan_disable(mode);
		break;
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_wlan_disable(mode);
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_set_fw_debug_mode() - Set FW debug mode
 * @dev: device
 * @enablefwlog: 0 for QXDM, 1 for WMI
 *
 * Switch Fw debug mode between DIAG logging and WMI logging.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_set_fw_debug_mode(struct device *dev, bool enablefwlog)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_set_fw_debug_mode(enablefwlog);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_default_fw_files() - Get default FW file names
 * @pfw_files: buffer for FW file names
 *
 * Return default FW file names to the buffer.
 *
 * Return: void
 */
void pld_get_default_fw_files(struct pld_fw_files *pfw_files)
{
	memset(pfw_files, 0, sizeof(*pfw_files));

	strlcpy(pfw_files->image_file, PLD_IMAGE_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->board_data, PLD_BOARD_DATA_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->otp_data, PLD_OTP_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->utf_file, PLD_UTF_FIRMWARE_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->utf_board_data, PLD_BOARD_DATA_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->epping_file, PLD_EPPING_FILE,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->setup_file, PLD_SETUP_FILE,
		PLD_MAX_FILE_NAME);
}

/**
 * pld_get_fw_files_for_target() - Get FW file names
 * @dev: device
 * @pfw_files: buffer for FW file names
 * @target_type: target type
 * @target_version: target version
 *
 * Return target specific FW file names to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_fw_files_for_target(struct device *dev,
				struct pld_fw_files *pfw_files,
				u32 target_type, u32 target_version)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_fw_files_for_target(pfw_files,
				       target_type, target_version);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		ret = pld_sdio_get_fw_files_for_target(pfw_files,
				       target_type, target_version);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_fw_image() - Get FW image descriptor
 * @dev: device
 * @image_desc_info: buffer for image descriptor
 *
 * Return FW image descriptor to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_fw_image(struct device *dev,
		     struct pld_image_desc_info *image_desc_info)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_fw_image(image_desc_info);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_is_pci_link_down() - Notification for pci link down event
 * @dev: device
 *
 * Notify platform that pci link is down.
 *
 * Return: void
 */
void pld_is_pci_link_down(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_link_down();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_shadow_control() - Control pci shadow registers
 * @dev: device
 * @enable: 0 for disable, 1 for enable
 *
 * This function is for suspend/resume. It can control if we
 * use pci shadow registers (for saving config space) or not.
 * During suspend we disable it to avoid config space corruption.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_shadow_control(struct device *dev, bool enable)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_shadow_control(enable);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_codeswap_struct() - Get codeswap structure
 * @dev: device
 * @swap_seg: buffer to codeswap information
 *
 * Return codeswap structure information to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_codeswap_struct(struct device *dev,
			    struct pld_codeswap_codeseg_info *swap_seg)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_codeswap_struct(swap_seg);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_set_wlan_unsafe_channel() - Set unsafe channel
 * @dev: device
 * @unsafe_ch_list: unsafe channel list
 * @ch_count: number of channel
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_set_wlan_unsafe_channel(struct device *dev,
				u16 *unsafe_ch_list, u16 ch_count)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_set_wlan_unsafe_channel(unsafe_ch_list,
						       ch_count);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do get unsafe channel via cnss sdio API */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_wlan_unsafe_channel() - Get unsafe channel
 * @dev: device
 * @unsafe_ch_list: buffer to unsafe channel list
 * @ch_count: number of channel
 * @buf_len: buffer length
 *
 * Return WLAN unsafe channel to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_wlan_unsafe_channel(struct device *dev, u16 *unsafe_ch_list,
				u16 *ch_count, u16 buf_len)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_wlan_unsafe_channel(unsafe_ch_list,
						       ch_count, buf_len);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do get unsafe channel via cnss sdio API */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_wlan_set_dfs_nol() - Set DFS info
 * @dev: device
 * @info: DFS info
 * @info_len: info length
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_wlan_set_dfs_nol(struct device *dev, void *info, u16 info_len)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_wlan_set_dfs_nol(info, info_len);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do get nol via cnss sdio API */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_wlan_get_dfs_nol() - Get DFS info
 * @dev: device
 * @info: buffer to DFS info
 * @info_len: info length
 *
 * Return DFS info to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_wlan_get_dfs_nol(struct device *dev, void *info, u16 info_len)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_wlan_get_dfs_nol(info, info_len);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_schedule_recovery_work() - Schedule recovery work
 * @dev: device
 *
 * Schedule a system self recovery work.
 *
 * Return: void
 */
void pld_schedule_recovery_work(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_schedule_recovery_work();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_wlan_pm_control() - WLAN PM control on PCIE
 * @dev: device
 * @vote: 0 for enable PCIE PC, 1 for disable PCIE PC
 *
 * This is for PCIE power collaps control during suspend/resume.
 * When PCIE power collaps is disabled, WLAN FW can access memory
 * through PCIE when system is suspended.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_wlan_pm_control(struct device *dev, bool vote)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_wlan_pm_control(vote);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_virt_ramdump_mem() - Get virtual ramdump memory
 * @dev: device
 * @size: buffer to virtual memory size
 *
 * Return: virtual ramdump memory address
 */
void *pld_get_virt_ramdump_mem(struct device *dev, unsigned long *size)
{
	void *mem = NULL;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		mem = pld_pcie_get_virt_ramdump_mem(size);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}

	return mem;
}

/**
 * pld_device_crashed() - Notification for device crash event
 * @dev: device
 *
 * Notify subsystem a device crashed event. A subsystem restart
 * is expected to happen after calling this function.
 *
 * Return: void
 */
void pld_device_crashed(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_device_crashed();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_device_self_recovery() - Device self recovery
 * @dev: device
 *
 * Return: void
 */
void pld_device_self_recovery(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_device_self_recovery();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_intr_notify_q6() - Notify Q6 FW interrupts
 * @dev: device
 *
 * Notify Q6 that a FW interrupt is triggered.
 *
 * Return: void
 */
void pld_intr_notify_q6(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_intr_notify_q6();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_request_pm_qos() - Request system PM
 * @dev: device
 * @qos_val: request value
 *
 * It votes for the value of aggregate QoS expectations.
 *
 * Return: void
 */
void pld_request_pm_qos(struct device *dev, u32 qos_val)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_request_pm_qos(qos_val);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do Add call cns API */
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_remove_pm_qos() - Remove system PM
 * @dev: device
 *
 * Remove the vote request for Qos expectations.
 *
 * Return: void
 */
void pld_remove_pm_qos(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_remove_pm_qos();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do Add call cns API */
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_request_bus_bandwidth() - Request bus bandwidth
 * @dev: device
 * @bandwidth: bus bandwidth
 *
 * Votes for HIGH/MEDIUM/LOW bus bandwidth.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_request_bus_bandwidth(struct device *dev, int bandwidth)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_request_bus_bandwidth(bandwidth);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		/* To do Add call cns API */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_platform_cap() - Get platform capabilities
 * @dev: device
 * @cap: buffer to the capabilities
 *
 * Return capabilities to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_platform_cap(struct device *dev, struct pld_platform_cap *cap)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_platform_cap(cap);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_set_driver_status() - Set driver status
 * @dev: device
 * @status: driver status
 *
 * Return: void
 */
void pld_set_driver_status(struct device *dev, enum pld_driver_status status)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_set_driver_status(status);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_get_sha_hash() - Get sha hash number
 * @dev: device
 * @data: input data
 * @data_len: data length
 * @hash_idx: hash index
 * @out:  output buffer
 *
 * Return computed hash to the out buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_sha_hash(struct device *dev, const u8 *data,
		     u32 data_len, u8 *hash_idx, u8 *out)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_sha_hash(data, data_len,
					    hash_idx, out);
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_fw_ptr() - Get secure FW memory address
 * @dev: device
 *
 * Return: secure memory address
 */
void *pld_get_fw_ptr(struct device *dev)
{
	void *ptr = NULL;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ptr = pld_pcie_get_fw_ptr();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}

	return ptr;
}

/**
 * pld_auto_suspend() - Auto suspend
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_auto_suspend(struct device *dev)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_auto_suspend();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_auto_resume() - Auto resume
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_auto_resume(struct device *dev)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_auto_resume();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_ce_request_irq() - Register IRQ for CE
 * @dev: device
 * @ce_id: CE number
 * @handler: IRQ callback function
 * @flags: IRQ flags
 * @name: IRQ name
 * @ctx: IRQ context
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_ce_request_irq(struct device *dev, unsigned int ce_id,
		       irqreturn_t (*handler)(int, void *),
		       unsigned long flags, const char *name, void *ctx)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_ce_request_irq(ce_id, handler, flags, name, ctx);
		break;
	case PLD_BUS_TYPE_PCIE:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_ce_free_irq() - Free IRQ for CE
 * @dev: device
 * @ce_id: CE number
 * @ctx: IRQ context
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_ce_free_irq(struct device *dev, unsigned int ce_id, void *ctx)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_ce_free_irq(ce_id, ctx);
		break;
	case PLD_BUS_TYPE_PCIE:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_enable_irq() - Enable IRQ for CE
 * @dev: device
 * @ce_id: CE number
 *
 * Return: void
 */
void pld_enable_irq(struct device *dev, unsigned int ce_id)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		pld_snoc_enable_irq(ce_id);
		break;
	case PLD_BUS_TYPE_PCIE:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_disable_irq() - Disable IRQ for CE
 * @dev: device
 * @ce_id: CE number
 *
 * Return: void
 */
void pld_disable_irq(struct device *dev, unsigned int ce_id)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		pld_snoc_disable_irq(ce_id);
		break;
	case PLD_BUS_TYPE_PCIE:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_get_soc_info() - Get SOC information
 * @dev: device
 * @info: buffer to SOC information
 *
 * Return SOC info to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_get_soc_info(struct device *dev, struct pld_soc_info *info)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_get_soc_info(info);
		break;
	case PLD_BUS_TYPE_PCIE:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_ce_id() - Get CE number for the provided IRQ
 * @dev: device
 * @irq: IRQ number
 *
 * Return: CE number
 */
int pld_get_ce_id(struct device *dev, int irq)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_get_ce_id(irq);
		break;
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_get_ce_id(irq);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_get_irq() - Get IRQ number for given CE ID
 * @dev: device
 * @ce_id: CE ID
 *
 * Return: IRQ number
 */
int pld_get_irq(struct device *dev, int ce_id)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_get_irq(ce_id);
		break;
	case PLD_BUS_TYPE_PCIE:
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * pld_lock_pm_sem() - Lock PM semaphore
 * @dev: device
 *
 * Return: void
 */
void pld_lock_pm_sem(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_lock_pm_sem();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_release_pm_sem() - Release PM semaphore
 * @dev: device
 *
 * Return: void
 */
void pld_release_pm_sem(struct device *dev)
{
	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		pld_pcie_release_pm_sem();
		break;
	case PLD_BUS_TYPE_SNOC:
		break;
	case PLD_BUS_TYPE_SDIO:
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}
}

/**
 * pld_power_on() - Power on WLAN hardware
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_power_on(struct device *dev)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_power_on(dev);
		break;
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_power_on(dev);
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}

	return ret;
}

/**
 * pld_power_off() - Power off WLAN hardware
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_power_off(struct device *dev)
{
	int ret = 0;

	switch (pld_get_bus_type(dev)) {
	case PLD_BUS_TYPE_PCIE:
		ret = pld_pcie_power_on(dev);
		break;
	case PLD_BUS_TYPE_SNOC:
		ret = pld_snoc_power_off(dev);
		break;
	default:
		pr_err("Invalid device type\n");
		break;
	}

	return ret;
}

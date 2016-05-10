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

#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/pci.h>
#include <linux/list.h>
#include <linux/slab.h>

#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif

#include "pld_common.h"
#include "pld_internal.h"

#ifdef CONFIG_PCI

#ifdef QCA_WIFI_3_0_ADRASTEA
#define CE_COUNT_MAX 12
#else
#define CE_COUNT_MAX 8
#endif

/**
 * pld_pcie_probe() - Probe function for PCIE platform driver
 * @pdev: PCIE device
 * @id: PCIE device ID table
 *
 * The probe function will be called when PCIE device provided
 * in the ID table is detected.
 *
 * Return: int
 */
static int pld_pcie_probe(struct pci_dev *pdev,
			  const struct pci_device_id *id)
{
	struct pld_context *pld_context;
	unsigned long flags;
	struct dev_node *dev_node;
	int ret = 0;

	pld_context = pld_get_global_context();
	if (!pld_context) {
		ret = -ENODEV;
		goto out;
	}

	dev_node = kzalloc(sizeof(*dev_node), GFP_KERNEL);
	if (dev_node == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	dev_node->dev = &pdev->dev;
	dev_node->bus_type = PLD_BUS_TYPE_PCIE;

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	list_add_tail(&dev_node->list, &pld_context->dev_list);
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);

	return pld_context->ops->probe(&pdev->dev,
		       PLD_BUS_TYPE_PCIE, pdev, (void *)id);

out:
	return ret;
}


/**
 * pld_pcie_remove() - Remove function for PCIE device
 * @pdev: PCIE device
 *
 * The remove function will be called when PCIE device is disconnected
 *
 * Return: void
 */
static void pld_pcie_remove(struct pci_dev *pdev)
{
	struct pld_context *pld_context;
	unsigned long flags;
	struct dev_node *dev_node, *tmp;

	pld_context = pld_get_global_context();

	if (!pld_context)
		return;

	spin_lock_irqsave(&pld_context->pld_lock, flags);
	list_for_each_entry_safe(dev_node, tmp, &pld_context->dev_list, list) {
		if (dev_node->dev == &pdev->dev) {
			list_del(&dev_node->list);
			kfree(dev_node);
		}
	}
	spin_unlock_irqrestore(&pld_context->pld_lock, flags);

	pld_context->ops->remove(&pdev->dev, PLD_BUS_TYPE_PCIE);
}

#ifdef CONFIG_CNSS
/**
 * pld_pcie_reinit() - SSR re-initialize function for PCIE device
 * @pdev: PCIE device
 * @id: PCIE device ID
 *
 * During subsystem restart(SSR), this function will be called to
 * re-initialize PCIE device.
 *
 * Return: int
 */
static int pld_pcie_reinit(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->reinit)
		return pld_context->ops->reinit(&pdev->dev,
				PLD_BUS_TYPE_PCIE, pdev, (void *)id);

	return -ENODEV;
}

/**
 * pld_pcie_shutdown() - SSR shutdown function for PCIE device
 * @pdev: PCIE device
 *
 * During SSR, this function will be called to shutdown PCIE device.
 *
 * Return: void
 */
static void pld_pcie_shutdown(struct pci_dev *pdev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->shutdown)
		pld_context->ops->shutdown(&pdev->dev, PLD_BUS_TYPE_PCIE);
}

/**
 * pld_pcie_crash_shutdown() - Crash shutdown function for PCIE device
 * @pdev: PCIE device
 *
 * This function will be called when a crash is detected, it will shutdown
 * the PCIE device.
 *
 * Return: void
 */
static void pld_pcie_crash_shutdown(struct pci_dev *pdev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->crash_shutdown)
		pld_context->ops->crash_shutdown(&pdev->dev, PLD_BUS_TYPE_PCIE);
}

/**
 * pld_pcie_notify_handler() - Modem state notification callback function
 * @pdev: PCIE device
 * @state: modem power state
 *
 * This function will be called when there's a modem power state change.
 *
 * Return: void
 */
static void pld_pcie_notify_handler(struct pci_dev *pdev, int state)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->modem_status)
		pld_context->ops->modem_status(&pdev->dev,
					       PLD_BUS_TYPE_PCIE, state);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * pld_pcie_runtime_suspend() - PM runtime suspend
 * @pdev: PCIE device
 *
 * PM runtime suspend callback function.
 *
 * Return: int
 */
static int pld_pcie_runtime_suspend(struct pci_dev *pdev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->runtime_suspend)
		return pld_context->ops->runtime_suspend(&pdev->dev,
							 PLD_BUS_TYPE_PCIE);

	return -ENODEV;
}

/**
 * pld_pcie_runtime_resume() - PM runtime resume
 * @pdev: PCIE device
 *
 * PM runtime resume callback function.
 *
 * Return: int
 */
static int pld_pcie_runtime_resume(struct pci_dev *pdev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->runtime_resume)
		return pld_context->ops->runtime_resume(&pdev->dev,
							PLD_BUS_TYPE_PCIE);

	return -ENODEV;
}
#endif
#endif

/**
 * pld_pcie_suspend() - Suspend callback function for power management
 * @pdev: PCIE device
 * @state: power state
 *
 * This function is to suspend the PCIE device when power management is
 * enabled.
 *
 * Return: void
 */
static int pld_pcie_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	return pld_context->ops->suspend(&pdev->dev,
					 PLD_BUS_TYPE_PCIE, state);
}

/**
 * pld_pcie_resume() - Resume callback function for power management
 * @pdev: PCIE device
 *
 * This function is to resume the PCIE device when power management is
 * enabled.
 *
 * Return: void
 */
static int pld_pcie_resume(struct pci_dev *pdev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	return pld_context->ops->resume(&pdev->dev, PLD_BUS_TYPE_PCIE);
}

static struct pci_device_id pld_pcie_id_table[] = {
	{ 0x168c, 0x003c, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x003e, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x0041, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0xabcd, PCI_ANY_ID, PCI_ANY_ID },
	{ 0x168c, 0x7021, PCI_ANY_ID, PCI_ANY_ID },
	{ 0 }
};

#ifdef CONFIG_CNSS
#ifdef FEATURE_RUNTIME_PM
struct cnss_wlan_runtime_ops runtime_pm_ops = {
	.runtime_suspend = pld_pcie_runtime_suspend,
	.runtime_resume = pld_pcie_runtime_resume,
};
#endif

struct cnss_wlan_driver pld_pcie_ops = {
	.name       = "pld_pcie",
	.id_table   = pld_pcie_id_table,
	.probe      = pld_pcie_probe,
	.remove     = pld_pcie_remove,
	.reinit     = pld_pcie_reinit,
	.shutdown   = pld_pcie_shutdown,
	.crash_shutdown = pld_pcie_crash_shutdown,
	.modem_status   = pld_pcie_notify_handler,
#ifdef CONFIG_PM
	.suspend    = pld_pcie_suspend,
	.resume     = pld_pcie_resume,
#endif
#ifdef FEATURE_RUNTIME_PM
	.runtime_ops = &runtime_pm_ops,
#endif
};

/**
 * pld_pcie_register_driver() - Register PCIE device callback functions
 *
 * Return: int
 */
int pld_pcie_register_driver(void)
{
	return cnss_wlan_register_driver(&pld_pcie_ops);
}

/**
 * pld_pcie_unregister_driver() - Unregister PCIE device callback functions
 *
 * Return: void
 */
void pld_pcie_unregister_driver(void)
{
	cnss_wlan_unregister_driver(&pld_pcie_ops);
}
#else
struct pci_driver pld_pcie_ops = {
	.name       = "pld_pcie",
	.id_table   = pld_pcie_id_table,
	.probe      = pld_pcie_probe,
	.remove     = pld_pcie_remove,
#ifdef CONFIG_PM
	.suspend    = pld_pcie_suspend,
	.resume     = pld_pcie_resume,
#endif
};

int pld_pcie_register_driver(void)
{
	return pci_register_driver(&pld_pcie_ops);
}

void pld_pcie_unregister_driver(void)
{
	pci_unregister_driver(&pld_pcie_ops);
}
#endif

/**
 * pld_pcie_get_ce_id() - Get CE number for the provided IRQ
 * @irq: IRQ number
 *
 * Return: CE number
 */
int pld_pcie_get_ce_id(int irq)
{
	int ce_id = irq - 100;
	if (ce_id < CE_COUNT_MAX && ce_id >= 0)
		return ce_id;

	return -EINVAL;
}

#ifdef CONFIG_CNSS
#ifdef QCA_WIFI_3_0_ADRASTEA
/**
 * pld_pcie_wlan_enable() - Enable WLAN
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
int pld_pcie_wlan_enable(struct pld_wlan_enable_cfg *config,
			 enum pld_driver_mode mode, const char *host_version)
{
	struct cnss_wlan_enable_cfg cfg;
	enum cnss_driver_mode cnss_mode;

	cfg.num_ce_tgt_cfg = config->num_ce_tgt_cfg;
	cfg.ce_tgt_cfg = (struct cnss_ce_tgt_pipe_cfg *)
		config->ce_tgt_cfg;
	cfg.num_ce_svc_pipe_cfg = config->num_ce_svc_pipe_cfg;
	cfg.ce_svc_cfg = (struct cnss_ce_svc_pipe_cfg *)
		config->ce_svc_cfg;
	cfg.num_shadow_reg_cfg = config->num_shadow_reg_cfg;
	cfg.shadow_reg_cfg = (struct cnss_shadow_reg_cfg *)
		config->shadow_reg_cfg;

	switch (mode) {
	case PLD_FTM:
		cnss_mode = CNSS_FTM;
		break;
	case PLD_EPPING:
		cnss_mode = CNSS_EPPING;
		break;
	default:
		cnss_mode = CNSS_MISSION;
		break;
	}
	return cnss_wlan_enable(&cfg, cnss_mode, host_version);
}

/**
 * pld_pcie_wlan_disable() - Disable WLAN
 * @mode: WLAN mode
 *
 * This function disables WLAN FW. It passes WLAN mode to FW.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_wlan_disable(enum pld_driver_mode mode)
{
	return cnss_wlan_disable(CNSS_OFF);
}

/**
 * pld_pcie_set_fw_debug_mode() - Set FW debug mode
 * @mode: 0 for QXDM, 1 for WMI
 *
 * Switch Fw debug mode between DIAG logging and WMI logging.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_set_fw_debug_mode(bool mode)
{
	return cnss_set_fw_debug_mode(mode);
}
#endif

/**
 * pld_pcie_get_fw_files_for_target() - Get FW file names
 * @pfw_files: buffer for FW file names
 * @target_type: target type
 * @target_version: target version
 *
 * Return target specific FW file names to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version)
{
	int ret = 0;
	struct cnss_fw_files cnss_fw_files;

	if (pfw_files == NULL)
		return -ENODEV;

	memset(pfw_files, 0, sizeof(*pfw_files));

	ret = cnss_get_fw_files_for_target(&cnss_fw_files,
					   target_type, target_version);
	if (0 != ret)
		return ret;

	strlcpy(pfw_files->image_file, cnss_fw_files.image_file,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->board_data, cnss_fw_files.board_data,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->otp_data, cnss_fw_files.otp_data,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->utf_file, cnss_fw_files.utf_file,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->utf_board_data, cnss_fw_files.utf_board_data,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->epping_file, cnss_fw_files.epping_file,
		PLD_MAX_FILE_NAME);
	strlcpy(pfw_files->evicted_data, cnss_fw_files.evicted_data,
		PLD_MAX_FILE_NAME);

	return 0;
}

/**
 * pld_pcie_get_fw_image() - Get FW image descriptor
 * @image_desc_info: buffer for image descriptor
 *
 * Return FW image descriptor to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_get_fw_image(struct pld_image_desc_info *image_desc_info)
{
	int ret = 0;
	struct image_desc_info cnss_image_desc_info;

	if (image_desc_info == NULL)
		return -ENODEV;

	ret = cnss_get_fw_image(&cnss_image_desc_info);
	if (0 != ret)
		return ret;

	memcpy(image_desc_info, &cnss_image_desc_info,
	       sizeof(*image_desc_info));
	return 0;
}

/**
 * pld_pcie_get_codeswap_struct() - Get codeswap structure
 * @swap_seg: buffer to codeswap information
 *
 * Return codeswap structure information to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_get_codeswap_struct(struct pld_codeswap_codeseg_info *swap_seg)
{
	int ret = 0;
	struct codeswap_codeseg_info cnss_swap_seg;

	if (swap_seg == NULL)
		return -ENODEV;

	ret = cnss_get_codeswap_struct(&cnss_swap_seg);
	if (0 != ret)
		return ret;

	memcpy(swap_seg, &cnss_swap_seg, sizeof(*swap_seg));
	return 0;
}

/**
 * pld_pcie_get_platform_cap() - Get platform capabilities
 * @cap: buffer to the capabilities
 *
 * Return capabilities to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_pcie_get_platform_cap(struct pld_platform_cap *cap)
{
	int ret = 0;
	struct cnss_platform_cap cnss_cap;

	if (cap == NULL)
		return -ENODEV;

	ret = cnss_get_platform_cap(&cnss_cap);
	if (0 != ret)
		return ret;

	memcpy(cap, &cnss_cap, sizeof(*cap));
	return 0;
}

/**
 * pld_pcie_set_driver_status() - Set driver status
 * @status: driver status
 *
 * Return: void
 */
void pld_pcie_set_driver_status(enum pld_driver_status status)
{
	enum cnss_driver_status cnss_status;

	switch (status) {
	case PLD_UNINITIALIZED:
		cnss_status = CNSS_UNINITIALIZED;
		break;
	case PLD_INITIALIZED:
		cnss_status = CNSS_INITIALIZED;
		break;
	default:
		cnss_status = CNSS_LOAD_UNLOAD;
		break;
	}
	cnss_set_driver_status(cnss_status);
}
#endif

#endif

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
#include <linux/list.h>
#include <linux/slab.h>

#ifdef CONFIG_PLD_SNOC_ICNSS
#include <soc/qcom/icnss.h>
#endif

#include "pld_internal.h"

#ifdef CONFIG_PLD_SNOC_ICNSS
/**
 * pld_snoc_probe() - Probe function for platform driver
 * @dev: device
 *
 * The probe function will be called when platform device
 * is detected.
 *
 * Return: int
 */
static int pld_snoc_probe(struct device *dev)
{
	struct pld_context *pld_context;
	int ret = 0;

	pld_context = pld_get_global_context();
	if (!pld_context) {
		ret = -ENODEV;
		goto out;
	}

	ret = pld_add_dev(pld_context, dev, PLD_BUS_TYPE_SNOC);
	if (ret)
		goto out;

	return pld_context->ops->probe(dev, PLD_BUS_TYPE_SNOC,
				       NULL, NULL);

out:
	return ret;
}

/**
 * pld_snoc_remove() - Remove function for platform device
 * @dev: device
 *
 * The remove function will be called when platform device
 * is disconnected
 *
 * Return: void
 */
static void pld_snoc_remove(struct device *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();

	if (!pld_context)
		return;

	pld_context->ops->remove(dev, PLD_BUS_TYPE_SNOC);

	pld_del_dev(pld_context, dev);
}

/**
 * pld_snoc_reinit() - SSR re-initialize function for platform device
 * @dev: device
 *
 * During subsystem restart(SSR), this function will be called to
 * re-initialize platform device.
 *
 * Return: int
 */
static int pld_snoc_reinit(struct device *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->reinit)
		return pld_context->ops->reinit(dev, PLD_BUS_TYPE_SNOC,
						NULL, NULL);

	return -ENODEV;
}

/**
 * pld_snoc_shutdown() - SSR shutdown function for platform device
 * @dev: device
 *
 * During SSR, this function will be called to shutdown platform device.
 *
 * Return: void
 */
static void pld_snoc_shutdown(struct device *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->shutdown)
		pld_context->ops->shutdown(dev, PLD_BUS_TYPE_SNOC);
}

/**
 * pld_snoc_crash_shutdown() - Crash shutdown function for platform device
 * @dev: device
 *
 * This function will be called when a crash is detected, it will shutdown
 * platform device.
 *
 * Return: void
 */
static void pld_snoc_crash_shutdown(void *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	if (pld_context->ops->crash_shutdown)
		pld_context->ops->crash_shutdown(dev, PLD_BUS_TYPE_SNOC);
}

/**
 * pld_snoc_suspend() - Suspend callback function for power management
 * @dev: device
 * @state: power state
 *
 * This function is to suspend the platform device when power management
 * is enabled.
 *
 * Return: void
 */
static int pld_snoc_suspend(struct device *dev, pm_message_t state)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	return pld_context->ops->suspend(dev, PLD_BUS_TYPE_SNOC, state);
}

/**
 * pld_snoc_resume() - Resume callback function for power management
 * @pdev: device
 *
 * This function is to resume the platform device when power management
 * is enabled.
 *
 * Return: void
 */
static int pld_snoc_resume(struct device *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	return pld_context->ops->resume(dev, PLD_BUS_TYPE_SNOC);
}

struct icnss_driver_ops pld_snoc_ops = {
	.name       = "pld_snoc",
	.probe      = pld_snoc_probe,
	.remove     = pld_snoc_remove,
	.shutdown   = pld_snoc_shutdown,
	.reinit     = pld_snoc_reinit,
	.crash_shutdown = pld_snoc_crash_shutdown,
	.suspend    = pld_snoc_suspend,
	.resume     = pld_snoc_resume,
};

/**
 * pld_snoc_register_driver() - Register platform device callback functions
 *
 * Return: int
 */
int pld_snoc_register_driver(void)
{
	return icnss_register_driver(&pld_snoc_ops);
}

/**
 * pld_snoc_unregister_driver() - Unregister platform device callback functions
 *
 * Return: void
 */
void pld_snoc_unregister_driver(void)
{
	icnss_unregister_driver(&pld_snoc_ops);
}

/**
 * pld_snoc_wlan_enable() - Enable WLAN
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
int pld_snoc_wlan_enable(struct pld_wlan_enable_cfg *config,
			 enum pld_driver_mode mode, const char *host_version)
{
	struct icnss_wlan_enable_cfg cfg;
	enum icnss_driver_mode icnss_mode;

	cfg.num_ce_tgt_cfg = config->num_ce_tgt_cfg;
	cfg.ce_tgt_cfg = (struct ce_tgt_pipe_cfg *)
		config->ce_tgt_cfg;
	cfg.num_ce_svc_pipe_cfg = config->num_ce_svc_pipe_cfg;
	cfg.ce_svc_cfg = (struct ce_svc_pipe_cfg *)
		config->ce_svc_cfg;
	cfg.num_shadow_reg_cfg = config->num_shadow_reg_cfg;
	cfg.shadow_reg_cfg = (struct icnss_shadow_reg_cfg *)
		config->shadow_reg_cfg;

	switch (mode) {
	case PLD_FTM:
		icnss_mode = ICNSS_FTM;
		break;
	case PLD_EPPING:
		icnss_mode = ICNSS_EPPING;
		break;
	default:
		icnss_mode = ICNSS_MISSION;
		break;
	}
	return icnss_wlan_enable(&cfg, icnss_mode, host_version);
}

/**
 * pld_snoc_wlan_disable() - Disable WLAN
 * @mode: WLAN mode
 *
 * This function disables WLAN FW. It passes WLAN mode to FW.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_wlan_disable(enum pld_driver_mode mode)
{
	return icnss_wlan_disable(ICNSS_OFF);
}

/**
 * pld_snoc_get_soc_info() - Get SOC information
 * @info: buffer to SOC information
 *
 * Return SOC info to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_get_soc_info(struct pld_soc_info *info)
{
	int ret = 0;
	struct icnss_soc_info icnss_info;

	if (info == NULL)
		return -ENODEV;

	ret = icnss_get_soc_info(&icnss_info);
	if (0 != ret)
		return ret;

	memcpy(info, &icnss_info, sizeof(*info));
	return 0;
}

/**
 * pld_snoc_ce_request_irq() - Register IRQ for CE
 * @ce_id: CE number
 * @handler: IRQ callback function
 * @flags: IRQ flags
 * @name: IRQ name
 * @ctx: IRQ context
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_ce_request_irq(unsigned int ce_id,
			    irqreturn_t (*handler)(int, void *),
			    unsigned long flags, const char *name, void *ctx)
{
	return icnss_ce_request_irq(ce_id, handler, flags, name, ctx);
}

/**
 * pld_snoc_ce_free_irq() - Free IRQ for CE
 * @ce_id: CE number
 * @ctx: IRQ context
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_ce_free_irq(unsigned int ce_id, void *ctx)
{
	return icnss_ce_free_irq(ce_id, ctx);
}

/**
 * pld_snoc_enable_irq() - Enable IRQ for CE
 * @ce_id: CE number
 *
 * Return: void
 */
void pld_snoc_enable_irq(unsigned int ce_id)
{
	icnss_enable_irq(ce_id);
}

/**
 * pld_snoc_disable_irq() - Disable IRQ for CE
 * @ce_id: CE number
 *
 * Return: void
 */
void pld_snoc_disable_irq(unsigned int ce_id)
{
	icnss_disable_irq(ce_id);
}

/**
 * pld_snoc_get_ce_id() - Get CE number for the provided IRQ
 * @irq: IRQ number
 *
 * Return: CE number
 */
int pld_snoc_get_ce_id(int irq)
{
	return icnss_get_ce_id(irq);
}

/**
 * pld_snoc_power_on() - Power on WLAN hardware
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_power_on(struct device *dev)
{
	return icnss_power_on(dev);
}

/**
 * pld_snoc_power_off() - Power off WLAN hardware
 * @dev: device
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_snoc_power_off(struct device *dev)
{
	return icnss_power_off(dev);
}

/**
 * pld_snoc_get_irq() - Get IRQ number for given CE ID
 * @ce_id: CE ID
 *
 * Return: IRQ number
 */
int pld_snoc_get_irq(int ce_id)
{
	return icnss_get_irq(ce_id);
}

#endif

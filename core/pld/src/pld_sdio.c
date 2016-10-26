/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mmc/sdio_func.h>
#include <linux/list.h>
#include <linux/slab.h>

#ifdef CONFIG_PLD_SDIO_CNSS
#include <net/cnss.h>
#endif

#include "pld_common.h"
#include "pld_internal.h"

#ifdef CONFIG_SDIO
/* SDIO manufacturer ID and Codes */
#define MANUFACTURER_ID_AR6320_BASE        0x500
#define MANUFACTURER_ID_QCA9377_BASE       0x700
#define MANUFACTURER_CODE                  0x271

/**
 * pld_sdio_probe() - Probe function for SDIO platform driver
 * sdio_func: pointer to sdio device function
 * @id: SDIO device ID table
 *
 * The probe function will be called when SDIO device provided
 * in the ID table is detected.
 *
 * Return: int
 */
static int pld_sdio_probe(struct sdio_func *sdio_func,
			  const struct sdio_device_id *id)
{
	struct pld_context *pld_context;
	struct device *dev = &sdio_func->dev;
	int ret = 0;

	pld_context = pld_get_global_context();
	if (!pld_context) {
		ret = -ENODEV;
		goto out;
	}

	ret = pld_add_dev(pld_context, dev, PLD_BUS_TYPE_SDIO);
	if (ret)
		goto out;

	return pld_context->ops->probe(dev, PLD_BUS_TYPE_SDIO,
		       sdio_func, (void *)id);

out:
	return ret;
}


/**
 * pld_sdio_remove() - Remove function for SDIO device
 * @sdio_func: pointer to sdio device function
 *
 * The remove function will be called when SDIO device is disconnected
 *
 * Return: void
 */
static void pld_sdio_remove(struct sdio_func *sdio_func)
{
	struct pld_context *pld_context;
	struct device *dev = &sdio_func->dev;

	pld_context = pld_get_global_context();

	if (!pld_context)
		return;

	pld_context->ops->remove(dev, PLD_BUS_TYPE_SDIO);
	pld_del_dev(pld_context, dev);
}

#ifdef CONFIG_PLD_SDIO_CNSS
/**
 * pld_sdio_reinit() - SSR re-initialize function for SDIO device
 * @sdio_func: pointer to sdio device function
 * @id: SDIO device ID
 *
 * During subsystem restart(SSR), this function will be called to
 * re-initialize SDIO device.
 *
 * Return: int
 */
static int pld_sdio_reinit(struct sdio_func *sdio_func,
			  const struct sdio_device_id *id)
{
	struct pld_context *pld_context;
	struct device *dev = &sdio_func->dev;

	pld_context = pld_get_global_context();
	if (pld_context->ops->reinit)
		return pld_context->ops->reinit(dev, PLD_BUS_TYPE_SDIO,
		       sdio_func, (void *)id);

	return -ENODEV;
}

/**
 * pld_sdio_shutdown() - SSR shutdown function for SDIO device
 * @sdio_func: pointer to sdio device function
 *
 * During SSR, this function will be called to shutdown SDIO device.
 *
 * Return: void
 */
static void pld_sdio_shutdown(struct sdio_func *sdio_func)
{
	struct pld_context *pld_context;
	struct device *dev = &sdio_func->dev;

	pld_context = pld_get_global_context();
	if (pld_context->ops->shutdown)
		pld_context->ops->shutdown(dev, PLD_BUS_TYPE_SDIO);
}

/**
 * pld_sdio_crash_shutdown() - Crash shutdown function for SDIO device
 * @sdio_func: pointer to sdio device function
 *
 * This function will be called when a crash is detected, it will shutdown
 * the SDIO device.
 *
 * Return: void
 */
static void pld_sdio_crash_shutdown(struct sdio_func *sdio_func)
{
	struct pld_context *pld_context;
	struct device *dev = &sdio_func->dev;

	pld_context = pld_get_global_context();
	if (pld_context->ops->crash_shutdown)
		pld_context->ops->crash_shutdown(dev, PLD_BUS_TYPE_SDIO);
}

#endif

#ifdef CONFIG_PM
/**
 * pld_sdio_suspend() - Suspend callback function for power management
 * @dev: SDIO device
 *
 * This function is to suspend the SDIO device when power management is
 * enabled.
 *
 * Return: void
 */
static int pld_sdio_suspend(struct device *dev)
{
	struct pld_context *pld_context;
	pm_message_t state = { .event = PM_EVENT_SUSPEND };

	pld_context = pld_get_global_context();
	return pld_context->ops->suspend(dev,
					 PLD_BUS_TYPE_SDIO, state);
}

/**
 * pld_sdio_resume() - Resume callback function for power management
 * @dev: SDIO device
 *
 * This function is to resume the SDIO device when power management is
 * enabled.
 *
 * Return: void
 */
static int pld_sdio_resume(struct device *dev)
{
	struct pld_context *pld_context;

	pld_context = pld_get_global_context();
	return pld_context->ops->resume(dev, PLD_BUS_TYPE_SDIO);
}
#endif

static struct sdio_device_id pld_sdio_id_table[] = {
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6320_BASE | 0xF))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9377_BASE | 0xF))},
	{},
};

#ifdef CONFIG_PLD_SDIO_CNSS
struct cnss_sdio_wlan_driver pld_sdio_ops = {
	.name       = "pld_sdio",
	.id_table   = pld_sdio_id_table,
	.probe      = pld_sdio_probe,
	.remove     = pld_sdio_remove,
	.reinit     = pld_sdio_reinit,
	.shutdown   = pld_sdio_shutdown,
	.crash_shutdown = pld_sdio_crash_shutdown,
#ifdef CONFIG_PM
	.suspend    = pld_sdio_suspend,
	.resume     = pld_sdio_resume,
#endif
};

/**
 * pld_sdio_register_driver() - Register SDIO device callback functions
 *
 * Return: int
 */
int pld_sdio_register_driver(void)
{
	return cnss_sdio_wlan_register_driver(&pld_sdio_ops);
}

/**
 * pld_sdio_unregister_driver() - Unregister SDIO device callback functions
 *
 * Return: void
 */
void pld_sdio_unregister_driver(void)
{
	cnss_sdio_wlan_unregister_driver(&pld_sdio_ops);
}
#else
struct sdio_driver pld_sdio_ops = {
	.name       = "pld_sdio",
	.id_table   = pld_sdio_id_table,
	.probe      = pld_sdio_probe,
	.remove     = pld_sdio_remove,
#ifdef CONFIG_PM
	.suspend    = pld_sdio_suspend,
	.resume     = pld_sdio_resume,
#endif
};

int pld_sdio_register_driver(void)
{
	return sdio_register_driver(&pld_sdio_ops);
}

void pld_sdio_unregister_driver(void)
{
	sdio_unregister_driver(&pld_sdio_ops);
}
#endif

#ifdef CONFIG_PLD_SDIO_CNSS
/**
 * pld_sdio_get_fw_files_for_target() - Get FW file names
 * @pfw_files: buffer for FW file names
 * @target_type: target type
 * @target_version: target version
 *
 * Return target specific FW file names to the buffer.
 *
 * Return: 0 for success
 *         Non zero failure code for errors
 */
int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
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

	return ret;
}
#else
#ifdef CONFIG_TUFELLO_DUAL_FW_SUPPORT
static inline void get_qca9377_fw_files(struct pld_fw_files *pfw_files,
					u32 size)
{
		memcpy(pfw_files, &fw_files_default, sizeof(*pfw_files));
}
#else
static inline void get_qca9377_fw_files(struct pld_fw_files *pfw_files,
					u32 size)
{
		memcpy(pfw_files, &fw_files_qca6174_fw_3_0, sizeof(*pfw_files));
}
#endif

int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version)
{
	if (!pfw_files)
		return -ENODEV;

	switch (target_version) {
	case PLD_AR6320_REV1_VERSION:
	case PLD_AR6320_REV1_1_VERSION:
		memcpy(pfw_files, &fw_files_qca6174_fw_1_1, sizeof(*pfw_files));
		break;
	case PLD_AR6320_REV1_3_VERSION:
		memcpy(pfw_files, &fw_files_qca6174_fw_1_3, sizeof(*pfw_files));
		break;
	case PLD_AR6320_REV2_1_VERSION:
		memcpy(pfw_files, &fw_files_qca6174_fw_2_0, sizeof(*pfw_files));
		break;
	case PLD_AR6320_REV3_VERSION:
	case PLD_AR6320_REV3_2_VERSION:
		memcpy(pfw_files, &fw_files_qca6174_fw_3_0, sizeof(*pfw_files));
		break;
	case PLD_QCA9377_REV1_1_VERSION:
		get_qca9377_fw_files(pfw_files, sizeof(*pfw_files));
		break;
	default:
		memcpy(pfw_files, &fw_files_default, sizeof(*pfw_files));
		pr_err("%s version mismatch 0x%X ",
				__func__, target_version);
		break;
	}

	return 0;
}
#endif
#endif

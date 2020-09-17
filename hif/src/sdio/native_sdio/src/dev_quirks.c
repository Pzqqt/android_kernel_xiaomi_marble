/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/module.h>
#include <qdf_atomic.h>
#include <cds_utils.h>
#include <qdf_timer.h>
#include <cds_api.h>
#include <qdf_time.h>
#include "hif_sdio_dev.h"
#include "if_sdio.h"
#include "regtable_sdio.h"
#include "wma_api.h"
#include "hif_internal.h"
#include <transfer/transfer.h>

/* QUIRK PARAMETERS */
unsigned int writecccr1;
module_param(writecccr1, uint, 0644);
unsigned int writecccr1value;
module_param(writecccr1value, uint, 0644);

unsigned int writecccr2;
module_param(writecccr2, uint, 0644);
unsigned int writecccr2value;
module_param(writecccr2value, uint, 0644);

unsigned int writecccr3;
module_param(writecccr3, uint, 0644);
unsigned int writecccr3value;
module_param(writecccr3value, uint, 0644);

unsigned int writecccr4;
module_param(writecccr4, uint, 0644);
unsigned int writecccr4value;
module_param(writecccr4value, uint, 0644);

unsigned int modstrength;
module_param(modstrength, uint, 0644);
MODULE_PARM_DESC(modstrength, "Adjust internal driver strength");

unsigned int mmcbuswidth;
/* PERM:S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH */
module_param(mmcbuswidth, uint, 0644);
MODULE_PARM_DESC(mmcbuswidth,
		 "Set MMC driver Bus Width: 1-1Bit, 4-4Bit, 8-8Bit");

unsigned int mmcclock;
module_param(mmcclock, uint, 0644);
MODULE_PARM_DESC(mmcclock, "Set MMC driver Clock value");

#ifdef CONFIG_X86
unsigned int asyncintdelay = 2;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,	"Delay clock count for async interrupt, 2 is default, valid values are 1 and 2");
#else
unsigned int asyncintdelay;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,	"Delay clock count for async interrupt, 0 is default, valid values are 1 and 2");
#endif

unsigned int brokenirq;
module_param(brokenirq, uint, 0644);
MODULE_PARM_DESC(brokenirq,
		 "Set as 1 to use polling method instead of interrupt mode");

#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
/**
 * hif_sdio_force_drive_strength() - Set SDIO drive strength
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * This function forces the driver strength of the SDIO
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
void hif_sdio_quirk_force_drive_strength(struct hif_softc *ol_sc,
					 struct sdio_func *func)
{
	int err = 0;
	unsigned char value = 0;
	uint32_t mask = 0, addr = SDIO_CCCR_DRIVE_STRENGTH;

	err = func0_cmd52_read_byte(func->card, addr, &value);
	if (err) {
		hif_err("read driver strength 0x%02X fail %d", addr, err);
		return;
	}

	mask = (SDIO_DRIVE_DTSx_MASK << SDIO_DRIVE_DTSx_SHIFT);
	value = (value & ~mask) | SDIO_DTSx_SET_TYPE_D;
	err = func0_cmd52_write_byte(func->card, addr, value);
	if (err) {
		hif_err("Write driver strength 0x%02X to 0x%02X failed: %d",
			(uint32_t)value, addr, err);
		return;
	}

	value = 0;
	addr = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR;
	err = func0_cmd52_read_byte(func->card,	addr, &value);
	if (err) {
		hif_err("Read CCCR 0x%02X failed: %d", addr, err);
		return;
	}

	mask = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK;
	value = (value & ~mask) | CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A |
		CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C |
		CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D;
	err = func0_cmd52_write_byte(func->card, addr, value);
	if (err)
		hif_err("Write CCCR 0x%02X to 0x%02X failed: %d",
			addr, value, err);
}

/**
 * hif_sdio_quirk_async_intr() - Set asynchronous interrupt settings
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter asyncintdelay
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
int hif_sdio_quirk_async_intr(struct hif_softc *ol_sc, struct sdio_func *func)
{
	uint8_t data;
	uint16_t manfid;
	int set_async_irq = 0, ret = 0;
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	switch (manfid) {
	case MANUFACTURER_ID_AR6003_BASE:
		set_async_irq = 1;
		ret =
		func0_cmd52_write_byte(func->card,
				       CCCR_SDIO_IRQ_MODE_REG_AR6003,
				       SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6003);
		if (ret)
			return ret;
		break;
	case MANUFACTURER_ID_AR6320_BASE:
	case MANUFACTURER_ID_QCA9377_BASE:
	case MANUFACTURER_ID_QCA9379_BASE:
		set_async_irq = 1;
		ret = func0_cmd52_read_byte(func->card,
					    CCCR_SDIO_IRQ_MODE_REG_AR6320,
					    &data);
		if (ret)
			return ret;

		data |= SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
		ret = func0_cmd52_write_byte(func->card,
					     CCCR_SDIO_IRQ_MODE_REG_AR6320,
					     data);
		if (ret)
			return ret;
		break;
	}

	if (asyncintdelay) {
		/* Set CCCR 0xF0[7:6] to increase async interrupt delay clock
		 * to fix interrupt missing issue on dell 8460p
		 */

		ret = func0_cmd52_read_byte(func->card,
					    CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					    &data);
		if (ret)
			return ret;

		data = (data & ~CCCR_SDIO_ASYNC_INT_DELAY_MASK) |
			((asyncintdelay << CCCR_SDIO_ASYNC_INT_DELAY_LSB) &
			 CCCR_SDIO_ASYNC_INT_DELAY_MASK);

		ret = func0_cmd52_write_byte(func->card,
					     CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					     data);
		if (ret)
			return ret;
	}

	return ret;
}
#else
/**
 * hif_sdio_force_drive_strength() - Set SDIO drive strength
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * This function forces the driver strength of the SDIO
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
void hif_sdio_quirk_force_drive_strength(struct hif_softc *ol_sc,
					 struct sdio_func *func)
{
}

/**
 * hif_sdio_quirk_async_intr() - Set asynchronous interrupt settings
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter asyncintdelay
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
int hif_sdio_quirk_async_intr(struct hif_softc *ol_sc, struct sdio_func *func)
{
	return 0;
}
#endif

/**
 * hif_sdio_quirk_write_cccr() - write a desired CCCR register
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter writecccr
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
void hif_sdio_quirk_write_cccr(struct hif_softc *ol_sc, struct sdio_func *func)
{
	int32_t err;

	if (writecccr1) {
		err = func0_cmd52_write_byte(func->card, writecccr1,
					     writecccr1value);
		if (err)
			hif_err("Write CCCR 0x%02X to 0x%02X failed: %d",
				(unsigned int)writecccr1,
				(unsigned int)writecccr1value,
				err);
		else
			HIF_INFO("%s Write CCCR 0x%02X to 0x%02X OK\n",
				 __func__,
				 (unsigned int)writecccr1,
				 writecccr1value);
	}

	if (writecccr2) {
		err = func0_cmd52_write_byte(func->card, writecccr2,
					     writecccr2value);
		if (err)
			hif_err("Write CCCR 0x%02X to 0x%02X failed: %d",
				(unsigned int)writecccr2,
				(unsigned int)writecccr2value,
				err);
		else
			HIF_INFO("%s Write CCCR 0x%02X to 0x%02X OK\n",
				 __func__,
				 (unsigned int)writecccr2,
				 (unsigned int)writecccr2value);
	}
	if (writecccr3) {
		err = func0_cmd52_write_byte(func->card, writecccr3,
					     writecccr3value);
		if (err)
			hif_err("Write CCCR 0x%02X to 0x%02X failed: %d",
				(unsigned int)writecccr3,
				(unsigned int)writecccr3value,
				err);
		else
			HIF_INFO("%s Write CCCR 0x%02X to 0x%02X OK\n",
				 __func__,
				 (unsigned int)writecccr3,
				 (unsigned int)writecccr3value);
	}
	if (writecccr4) {
		err = func0_cmd52_write_byte(func->card, writecccr4,
					     writecccr4value);
		if (err)
			hif_err("Write CCCR 0x%02X to 0x%02X failed: %d",
				(unsigned int)writecccr4,
				(unsigned int)writecccr4value,
				err);
		else
			HIF_INFO("%s Write CCCR 0x%02X to 0x%02X OK\n",
				 __func__,
				 (unsigned int)writecccr4,
				 (unsigned int)writecccr4value);
	}
}

/**
 * hif_sdio_quirk_mod_strength() - write a desired CCCR register
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter writecccr
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
int hif_sdio_quirk_mod_strength(struct hif_softc *ol_sc, struct sdio_func *func)
{
	int ret = 0;
	uint32_t addr, value;
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);
	uint16_t  manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (!modstrength) /* TODO: Dont set this : scn is not popolated yet */
		return 0;

	if (!scn) {
		hif_err("scn is null");
		return -1;
	}

	if (!scn->hostdef) {
		hif_err("scn->hostdef is null");
		return -1;
	}

	switch (manfid) {
	case MANUFACTURER_ID_QCN7605_BASE:
		break;
	default:
		addr = WINDOW_DATA_ADDRESS;
		value = 0x0FFF;
		ret = sdio_memcpy_toio(func, addr, &value, 4);
		if (ret) {
			hif_err("write 0x%x 0x%x error:%d", addr, value, ret);
			break;
		}
		HIF_INFO("%s: addr 0x%x val 0x%x", __func__, addr, value);

		addr = WINDOW_WRITE_ADDR_ADDRESS;
		value = 0x50F8;
		ret = sdio_memcpy_toio(func, addr, &value, 4);
		if (ret) {
			hif_err("write 0x%x 0x%x error:%d", addr, value, ret);
			break;
		}
		HIF_INFO("%s: addr 0x%x val 0x%x\n", __func__, addr, value);
		break;
	}

	return ret;
}

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
#ifdef SDIO_BUS_WIDTH_8BIT
static int hif_cmd52_write_byte_8bit(struct sdio_func *func)
{
	return func0_cmd52_write_byte(func->card, SDIO_CCCR_IF,
			SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_8BIT);
}
#else
static int hif_cmd52_write_byte_8bit(struct sdio_func *func)
{
	hif_err("8BIT Bus Width not supported");
	return QDF_STATUS_E_FAILURE;
}
#endif
#endif

/**
 * hif_sdio_set_bus_speed() - Set the sdio bus speed
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_sdio_set_bus_speed(struct hif_softc *ol_sc,
				  struct sdio_func *func)
{
	uint32_t clock, clock_set = 12500000;
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);
	uint16_t manfid;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return QDF_STATUS_SUCCESS;

	if (mmcclock > 0)
		clock_set = mmcclock;
#if (KERNEL_VERSION(3, 16, 0) > LINUX_VERSION_CODE)
	if (sdio_card_highspeed(func->card))
#else
		if (mmc_card_hs(func->card))
#endif
			clock = 50000000;
		else
			clock = func->card->cis.max_dtr;

	if (clock > device->host->f_max)
		clock = device->host->f_max;

	HIF_INFO("%s: Clock setting: (%d,%d)\n", __func__,
		 func->card->cis.max_dtr, device->host->f_max);

	/* Limit clock if specified */
	if (mmcclock > 0) {
		HIF_INFO("%s: Limit clock from %d to %d\n",
			 __func__, clock, clock_set);
		device->host->ios.clock = clock_set;
		device->host->ops->set_ios(device->host,
				&device->host->ios);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_sdio_set_bus_width() - Set the sdio bus width
 * @ol_sc: softc instance
 * @func: pointer to sdio_func
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_sdio_set_bus_width(struct hif_softc *ol_sc,
				  struct sdio_func *func)
{
	int ret = 0;
	uint16_t manfid;
	uint8_t data = 0;
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return status;

#if KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE
	if (mmcbuswidth == 0)
		return status;

	/* Set MMC Bus Width: 1-1Bit, 4-4Bit, 8-8Bit */
	if (mmcbuswidth == 1) {
		data = SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_1BIT;
		ret = func0_cmd52_write_byte(func->card,
					     SDIO_CCCR_IF,
					     data);
		if (ret)
			hif_err("Bus Width 0x%x failed %d", data, ret);
		device->host->ios.bus_width = MMC_BUS_WIDTH_1;
		device->host->ops->set_ios(device->host,
					   &device->host->ios);
	} else if (mmcbuswidth == 4 &&
		   (device->host->caps & MMC_CAP_4_BIT_DATA)) {
		data = SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_4BIT;
		ret = func0_cmd52_write_byte(func->card,
					     SDIO_CCCR_IF,
					     data);
		if (ret)
			hif_err("Bus Width 0x%x failed: %d", data, ret);
		device->host->ios.bus_width = MMC_BUS_WIDTH_4;
		device->host->ops->set_ios(device->host,
				&device->host->ios);
	} else if (mmcbuswidth == 8 &&
		   (device->host->caps & MMC_CAP_8_BIT_DATA)) {
		ret = hif_cmd52_write_byte_8bit(func);
		if (ret)
			hif_err("Bus Width 8 failed: %d", ret);
		device->host->ios.bus_width = MMC_BUS_WIDTH_8;
		device->host->ops->set_ios(device->host,
				&device->host->ios);
	} else {
		hif_err("Unsupported bus width %d", mmcbuswidth);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	status = qdf_status_from_os_return(ret);

out:
	HIF_INFO("%s: Bus with : %d\n",  __func__, mmcbuswidth);
#endif
	return status;
}


/**
 * hif_mask_interrupt() - Disable hif device irq
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: None.
 */
void hif_mask_interrupt(struct hif_sdio_dev *device)
{
	int ret;
	uint16_t manfid;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return;

	HIF_ENTER();

	/* Mask our function IRQ */
	sdio_claim_host(device->func);
	while (atomic_read(&device->irq_handling)) {
		sdio_release_host(device->func);
		schedule_timeout_interruptible(HZ / 10);
		sdio_claim_host(device->func);
	}
	ret = sdio_release_irq(device->func);
	sdio_release_host(device->func);
	if (ret)
		hif_err("Failed %d", ret);

	HIF_EXIT();
}

/**
 * hif_irq_handler() - hif-sdio interrupt handler
 * @func: pointer to sdio_func
 *
 * Return: None.
 */
static void hif_irq_handler(struct sdio_func *func)
{
	struct hif_sdio_dev *device = get_hif_device(NULL, func);
	atomic_set(&device->irq_handling, 1);
	/* release the host during intr so we can use
	 * it when we process cmds
	 */
	sdio_release_host(device->func);
	device->htc_callbacks.dsr_handler(device->htc_callbacks.context);
	sdio_claim_host(device->func);
	atomic_set(&device->irq_handling, 0);
}

/**
 * hif_un_mask_interrupt() - Re-enable hif device irq
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: None.
 */
void hif_un_mask_interrupt(struct hif_sdio_dev *device)
{
	int ret;
	uint16_t manfid;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return;

	HIF_ENTER();
	/*
	 * On HP Elitebook 8460P, interrupt mode is not stable
	 * in high throughput, so polling method should be used
	 * instead of interrupt mode.
	 */
	if (brokenirq) {
		HIF_INFO("%s: Using broken IRQ mode", __func__);
		device->func->card->host->caps &= ~MMC_CAP_SDIO_IRQ;
	}
	/* Register the IRQ Handler */
	sdio_claim_host(device->func);
	ret = sdio_claim_irq(device->func, hif_irq_handler);
	sdio_release_host(device->func);

	HIF_EXIT();
}

/**
 * hif_sdio_func_disable() - Handle device enabling as per device
 * @device: HIF device object
 * @func: function pointer
 *
 * Return success or failure
 */
QDF_STATUS hif_sdio_func_disable(struct hif_sdio_dev *device,
				 struct sdio_func *func,
				 bool reset)
{
	int ret = 0;
	uint16_t manfid;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return 0;

	/* Disable the card */
	sdio_claim_host(device->func);

	ret = sdio_disable_func(device->func);
	if (ret)
		status = QDF_STATUS_E_FAILURE;

	if (reset && status == QDF_STATUS_SUCCESS)
		ret = func0_cmd52_write_byte(device->func->card,
					     SDIO_CCCR_ABORT,
					     (1 << 3));

	if (ret) {
		status = QDF_STATUS_E_FAILURE;
		hif_err("reset failed: %d", ret);
	}

	sdio_release_host(device->func);

	return status;
}

/**
 * reinit_sdio() - re-initialize sdio bus
 * @device: pointer to hif device
 *
 * Return: 0 on success, error number otherwise.
 */
QDF_STATUS reinit_sdio(struct hif_sdio_dev *device)
{
	int32_t err = 0;
	struct mmc_host *host;
	struct mmc_card *card;
	struct sdio_func *func;
	uint8_t  cmd52_resp;
	uint32_t clock;
	uint16_t manfid;

	func = device->func;
	card = func->card;
	host = card->host;

	manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (manfid == MANUFACTURER_ID_QCN7605_BASE)
		return QDF_STATUS_SUCCESS;

	sdio_claim_host(func);

	do {
		/* Enable high speed */
		if (card->host->caps & MMC_CAP_SD_HIGHSPEED) {
			HIF_INFO_HI("%s: Set high speed mode", __func__);
			err = func0_cmd52_read_byte(card, SDIO_CCCR_SPEED,
						    &cmd52_resp);
			if (err) {
				hif_err("CCCR speed set failed: %d", err);
				sdio_card_state(card);
				/* no need to break */
			} else {
				err = func0_cmd52_write_byte(card,
							     SDIO_CCCR_SPEED,
							     (cmd52_resp |
							      SDIO_SPEED_EHS));
				if (err) {
					hif_err("CCCR speed set failed: %d", err);
					break;
				}
				sdio_card_set_highspeed(card);
				host->ios.timing = MMC_TIMING_SD_HS;
				host->ops->set_ios(host, &host->ios);
			}
		}

		/* Set clock */
		if (sdio_card_highspeed(card))
			clock = 50000000;
		else
			clock = card->cis.max_dtr;

		if (clock > host->f_max)
			clock = host->f_max;
		/*
		 * In fpga mode the clk should be set to 12500000,
		 * or will result in scan channel setting timeout error.
		 * So in fpga mode, please set module parameter mmcclock
		 * to 12500000.
		 */
		if (mmcclock > 0)
			clock = mmcclock;
		host->ios.clock = clock;
		host->ops->set_ios(host, &host->ios);

		if (card->host->caps & MMC_CAP_4_BIT_DATA) {
			/* Set bus width & disable card detect resistor */
			err = func0_cmd52_write_byte(card, SDIO_CCCR_IF,
						     SDIO_BUS_CD_DISABLE |
						     SDIO_BUS_WIDTH_4BIT);
			if (err) {
				hif_err("Set bus mode failed: %d", err);
				break;
			}
			host->ios.bus_width = MMC_BUS_WIDTH_4;
			host->ops->set_ios(host, &host->ios);
		}
	} while (0);

	sdio_release_host(func);

	return (err) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

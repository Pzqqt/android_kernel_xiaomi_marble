/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
 *
 *
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

#ifdef CONFIG_X86
unsigned int asyncintdelay = 2;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,	"Delay clock count for async interrupt, 2 is default, valid values are 1 and 2");
#else
unsigned int asyncintdelay;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,	"Delay clock count for async interrupt, 0 is default, valid values are 1 and 2");
#endif

/**
 * hif_sdio_force_drive_strength() - Set SDIO drive strength
 * @func: pointer to sdio_func
 *
 * This function forces the driver strength of the SDIO
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
void hif_sdio_quirk_force_drive_strength(struct sdio_func *func)
{
	int err = 0;
	unsigned char value = 0;
	uint32_t mask = 0, addr = SDIO_CCCR_DRIVE_STRENGTH;
	struct hif_sdio_dev *device = sdio_get_drvdata(func);

	uint16_t  manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	switch (manfid) {
	case MANUFACTURER_ID_QCN7605_BASE:
		break;
	default:
		err = func0_cmd52_read_byte(func->card, addr, &value);
		if (err) {
			HIF_ERROR("%s: read driver strength 0x%02X fail %d\n",
				  __func__, addr, err);
			break;
		}

		mask = (SDIO_DRIVE_DTSx_MASK << SDIO_DRIVE_DTSx_SHIFT);
		value = (value & ~mask) | SDIO_DTSx_SET_TYPE_D;
		err = func0_cmd52_write_byte(func->card, addr, value);
		if (err) {
			HIF_ERROR("%s: write driver strength failed", __func__);
			HIF_ERROR("%s: 0x%02X to 0x%02X failed: %d\n", __func__,
				  (uint32_t)value, addr, err);
			break;
		}

		value = 0;
		addr = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR;
		err = func0_cmd52_read_byte(func->card,	addr, &value);
		if (err) {
			HIF_ERROR("%s Read CCCR 0x%02X failed: %d\n",
				  __func__, addr, err);
			break;
		}

		mask = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK;
		value = (value & ~mask) |
			CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A |
			CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C |
			CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D;
		err = func0_cmd52_write_byte(func->card, addr, value);
		if (err)
			HIF_ERROR("%s Write CCCR 0x%02X to 0x%02X failed: %d\n",
				  __func__, addr, value, err);

		break;
	}
}

/**
 * hif_sdio_quirk_write_cccr() - write a desired CCCR register
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter writecccr
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
void hif_sdio_quirk_write_cccr(struct sdio_func *func)
{
	int32_t err;

	if (writecccr1) {
		err = func0_cmd52_write_byte(func->card, writecccr1,
					     writecccr1value);
		if (err)
			HIF_ERROR("%s Write CCCR 0x%02X to 0x%02X failed: %d\n",
				  __func__,
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
			HIF_ERROR("%s Write CCCR 0x%02X to 0x%02X failed: %d\n",
				  __func__,
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
			HIF_ERROR("%s Write CCCR 0x%02X to 0x%02X failed: %d\n",
				  __func__,
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
			HIF_ERROR("%s Write CCCR 0x%02X to 0x%02X failed: %d\n",
				  __func__,
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
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter writecccr
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
int hif_sdio_quirk_mod_strength(struct sdio_func *func)
{
	int ret = 0;
	uint32_t addr, value;
	struct hif_sdio_dev *device = sdio_get_drvdata(func);
	uint16_t  manfid = device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

	if (!modstrength) /* TODO: Dont set this : scn is not popolated yet */
		return 0;

	if (!scn) {
		HIF_ERROR("%s: scn is null", __func__);
		return -1;
	}

	if (!scn->hostdef) {
		HIF_ERROR("%s: scn->hostdef is null", __func__);
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
			HIF_ERROR("%s write 0x%x 0x%x error:%d\n",
				  __func__, addr, value, ret);
			break;
		}
		HIF_INFO("%s: addr 0x%x val 0x%x", __func__, addr, value);

		addr = WINDOW_WRITE_ADDR_ADDRESS;
		value = 0x50F8;
		ret = sdio_memcpy_toio(func, addr, &value, 4);
		if (ret) {
			HIF_ERROR("%s write 0x%x 0x%x error:%d\n",
				  __func__, addr, value, ret);
			break;
		}
		HIF_INFO("%s: addr 0x%x val 0x%x\n", __func__, addr, value);
		break;
	}

	return ret;
}

/**
 * hif_sdio_quirk_async_intr() - Set asynchronous interrupt settings
 * @func: pointer to sdio_func
 *
 * The values are taken from the module parameter asyncintdelay
 * Call this with the sdhci host claimed
 *
 * Return: none.
 */
int hif_sdio_quirk_async_intr(struct sdio_func *func)
{
	uint8_t data;
	uint16_t manfid;
	int set_async_irq = 0, ret = 0;
	struct hif_sdio_dev *device = sdio_get_drvdata(func);

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
	case MANUFACTURER_ID_QCN7605_BASE:
		/* No async intr delay settings */
		asyncintdelay = 0;
		return ret;
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

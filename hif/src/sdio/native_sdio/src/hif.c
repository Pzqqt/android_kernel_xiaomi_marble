/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

/* by default setup a bounce buffer for the data packets,
 * if the underlying host controller driver
 * does not use DMA you may be able to skip this step
 * and save the memory allocation and transfer time
 */
#define HIF_USE_DMA_BOUNCE_BUFFER 1
#define ATH_MODULE_NAME hif
#include "a_debug.h"

#if HIF_USE_DMA_BOUNCE_BUFFER
/* macro to check if DMA buffer is WORD-aligned and DMA-able.
 * Most host controllers assume the
 * buffer is DMA'able and will bug-check otherwise (i.e. buffers on the stack).
 * virt_addr_valid check fails on stack memory.
 */
#define BUFFER_NEEDS_BOUNCE(buffer)  (((unsigned long)(buffer) & 0x3) || \
					!virt_addr_valid((buffer)))
#else
#define BUFFER_NEEDS_BOUNCE(buffer)   (false)
#endif
#define MAX_HIF_DEVICES 2
#ifdef HIF_MBOX_SLEEP_WAR
#define HIF_MIN_SLEEP_INACTIVITY_TIME_MS     50
#define HIF_SLEEP_DISABLE_UPDATE_DELAY 1
#define HIF_IS_WRITE_REQUEST_MBOX1_TO_3(request) \
				((request->request & HIF_SDIO_WRITE) && \
				(request->address >= 0x1000 && \
				request->address < 0x1FFFF))
#endif

unsigned int mmcbuswidth;
/* PERM:S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH */
module_param(mmcbuswidth, uint, 0644);
MODULE_PARM_DESC(mmcbuswidth,
		 "Set MMC driver Bus Width: 1-1Bit, 4-4Bit, 8-8Bit");

unsigned int mmcclock;
module_param(mmcclock, uint, 0644);
MODULE_PARM_DESC(mmcclock, "Set MMC driver Clock value");

unsigned int brokenirq;
module_param(brokenirq, uint, 0644);
MODULE_PARM_DESC(brokenirq,
		 "Set as 1 to use polling method instead of interrupt mode");

unsigned int forcesleepmode;
module_param(forcesleepmode, uint, 0644);
MODULE_PARM_DESC(forcesleepmode,
		"Set sleep mode: 0-host capbility, 1-force WOW, 2-force DeepSleep, 3-force CutPower");

#ifdef CONFIG_X86
unsigned int asyncintdelay = 2;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,
		 "Delay clock count for async interrupt, 2 is default, valid values are 1 and 2");
#else
unsigned int asyncintdelay;
module_param(asyncintdelay, uint, 0644);
MODULE_PARM_DESC(asyncintdelay,
		 "Delay clock count for async interrupt, 0 is default, valid values are 1 and 2");
#endif

unsigned int forcecard;
module_param(forcecard, uint, 0644);
MODULE_PARM_DESC(forcecard,
		 "Ignore card capabilities information to switch bus mode");

unsigned int debugcccr = 1;
module_param(debugcccr, uint, 0644);
MODULE_PARM_DESC(debugcccr, "Output this cccr values");

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

#define dev_to_sdio_func(d)		container_of(d, struct sdio_func, dev)
#define to_sdio_driver(d)		container_of(d, struct sdio_driver, drv)
static struct hif_sdio_dev *add_hif_device(struct sdio_func *func);
static struct hif_sdio_dev *get_hif_device(struct sdio_func *func);
static void del_hif_device(struct hif_sdio_dev *device);
static int func0_cmd52_write_byte(struct mmc_card *card, unsigned int address,
				  unsigned char byte);
static int func0_cmd52_read_byte(struct mmc_card *card, unsigned int address,
				 unsigned char *byte);

int reset_sdio_on_unload;
module_param(reset_sdio_on_unload, int, 0644);

uint32_t nohifscattersupport = 1;

uint32_t forcedriverstrength = 1; /* force driver strength to type D */

/* ------ Static Variables ------ */
static const struct sdio_device_id ar6k_id_table[] = {
#ifdef AR6002_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6002_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6002_BASE | 0x1))},
#endif
#ifdef AR6003_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6003_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6003_BASE | 0x1))},
#endif
#ifdef AR6004_HEADERS_DEF
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6004_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_AR6004_BASE | 0x1))},
#endif
#ifdef AR6320_HEADERS_DEF
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
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x1))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x2))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x3))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x4))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x5))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x6))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x7))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x8))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0x9))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xA))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xB))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xC))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xD))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xE))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (MANUFACTURER_ID_QCA9379_BASE | 0xF))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (0 | 0x0))},
	{SDIO_DEVICE(MANUFACTURER_CODE, (0 | 0x1))},
#endif
	{ /* null */ },
};

/* make sure we only unregister when registered. */
static int registered;

struct osdrv_callbacks osdrv_callbacks;
uint32_t onebitmode;
uint32_t busspeedlow;
uint32_t debughif;

static struct hif_sdio_dev *hif_devices[MAX_HIF_DEVICES];

static void reset_all_cards(void);
static QDF_STATUS hif_disable_func(struct hif_sdio_dev *device,
		   struct sdio_func *func);
static QDF_STATUS hif_enable_func(struct hif_sdio_dev *device,
		   struct sdio_func *func);

#if defined(WLAN_DEBUG) || defined(DEBUG)
ATH_DEBUG_INSTANTIATE_MODULE_VAR(hif,
				 "hif",
				 "(Linux MMC) Host Interconnect Framework",
				 ATH_DEBUG_MASK_DEFAULTS, 0, NULL);
#endif

static int hif_sdio_init_callbacks(struct osdrv_callbacks *callbacks)
{
	int status = 0;
	/* store the callback handlers */
	osdrv_callbacks = *callbacks;

	/* Register with bus driver core is done from HDD */
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE, ("%s: HIFInit registering\n",
					__func__));
	registered = 1;

	return status;
}
static void hif_sdio_remove_callbacks(void)
{
	qdf_mem_zero(&osdrv_callbacks, sizeof(osdrv_callbacks));
}


/**
 * hif_init() - Initializes the driver callbacks
 * @callbacks: pointer to driver callback structure
 *
 * Return: 0 on success, error number otherwise.
 */
QDF_STATUS hif_init(struct osdrv_callbacks *callbacks)
{
	int status;

	AR_DEBUG_ASSERT(callbacks != NULL);
	A_REGISTER_MODULE_DEBUG_INFO(hif);

	HIF_ENTER();

	status = hif_sdio_init_callbacks(callbacks);
	AR_DEBUG_ASSERT(status == 0);

	if (status != 0) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s sdio_register_driver failed!", __func__));
		return QDF_STATUS_E_FAILURE;
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s sdio_register_driver successful", __func__));

	return QDF_STATUS_SUCCESS;

}

/**
 * __hif_read_write() - sdio read/write wrapper
 * @device: pointer to hif device structure
 * @address: address to read
 * @buffer: buffer to hold read/write data
 * @length: length to read/write
 * @request: read/write/sync/async request
 * @context: pointer to hold calling context
 *
 * Return: 0 on success, error number otherwise.
 */
static QDF_STATUS
__hif_read_write(struct hif_sdio_dev *device,
		 uint32_t address,
		 char *buffer,
		 uint32_t length, uint32_t request, void *context)
{
	uint8_t opcode;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int ret = A_OK;
	uint8_t *tbuffer;
	bool bounced = false;

	AR_DEBUG_ASSERT(device != NULL);
	AR_DEBUG_ASSERT(device->func != NULL);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("__hif_read_write, addr:0X%06X, len:%08d, %s, %s\n",
			 address, length,
			 request & HIF_SDIO_READ ? "Read " : "Write",
			 request & HIF_ASYNCHRONOUS ? "Async" : "Sync "));

	do {
		if (request & HIF_EXTENDED_IO) {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: Command type: CMD53\n", __func__));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Invalid command type: 0x%08x\n",
				__func__, request));
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if (request & HIF_BLOCK_BASIS) {
			/* round to whole block length size */
			length =
				(length / HIF_MBOX_BLOCK_SIZE) *
				HIF_MBOX_BLOCK_SIZE;
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: Block mode (BlockLen: %d)\n",
					 __func__, length));
		} else if (request & HIF_BYTE_BASIS) {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: Byte mode (BlockLen: %d)\n",
					 __func__, length));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: Invalid data mode: 0x%08x\n",
					 __func__, request));
			status = QDF_STATUS_E_INVAL;
			break;
		}
		if (request & HIF_SDIO_WRITE) {
			struct hif_device_mbox_info MailBoxInfo;
			unsigned int mboxLength = 0;

			hif_configure_device(device,
					     HIF_DEVICE_GET_MBOX_ADDR,
					     &MailBoxInfo, sizeof(MailBoxInfo));
			if (address >= 0x800 && address < 0xC00) {
				/* Host control register and CIS Window */
				mboxLength = 0;
			} else if (address == MailBoxInfo.mbox_addresses[0]
				   || address == MailBoxInfo.mbox_addresses[1]
				   || address == MailBoxInfo.mbox_addresses[2]
				   || address ==
						MailBoxInfo.mbox_addresses[3]) {
				mboxLength = HIF_MBOX_WIDTH;
			} else if (address ==
				   MailBoxInfo.mbox_prop[0].extended_address) {
				mboxLength =
					MailBoxInfo.mbox_prop[0].extended_size;
			} else if (address ==
				   MailBoxInfo.mbox_prop[1].extended_address) {
				mboxLength =
					MailBoxInfo.mbox_prop[1].extended_size;
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("Invalid written address: 0x%08x\n",
					address));
				break;
			}
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("address:%08X, Length:0x%08X, Dummy:0x%04X, Final:0x%08X\n",
				 address, length,
				 (request & HIF_DUMMY_SPACE_MASK) >> 16,
				 mboxLength ==
				 0 ? address : address + (mboxLength -
				 length)));
			if (mboxLength != 0) {
				if (length > mboxLength) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: written length(0x%08X) larger than mbox len(0x%08x)\n",
						 __func__, length, mboxLength));
					break;
				}
				address += (mboxLength - length);
				/*
				 * plus dummy byte count
				 */
				address += ((request &
						HIF_DUMMY_SPACE_MASK) >> 16);
			}
		}

		if (request & HIF_FIXED_ADDRESS) {
			opcode = CMD53_FIXED_ADDRESS;
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: Address mode: Fixed 0x%X\n",
					 __func__, address));
		} else if (request & HIF_INCREMENTAL_ADDRESS) {
			opcode = CMD53_INCR_ADDRESS;
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: Address mode: Incremental 0x%X\n",
				 __func__, address));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Invalid address mode: 0x%08x\n",
				 __func__, request));
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if (request & HIF_SDIO_WRITE) {
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (BUFFER_NEEDS_BOUNCE(buffer)) {
				AR_DEBUG_ASSERT(device->dma_buffer != NULL);
				tbuffer = device->dma_buffer;
				/* copy the write data to the dma buffer */
				AR_DEBUG_ASSERT(length <= HIF_DMA_BUFFER_SIZE);
				if (length > HIF_DMA_BUFFER_SIZE) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
						("%s: Invalid write length: %d\n",
						__func__, length));
					status = QDF_STATUS_E_INVAL;
					break;
				}
				memcpy(tbuffer, buffer, length);
				bounced = true;
			} else {
				tbuffer = buffer;
			}
#else
			tbuffer = buffer;
#endif
			if (opcode == CMD53_FIXED_ADDRESS  && tbuffer != NULL) {
				ret =
					sdio_writesb(device->func, address,
						tbuffer,
						length);
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: writesb ret=%d address: 0x%X, len: %d, 0x%X\n",
					 __func__, ret, address, length,
					 *(int *)tbuffer));
			} else if (tbuffer) {
				ret =
					sdio_memcpy_toio(device->func, address,
							 tbuffer, length);
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: writeio ret=%d address: 0x%X, len: %d, 0x%X\n",
					 __func__, ret, address, length,
					 *(int *)tbuffer));
			}
		} else if (request & HIF_SDIO_READ) {
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (BUFFER_NEEDS_BOUNCE(buffer)) {
				AR_DEBUG_ASSERT(device->dma_buffer != NULL);
				AR_DEBUG_ASSERT(length <= HIF_DMA_BUFFER_SIZE);
				if (length > HIF_DMA_BUFFER_SIZE) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: Invalid read length: %d\n",
					__func__, length));
					status = QDF_STATUS_E_INVAL;
					break;
				}
				tbuffer = device->dma_buffer;
				bounced = true;
			} else {
				tbuffer = buffer;
			}
#else
			tbuffer = buffer;
#endif
			if (opcode == CMD53_FIXED_ADDRESS && tbuffer != NULL) {
				ret =
					sdio_readsb(device->func, tbuffer,
						    address,
						    length);
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: readsb ret=%d address: 0x%X, len: %d, 0x%X\n",
					 __func__, ret, address, length,
					 *(int *)tbuffer));
			} else if (tbuffer) {
				ret =
					sdio_memcpy_fromio(device->func,
							   tbuffer,
							   address, length);
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: readio ret=%d address: 0x%X, len: %d, 0x%X\n",
					 __func__, ret, address, length,
					 *(int *)tbuffer));
			}
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (bounced && tbuffer)
				memcpy(buffer, tbuffer, length);
#endif
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: Invalid direction: 0x%08x\n",
					 __func__, request));
			status = QDF_STATUS_E_INVAL;
			return status;
		}

		if (ret) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: SDIO bus operation failed! MMC stack returned : %d\n",
					 __func__, ret));
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("__hif_read_write, addr:0X%06X, len:%08d, %s, %s\n",
				 address, length,
				 request & HIF_SDIO_READ ? "Read " : "Write",
				 request & HIF_ASYNCHRONOUS ? "Async" :
					 "Sync "));
			status = QDF_STATUS_E_FAILURE;
		}
	} while (false);

	return status;
}

/**
 * add_to_async_list() - add bus reqest to async task list
 * @device: pointer to hif device
 * @busrequest: pointer to type of bus request
 *
 * Return: None.
 */
void add_to_async_list(struct hif_sdio_dev *device,
		      struct bus_request *busrequest)
{
	struct bus_request *async;
	struct bus_request *active;

	qdf_spin_lock_irqsave(&device->asynclock);
	active = device->asyncreq;
	if (active == NULL) {
		device->asyncreq = busrequest;
		device->asyncreq->inusenext = NULL;
	} else {
		for (async = device->asyncreq;
		     async != NULL; async = async->inusenext) {
			active = async;
		}
		active->inusenext = busrequest;
		busrequest->inusenext = NULL;
	}
	qdf_spin_unlock_irqrestore(&device->asynclock);
}

/**
 * hif_read_write() - queue a read/write request
 * @device: pointer to hif device structure
 * @address: address to read
 * @buffer: buffer to hold read/write data
 * @length: length to read/write
 * @request: read/write/sync/async request
 * @context: pointer to hold calling context
 *
 * Return: 0 on success, error number otherwise.
 */
QDF_STATUS
hif_read_write(struct hif_sdio_dev *device,
		uint32_t address,
		char *buffer, uint32_t length,
		uint32_t request, void *context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct bus_request *busrequest;

	AR_DEBUG_ASSERT(device != NULL);
	AR_DEBUG_ASSERT(device->func != NULL);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: device 0x%pK addr 0x%X buffer 0x%pK len %d req 0x%X context 0x%pK",
			 __func__, device, address, buffer,
			 length, request, context));

	/*sdio r/w action is not needed when suspend, so just return */
	if ((device->is_suspend == true)
	    && (device->power_config == HIF_DEVICE_POWER_CUT)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_TRACE, ("skip io when suspending\n"));
		return QDF_STATUS_SUCCESS;
	}
	do {
		if ((request & HIF_ASYNCHRONOUS) ||
			(request & HIF_SYNCHRONOUS)) {
			/* serialize all requests through the async thread */
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: Execution mode: %s\n", __func__,
					 (request & HIF_ASYNCHRONOUS) ? "Async"
					 : "Synch"));
			busrequest = hif_allocate_bus_request(device);
			if (busrequest == NULL) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("no async bus requests available (%s, addr:0x%X, len:%d)\n",
					 request & HIF_SDIO_READ ? "READ" :
					 "WRITE", address, length));
				return QDF_STATUS_E_FAILURE;
			}
			busrequest->address = address;
			busrequest->buffer = buffer;
			busrequest->length = length;
			busrequest->request = request;
			busrequest->context = context;

			add_to_async_list(device, busrequest);

			if (request & HIF_SYNCHRONOUS) {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: queued sync req: 0x%lX\n",
					 __func__, (unsigned long)busrequest));

				/* wait for completion */
				up(&device->sem_async);
				if (down_interruptible(&busrequest->sem_req) !=
				    0) {
					/* interrupted, exit */
					return QDF_STATUS_E_FAILURE;
				} else {
					QDF_STATUS status = busrequest->status;

					AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				    ("%s: sync return freeing 0x%lX: 0x%X\n",
						 __func__,
						 (unsigned long)
						 busrequest,
						 busrequest->status));
					AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
						("%s: freeing req: 0x%X\n",
						 __func__,
						 (unsigned int)
						 request));
					hif_free_bus_request(device,
						busrequest);
					return status;
				}
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: queued async req: 0x%lX\n",
						__func__,
						 (unsigned long)busrequest));
				up(&device->sem_async);
				return QDF_STATUS_E_PENDING;
			}
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Invalid execution mode: 0x%08x\n",
					__func__,
					 (unsigned int)request));
			status = QDF_STATUS_E_INVAL;
			break;
		}
	} while (0);

	return status;
}

/**
 * async_task() - thread function to serialize all bus requests
 * @param: pointer to hif device
 *
 * thread function to serialize all requests, both sync and async
 * Return: 0 on success, error number otherwise.
 */
static int async_task(void *param)
{
	struct hif_sdio_dev *device;
	struct bus_request *request;
	QDF_STATUS status;

	device = (struct hif_sdio_dev *) param;
	set_current_state(TASK_INTERRUPTIBLE);
	while (!device->async_shutdown) {
		/* wait for work */
		if (down_interruptible(&device->sem_async) != 0) {
			/* interrupted, exit */
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: async task interrupted\n",
					 __func__));
			break;
		}
		if (device->async_shutdown) {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: async task stopping\n",
					 __func__));
			break;
		}
		/* we want to hold the host over multiple cmds
		 * if possible, but holding the host blocks
		 * card interrupts
		 */
		sdio_claim_host(device->func);
		qdf_spin_lock_irqsave(&device->asynclock);
		/* pull the request to work on */
		while (device->asyncreq != NULL) {
			request = device->asyncreq;
			if (request->inusenext != NULL)
				device->asyncreq = request->inusenext;
			else
				device->asyncreq = NULL;
			qdf_spin_unlock_irqrestore(&device->asynclock);
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: async_task processing req: 0x%lX\n",
				 __func__, (unsigned long)request));

			if (request->scatter_req != NULL) {
				A_ASSERT(device->scatter_enabled);
				/* pass the request to scatter routine which
				 * executes it synchronously, note, no need
				 * to free the request since scatter requests
				 * are maintained on a separate list
				 */
				status = do_hif_read_write_scatter(device,
							request);
			} else {
				/* call hif_read_write in sync mode */
				status =
					__hif_read_write(device,
							 request->address,
							 request->buffer,
							 request->length,
							 request->
							 request &
							 ~HIF_SYNCHRONOUS,
							 NULL);
				if (request->request & HIF_ASYNCHRONOUS) {
					void *context = request->context;

					AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: freeing req: 0x%lX\n",
						 __func__, (unsigned long)
						 request));
					hif_free_bus_request(device, request);
					AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				      ("%s: async_task completion req 0x%lX\n",
						 __func__, (unsigned long)
						 request));
					device->htc_callbacks.
					rwCompletionHandler(context,
							    status);
				} else {
					AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				      ("%s: async_task upping req: 0x%lX\n",
						 __func__, (unsigned long)
						 request));
					request->status = status;
					up(&request->sem_req);
				}
			}
			qdf_spin_lock_irqsave(&device->asynclock);
		}
		qdf_spin_unlock_irqrestore(&device->asynclock);
		sdio_release_host(device->func);
	}

	complete_and_exit(&device->async_completion, 0);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
/**
 * sdio_card_highspeed() - check if high speed supported
 * @card: pointer to mmc card struct
 *
 * Return: non zero if card supports high speed.
 */
static inline int sdio_card_highspeed(struct mmc_card *card)
{
	return mmc_card_highspeed(card);
}
#else
static inline int sdio_card_highspeed(struct mmc_card *card)
{
	return mmc_card_hs(card);
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
/**
 * sdio_card_set_highspeed() - set high speed
 * @card: pointer to mmc card struct
 *
 * Return: none.
 */
static inline void sdio_card_set_highspeed(struct mmc_card *card)
{
	mmc_card_set_highspeed(card);
}
#else
static inline void sdio_card_set_highspeed(struct mmc_card *card)
{
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
/**
 * sdio_card_state() - set card state
 * @card: pointer to mmc card struct
 *
 * Return: none.
 */
static inline void sdio_card_state(struct mmc_card *card)
{
	card->state &= ~MMC_STATE_HIGHSPEED;
}
#else
static inline void sdio_card_state(struct mmc_card *card)
{
}
#endif

/**
 * reinit_sdio() - re-initialize sdio bus
 * @param: pointer to hif device
 *
 * Return: 0 on success, error number otherwise.
 */
static QDF_STATUS reinit_sdio(struct hif_sdio_dev *device)
{
	int32_t err = 0;
	struct mmc_host *host;
	struct mmc_card *card;
	struct sdio_func *func;
	uint8_t  cmd52_resp;
	uint32_t clock;

	func = device->func;
	card = func->card;
	host = card->host;

	sdio_claim_host(func);

	do {
		/* Enable high speed */
		if (card->host->caps & MMC_CAP_SD_HIGHSPEED) {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: Set high speed mode\n",
				__func__));
			err = func0_cmd52_read_byte(card, SDIO_CCCR_SPEED,
						&cmd52_resp);
			if (err) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: CMD52 read to CCCR speed register failed  : %d\n",
					__func__, err));
				sdio_card_state(card);
		/* no need to break */
			} else {
				err = func0_cmd52_write_byte(card,
						SDIO_CCCR_SPEED,
						(cmd52_resp | SDIO_SPEED_EHS));
				if (err) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("%s: CMD52 write to CCCR speed register failed  : %d\n",
						 __func__, err));
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
		/* CMD52: Set bus width & disable card detect resistor */
		err = func0_cmd52_write_byte(card, SDIO_CCCR_IF,
				SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_4BIT);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: CMD52 to set bus mode failed : %d\n",
					__func__, err));
			break;
		}
		host->ios.bus_width = MMC_BUS_WIDTH_4;
		host->ops->set_ios(host, &host->ios);
	}
	} while (0);

	sdio_release_host(func);

	return (err) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/*
 * Setup IRQ mode for deep sleep and WoW
 * Switch back to 1 bits mode when we suspend for
 * WoW in order to detect SDIO irq without clock.
 * Re-enable async 4-bit irq mode for some host controllers
 * after resume.
 */
static int sdio_enable4bits(struct hif_sdio_dev *device, int enable)
{
	int ret = 0;
	struct sdio_func *func = device->func;
	struct mmc_card *card = func->card;
	struct mmc_host *host = card->host;

	if (!(host->caps & (MMC_CAP_4_BIT_DATA)))
		return 0;

	if (card->cccr.low_speed && !card->cccr.wide_bus)
		return 0;

	sdio_claim_host(func);
	do {
		int setAsyncIRQ = 0;
		__u16 manufacturer_id =
			device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;

		/* Re-enable 4-bit ASYNC interrupt on AR6003x
		 * after system resume for some host controller
		 */
		if (manufacturer_id == MANUFACTURER_ID_AR6003_BASE) {
			setAsyncIRQ = 1;
			ret =
				func0_cmd52_write_byte(func->card,
					    CCCR_SDIO_IRQ_MODE_REG_AR6003,
					    enable ?
					    SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6003
					    : 0);
		} else if (manufacturer_id == MANUFACTURER_ID_AR6320_BASE ||
			     manufacturer_id == MANUFACTURER_ID_QCA9377_BASE ||
			     manufacturer_id == MANUFACTURER_ID_QCA9379_BASE) {
			unsigned char data = 0;

			setAsyncIRQ = 1;
			ret =
				func0_cmd52_read_byte(func->card,
					      CCCR_SDIO_IRQ_MODE_REG_AR6320,
						   &data);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: failed to read interrupt extension register %d\n",
						 __func__, ret));
				sdio_release_host(func);
				return ret;
			}
			if (enable)
				data |= SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
			else
				data &= ~SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
			ret =
				func0_cmd52_write_byte(func->card,
					       CCCR_SDIO_IRQ_MODE_REG_AR6320,
					       data);
		}
		if (setAsyncIRQ) {
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: failed to setup 4-bit ASYNC IRQ mode into %d err %d\n",
					 __func__, enable, ret));
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
					("%s: Setup 4-bit ASYNC IRQ mode into %d successfully\n",
					 __func__, enable));
			}
		}
	} while (0);
	sdio_release_host(func);

	return ret;
}


/**
 * power_state_change_notify() - SDIO bus power notification handler
 * @config: hif device power change type
 *
 * Return: 0 on success, error number otherwise.
 */
static QDF_STATUS
power_state_change_notify(struct hif_sdio_dev *device,
			enum HIF_DEVICE_POWER_CHANGE_TYPE config)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sdio_func *func = device->func;
	int old_reset_val;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: config type %d\n",
			__func__, config));
	switch (config) {
	case HIF_DEVICE_POWER_DOWN:
		/* Disable 4bits to allow SDIO bus to detect
		 * DAT1 as interrupt source
		 */
		sdio_enable4bits(device, 0);
		break;
	case HIF_DEVICE_POWER_CUT:
		old_reset_val = reset_sdio_on_unload;
		reset_sdio_on_unload = 1;
		status = hif_disable_func(device, func);
		reset_sdio_on_unload = old_reset_val;
		if (!device->is_suspend) {
			device->power_config = config;
			mmc_detect_change(device->host, HZ / 3);
		}
		break;
	case HIF_DEVICE_POWER_UP:
		if (device->power_config == HIF_DEVICE_POWER_CUT) {
			if (device->is_suspend) {
				status = reinit_sdio(device);
				/* set power_config before EnableFunc to
				 * passthrough sdio r/w action when resuming
				 * from cut power
				 */
				device->power_config = config;
				if (status == QDF_STATUS_SUCCESS)
					status = hif_enable_func(device, func);
			} else {
				/* device->func is bad pointer at this time */
				mmc_detect_change(device->host, 0);
				return QDF_STATUS_E_PENDING;
			}
		} else if (device->power_config == HIF_DEVICE_POWER_DOWN) {
			int ret = sdio_enable4bits(device, 1);

			status = (ret == 0) ? QDF_STATUS_SUCCESS :
						QDF_STATUS_E_FAILURE;
		}
		break;
	}
	device->power_config = config;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s:\n", __func__));

	return status;
}

#ifdef SDIO_3_0
/**
 * set_extended_mbox_size() - set extended MBOX size
 * @pinfo: sdio mailbox info
 *
 * Return: none.
 */
static void set_extended_mbox_size(struct hif_device_mbox_info *pinfo)
{
	pinfo->mbox_prop[0].extended_size =
		HIF_MBOX0_EXTENDED_WIDTH_AR6320_ROME_2_0;
	pinfo->mbox_prop[1].extended_size =
		HIF_MBOX1_EXTENDED_WIDTH_AR6320;
}

/**
 * set_extended_mbox_address() - set extended MBOX address
 * @pinfo: sdio mailbox info
 *
 * Return: none.
 */
static void set_extended_mbox_address(struct hif_device_mbox_info *pinfo)
{
	pinfo->mbox_prop[1].extended_address =
		pinfo->mbox_prop[0].extended_address +
		pinfo->mbox_prop[0].extended_size +
		HIF_MBOX_DUMMY_SPACE_SIZE_AR6320;
}
#else
static void set_extended_mbox_size(struct hif_device_mbox_info *pinfo)
{
	pinfo->mbox_prop[0].extended_size =
		HIF_MBOX0_EXTENDED_WIDTH_AR6320;
}
static inline void
set_extended_mbox_address(struct hif_device_mbox_info *pinfo)
{

}
#endif

/**
 * set_extended_mbox_window_info() - set extended MBOX window
 * information for SDIO interconnects
 * @manf_id: manufacturer id
 * @pinfo: sdio mailbox info
 *
 * Return: none.
 */
static void set_extended_mbox_window_info(uint16_t manf_id,
			 struct hif_device_mbox_info *pinfo)
{
	switch (manf_id & MANUFACTURER_ID_AR6K_BASE_MASK) {
	case MANUFACTURER_ID_AR6002_BASE:
		/* MBOX 0 has an extended range */

		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6003_V1;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6003_V1;

		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6003_V1;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6003_V1;

		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6004;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6004;

		break;
	case MANUFACTURER_ID_AR6003_BASE:
		/* MBOX 0 has an extended range */
		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6003_V1;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6003_V1;
		pinfo->gmbox_address = HIF_GMBOX_BASE_ADDR;
		pinfo->gmbox_size = HIF_GMBOX_WIDTH;
		break;
	case MANUFACTURER_ID_AR6004_BASE:
		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6004;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6004;
		pinfo->gmbox_address = HIF_GMBOX_BASE_ADDR;
		pinfo->gmbox_size = HIF_GMBOX_WIDTH;
		break;
	case MANUFACTURER_ID_AR6320_BASE: {
		uint16_t ManuRevID =
			manf_id & MANUFACTURER_ID_AR6K_REV_MASK;
		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320;
		if (ManuRevID < 4) {
			pinfo->mbox_prop[0].extended_size =
				HIF_MBOX0_EXTENDED_WIDTH_AR6320;
		} else {
		/* from rome 2.0(0x504), the width has been extended to 56K */
			set_extended_mbox_size(pinfo);
		}
		set_extended_mbox_address(pinfo);
		pinfo->gmbox_address = HIF_GMBOX_BASE_ADDR;
		pinfo->gmbox_size = HIF_GMBOX_WIDTH;
		break;
	}
	case MANUFACTURER_ID_QCA9377_BASE:
	case MANUFACTURER_ID_QCA9379_BASE:
		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320;
		pinfo->mbox_prop[0].extended_size =
			HIF_MBOX0_EXTENDED_WIDTH_AR6320_ROME_2_0;
		pinfo->mbox_prop[1].extended_address =
			pinfo->mbox_prop[0].extended_address +
			pinfo->mbox_prop[0].extended_size +
			HIF_MBOX_DUMMY_SPACE_SIZE_AR6320;
		pinfo->mbox_prop[1].extended_size =
			HIF_MBOX1_EXTENDED_WIDTH_AR6320;
		pinfo->gmbox_address = HIF_GMBOX_BASE_ADDR;
		pinfo->gmbox_size = HIF_GMBOX_WIDTH;
		break;
	default:
		A_ASSERT(false);
		break;
	}
}

/**
 * hif_configure_device() - configure sdio device
 * @device: pointer to hif device structure
 * @opcode: configuration type
 * @config: configuration value to set
 * @configLen: configuration length
 *
 * Return: 0 on success, error number otherwise.
 */
QDF_STATUS
hif_configure_device(struct hif_sdio_dev *device,
		     enum hif_device_config_opcode opcode,
		     void *config, uint32_t config_len)
{
	uint32_t count;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (opcode) {
	case HIF_DEVICE_GET_MBOX_BLOCK_SIZE:
		((uint32_t *) config)[0] = HIF_MBOX0_BLOCK_SIZE;
		((uint32_t *) config)[1] = HIF_MBOX1_BLOCK_SIZE;
		((uint32_t *) config)[2] = HIF_MBOX2_BLOCK_SIZE;
		((uint32_t *) config)[3] = HIF_MBOX3_BLOCK_SIZE;
		break;

	case HIF_DEVICE_GET_MBOX_ADDR:
		for (count = 0; count < 4; count++) {
			((uint32_t *) config)[count] =
				HIF_MBOX_START_ADDR(count);
		}

		if (config_len >= sizeof(struct hif_device_mbox_info)) {
			set_extended_mbox_window_info((uint16_t) device->func->
					      device,
					      (struct hif_device_mbox_info *)
					      config);
		}

		break;
	case HIF_DEVICE_GET_PENDING_EVENTS_FUNC:
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s: configuration opcode %d\n",
				 __func__, opcode));
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_DEVICE_GET_IRQ_PROC_MODE:
		*((enum hif_device_irq_mode *) config) =
			HIF_DEVICE_IRQ_SYNC_ONLY;
		break;
	case HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC:
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s: configuration opcode %d\n",
				 __func__, opcode));
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT:
		if (!device->scatter_enabled)
			return QDF_STATUS_E_NOSUPPORT;
		status =
			setup_hif_scatter_support(device,
				  (struct HIF_DEVICE_SCATTER_SUPPORT_INFO *)
				   config);
		if (QDF_IS_STATUS_ERROR(status))
			device->scatter_enabled = false;
		break;
	case HIF_DEVICE_GET_OS_DEVICE:
		/* pass back a pointer to the SDIO function's "dev" struct */
		((struct HIF_DEVICE_OS_DEVICE_INFO *) config)->os_dev =
			&device->func->dev;
		break;
	case HIF_DEVICE_POWER_STATE_CHANGE:
		status =
			power_state_change_notify(device,
					  *(enum HIF_DEVICE_POWER_CHANGE_TYPE *)
					   config);
		break;
	case HIF_DEVICE_GET_IRQ_YIELD_PARAMS:
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
			("%s: configuration opcode %d\n",
				 __func__, opcode));
		status = QDF_STATUS_E_FAILURE;
		break;
	case HIF_DEVICE_SET_HTC_CONTEXT:
		device->htc_context = config;
		break;
	case HIF_DEVICE_GET_HTC_CONTEXT:
		if (config == NULL) {
			AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s: htc context is NULL\n",
				__func__));
			return QDF_STATUS_E_FAILURE;
		}
		*(void **)config = device->htc_context;
		break;
	case HIF_BMI_DONE:
	{
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s: BMI_DONE\n", __func__));
		break;
	}
	default:
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
			("%s: Unsupported configuration opcode: %d\n",
			 __func__, opcode));
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * hif_sdio_shutdown() - hif-sdio shutdown routine
 * @hif_ctx: pointer to hif_softc structore
 *
 * Return: None.
 */
void hif_sdio_shutdown(struct hif_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Enter\n", __func__));
	if (hif_device != NULL) {
		AR_DEBUG_ASSERT(hif_device->power_config == HIF_DEVICE_POWER_CUT
				|| hif_device->func != NULL);
	} else {
		int i;
		/* since we are unloading the driver anyways,
		 * reset all cards in case the SDIO card is
		 * externally powered and we are unloading the SDIO
		 * stack. This avoids the problem when the SDIO stack
		 * is reloaded and attempts are made to re-enumerate
		 * a card that is already enumerated
		 */
		AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: hif_shut_down_device, resetting\n",
				__func__));
		reset_all_cards();

		/* Unregister with bus driver core */
		if (registered) {
			registered = 0;
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			   ("%s: Unregistering with the bus driver\n",
			   __func__));
			hif_sdio_remove_callbacks();
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: Unregistered!",
					__func__));
		}

		for (i = 0; i < MAX_HIF_DEVICES; ++i) {
			if (hif_devices[i] && hif_devices[i]->func == NULL) {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: Remove pending hif_device %pK\n",
					 __func__, hif_devices[i]));
				del_hif_device(hif_devices[i]);
				hif_devices[i] = NULL;
			}
		}
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Exit\n", __func__));
}

/**
 * hif_irq_handler() - hif-sdio interrupt handler
 * @func: pointer to sdio_func
 *
 * Return: None.
 */
static void hif_irq_handler(struct sdio_func *func)
{
	QDF_STATUS status;
	struct hif_sdio_dev *device;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Enter\n", __func__));

	device = get_hif_device(func);
	atomic_set(&device->irq_handling, 1);
	/* release the host during intr so we can use
	 * it when we process cmds
	 */
	sdio_release_host(device->func);
	status = device->htc_callbacks.dsrHandler(device->htc_callbacks
						  .context);
	sdio_claim_host(device->func);
	atomic_set(&device->irq_handling, 0);
	AR_DEBUG_ASSERT(status == QDF_STATUS_SUCCESS ||
			status == QDF_STATUS_E_CANCELED);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Exit\n", __func__));
}

/**
 * startup_task() - startup task to fill ol_softc
 * @param: pointer to struct hif_sdio_dev
 *
 * Return: 0 on success, error number otherwise.
 */
static int startup_task(void *param)
{
	struct hif_sdio_dev *device;

	device = (struct hif_sdio_dev *) param;
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: call HTC from startup_task\n",
			__func__));
	/* start  up inform DRV layer */
	if ((osdrv_callbacks.
	     device_inserted_handler(osdrv_callbacks.context,
				device)) != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Device rejected\n", __func__));
	}

	return 0;
}

static int enable_task(void *param)
{
	struct hif_sdio_dev *device;

	device = (struct hif_sdio_dev *) param;
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
		("%s: call  from resume_task\n",
		__func__));

	/* start  up inform DRV layer */
	if (device &&
	    device->claimed_ctx &&
	    osdrv_callbacks.device_power_change_handler &&
	    osdrv_callbacks.device_power_change_handler(device->claimed_ctx,
						    HIF_DEVICE_POWER_UP) !=
	    QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Device rejected\n",
			__func__));
	}

	return 0;
}

/**
 * foce_drive_strength() - Set sdio drive strength
 * @func: pointer to sdio_func
 *
 * Return: none.
 */
static void foce_drive_strength(struct sdio_func *func)
{
	unsigned int  addr = SDIO_CCCR_DRIVE_STRENGTH;
	unsigned char value = 0;

	uint32_t err = func0_cmd52_read_byte(func->card,
			addr, &value);
	if (err) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
		  ("%s: Read CCCR 0x%02X failed: %d\n",
			 __func__,
			(unsigned int) addr,
			(unsigned int) err));
	} else {
		value = (value &
			(~(SDIO_DRIVE_DTSx_MASK <<
			SDIO_DRIVE_DTSx_SHIFT))) |
			SDIO_DTSx_SET_TYPE_D;
		err = func0_cmd52_write_byte(func->card, addr,
				value);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			  ("%s: Write CCCR 0x%02X to 0x%02X failed: %d\n",
			    __func__,
				(unsigned int) addr,
				(unsigned int) value,
				(unsigned int) err));
		} else {
			addr = CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR;
			value = 0;
			err = func0_cmd52_read_byte(func->card,
					 addr, &value);
			if (err) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("Read CCCR 0x%02X failed: %d\n",
					(unsigned int) addr,
					(unsigned int) err));
			} else {
				value = (value &
					(~CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK)
					) |
					CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A |
					CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C |
					CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D;
				err = func0_cmd52_write_byte(func->card,
						addr, value);
				if (err) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					  ("Write CCCR 0x%02X to 0x%02X failed: %d\n",
						(unsigned int) addr,
						(unsigned int) value,
						(unsigned int) err));
				}
			}
		}
	}
}

/**
 * write_cccr() - write CCCR
 * @func: pointer to sdio_func
 *
 * Return: none.
 */
static void write_cccr(struct sdio_func *func)
{
	if (writecccr1) {
		uint32_t err = func0_cmd52_write_byte(func->card,
				      writecccr1,
				      writecccr1value);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X failed: %d\n",
				(unsigned int)writecccr1,
				(unsigned int)writecccr1value,
				(unsigned int)err));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X OK\n",
				(unsigned int)writecccr1,
				(unsigned int)writecccr1value));
		}
	}
	if (writecccr2) {
		uint32_t err = func0_cmd52_write_byte(func->card,
						      writecccr2,
						      writecccr2value);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X failed: %d\n",
				(unsigned int)writecccr2,
				(unsigned int)writecccr2value,
				(unsigned int)err));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X OK\n",
				(unsigned int)writecccr2,
				(unsigned int)writecccr2value));
		}
	}
	if (writecccr3) {
		uint32_t err = func0_cmd52_write_byte(func->card,
					      writecccr3,
						      writecccr3value);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X failed: %d\n",
				(unsigned int)writecccr3,
				(unsigned int)writecccr3value,
				(unsigned int)err));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X OK\n",
				(unsigned int)writecccr3,
				(unsigned int)writecccr3value));
		}
	}
	if (writecccr4) {
		uint32_t err = func0_cmd52_write_byte(func->card,
						      writecccr4,
						      writecccr4value);
		if (err)
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X failed: %d\n",
				(unsigned int)writecccr4,
				(unsigned int)writecccr4value,
				(unsigned int)err));
		else
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Write CCCR 0x%02X to 0x%02X OK\n",
				(unsigned int)writecccr4,
				(unsigned int)writecccr4value));
	}
}

#ifdef SDIO_BUS_WIDTH_8BIT
static int hif_cmd52_write_byte_8bit(struct sdio_func *func)
{
	return func0_cmd52_write_byte(func->card, SDIO_CCCR_IF,
			SDIO_BUS_CD_DISABLE | SDIO_BUS_WIDTH_8BIT);
}
#else
static int hif_cmd52_write_byte_8bit(struct sdio_func *func)
{
	AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("%s: 8BIT Bus Width not supported\n", __func__));
	return QDF_STATUS_E_FAILURE;
}
#endif

/**
 * hif_device_inserted() - hif-sdio driver probe handler
 * @func: pointer to sdio_func
 * @id: pointer to sdio_device_id
 *
 * Return: 0 on success, error number otherwise.
 */
static int hif_device_inserted(struct sdio_func *func,
			       const struct sdio_device_id *id)
{
	int i;
	int ret;
	struct hif_sdio_dev *device = NULL;
	int count;
	uint32_t clock, clock_set = 12500000;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: Function: 0x%X, Vendor ID: 0x%X, Device ID: 0x%X, block size: 0x%X/0x%X\n",
			 __func__, func->num, func->vendor, id->device,
			 func->max_blksize, func->cur_blksize));
	/* dma_mask should not be NULL, otherwise dma_map_single
	 * will crash. TODO: check why dma_mask is NULL here
	 */
	if (func->dev.dma_mask == NULL) {
		static u64 dma_mask = 0xFFFFFFFF;

		func->dev.dma_mask = &dma_mask;
	}
	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		struct hif_sdio_dev *hifdevice = hif_devices[i];

		if (hifdevice && hifdevice->power_config == HIF_DEVICE_POWER_CUT
		    && hifdevice->host == func->card->host) {
			hifdevice->func = func;
			hifdevice->power_config = HIF_DEVICE_POWER_UP;
			sdio_set_drvdata(func, hifdevice);
			device = get_hif_device(func);

			if (device->is_suspend) {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: Resume from suspend",
					__func__));
				ret = reinit_sdio(device);
			}
			break;
		}
	}

	if (device == NULL) {
		if (add_hif_device(func) == NULL)
			return QDF_STATUS_E_FAILURE;
		device = get_hif_device(func);

		for (i = 0; i < MAX_HIF_DEVICES; ++i) {
			if (hif_devices[i] == NULL) {
				hif_devices[i] = device;
				break;
			}
		}
		if (i == MAX_HIF_DEVICES) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: No more hif_devices[] slot for %pK",
				 __func__, device));
		}

		device->id = id;
		device->host = func->card->host;
		device->is_disabled = true;
		/* TODO: MMC SDIO3.0 Setting should also be modified in ReInit()
		 * function when Power Manage work.
		 */
		sdio_claim_host(func);
		/* force driver strength to type D */
		if (forcedriverstrength == 1)
			foce_drive_strength(func);
		write_cccr(func);
		/* Set MMC Clock */
		if (mmcclock > 0)
			clock_set = mmcclock;
		if (sdio_card_highspeed(func->card))
			clock = 50000000;
		else
			clock = func->card->cis.max_dtr;
		if (clock > device->host->f_max)
			clock = device->host->f_max;

		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s: Dumping clocks (%d,%d)\n",
		       __func__, func->card->cis.max_dtr,
		       device->host->f_max));

		/* only when mmcclock module parameter is specified,
		 * set the clock explicitly
		 */
		if (mmcclock > 0) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("Decrease host clock from %d to %d(%d,%d)\n",
					 clock, clock_set,
					 func->card->cis.max_dtr,
					 device->host->f_max));
			device->host->ios.clock = clock_set;
			device->host->ops->set_ios(device->host,
						   &device->host->ios);
		}
		/* Set SDIO3.0 */
		/* Set MMC Bus Width: 1-1Bit, 4-4Bit, 8-8Bit */
		if (mmcbuswidth > 0) {
			if (mmcbuswidth == 1) {
				ret =
					func0_cmd52_write_byte(func->card,
							  SDIO_CCCR_IF,
							  SDIO_BUS_CD_DISABLE
							  |
							  SDIO_BUS_WIDTH_1BIT);
				if (ret) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("%s: CMD52 to set bus width failed: %d\n",
						 __func__, ret));
					goto del_hif_dev;;
				}
				device->host->ios.bus_width =
					MMC_BUS_WIDTH_1;
				device->host->ops->set_ios(device->host,
							   &device->
							   host->ios);
			} else if (mmcbuswidth == 4
				   && (device->host->
				       caps & MMC_CAP_4_BIT_DATA)) {
				ret =
					func0_cmd52_write_byte(func->card,
						       SDIO_CCCR_IF,
						       SDIO_BUS_CD_DISABLE
						       |
						       SDIO_BUS_WIDTH_4BIT);
				if (ret) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: CMD52 to bus width failed: %d\n",
					 __func__,
						 ret));
					goto del_hif_dev;
				}
				device->host->ios.bus_width =
					MMC_BUS_WIDTH_4;
				device->host->ops->set_ios(device->host,
							   &device->
							   host->ios);
			} else if (mmcbuswidth == 8
				 && (device->host->
				     caps & MMC_CAP_8_BIT_DATA)) {
				ret = hif_cmd52_write_byte_8bit(func);
				if (ret) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: CMD52 to bus width failed: %d\n",
							 __func__,
							 ret));
					goto del_hif_dev;
				}
				device->host->ios.bus_width =
					MMC_BUS_WIDTH_8;
				device->host->ops->set_ios(device->host,
							   &device->
							   host->ios);
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: MMC bus width %d is not supported.\n",
						 __func__,
						 mmcbuswidth));
				ret = QDF_STATUS_E_FAILURE;
				goto del_hif_dev;
			}
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("%s: Set MMC bus width to %dBit.\n",
					 __func__, mmcbuswidth));
		}
		if (debugcccr)
			hif_dump_cccr(device);

		sdio_release_host(func);
	}

	qdf_spinlock_create(&device->lock);

	qdf_spinlock_create(&device->asynclock);

	DL_LIST_INIT(&device->scatter_req_head);

	if (!nohifscattersupport) {
		/* try to allow scatter operation on all instances,
		 * unless globally overridden
		 */
		device->scatter_enabled = true;
	} else
		device->scatter_enabled = false;

	/* Initialize the bus requests to be used later */
	qdf_mem_zero(device->bus_request, sizeof(device->bus_request));
	for (count = 0; count < BUS_REQUEST_MAX_NUM; count++) {
		sema_init(&device->bus_request[count].sem_req, 0);
		hif_free_bus_request(device, &device->bus_request[count]);
	}
	sema_init(&device->sem_async, 0);

	ret = hif_enable_func(device, func);
	if ((ret == QDF_STATUS_SUCCESS || ret == QDF_STATUS_E_PENDING))
		return 0;
	ret = QDF_STATUS_E_FAILURE;
del_hif_dev:
	del_hif_device(device);
	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		if (hif_devices[i] == device) {
			hif_devices[i] = NULL;
			break;
		}
	}
	if (i == MAX_HIF_DEVICES) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s: No hif_devices[] slot for %pK",
			__func__, device));
	}
	return ret;
}

/**
 * hif_ack_interrupt() - Acknowledge hif device irq
 * @device: pointer to struct hif_sdio_dev
 *
 * This should translate to an acknowledgment to the bus driver indicating that
 * the previous interrupt request has been serviced and the all the relevant
 * sources have been cleared. HTC is ready to process more interrupts.
 * This should prevent the bus driver from raising an interrupt unless the
 * previous one has been serviced and acknowledged using the previous API.
 *
 * Return: None.
 */
void hif_ack_interrupt(struct hif_sdio_dev *device)
{
	AR_DEBUG_ASSERT(device != NULL);

	/* Acknowledge our function IRQ */
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

	AR_DEBUG_ASSERT(device != NULL);
	AR_DEBUG_ASSERT(device->func != NULL);

	HIF_ENTER();
	/*
	 * On HP Elitebook 8460P, interrupt mode is not stable
	 * in high throughput, so polling method should be used
	 * instead of interrupt mode.
	 */
	if (brokenirq) {
		AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
			("%s: Using broken IRQ mode\n",
			__func__));
		/* disable IRQ support even the capability exists */
		device->func->card->host->caps &= ~MMC_CAP_SDIO_IRQ;
	}
	/* Register the IRQ Handler */
	sdio_claim_host(device->func);
	ret = sdio_claim_irq(device->func, hif_irq_handler);
	sdio_release_host(device->func);
	AR_DEBUG_ASSERT(ret == 0);
	HIF_EXIT();
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

	AR_DEBUG_ASSERT(device != NULL);
	AR_DEBUG_ASSERT(device->func != NULL);

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
	if (ret) {
		if (ret == -ETIMEDOUT) {
			AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s: Timeout to mask interrupt\n",
				__func__));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Unable to mask interrupt %d\n",
					 __func__, ret));
			AR_DEBUG_ASSERT(ret == 0);
		}
	}
	HIF_EXIT();
}

/**
 * hif_allocate_bus_request() - Allocate hif bus request
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: pointer to struct bus_request structure.
 */
struct bus_request *hif_allocate_bus_request(struct hif_sdio_dev *device)
{
	struct bus_request *busrequest;

	qdf_spin_lock_irqsave(&device->lock);
	busrequest = device->bus_request_free_queue;
	/* Remove first in list */
	if (busrequest != NULL)
		device->bus_request_free_queue = busrequest->next;

	/* Release lock */
	qdf_spin_unlock_irqrestore(&device->lock);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: hif_allocate_bus_request: 0x%pK\n",
			__func__, busrequest));

	return busrequest;
}

/**
 * hif_free_bus_request() - Free hif bus request
 * @device: pointer to struct hif_sdio_dev
 *
 *
 * Return: None.
 */
void hif_free_bus_request(struct hif_sdio_dev *device,
			  struct bus_request *busrequest)
{
	AR_DEBUG_ASSERT(busrequest != NULL);
	/* Acquire lock */
	qdf_spin_lock_irqsave(&device->lock);

	/* Insert first in list */
	busrequest->next = device->bus_request_free_queue;
	busrequest->inusenext = NULL;
	device->bus_request_free_queue = busrequest;

	/* Release lock */
	qdf_spin_unlock_irqrestore(&device->lock);
}

static QDF_STATUS hif_disable_func(struct hif_sdio_dev *device,
		struct sdio_func *func)
{
	int ret;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_ENTER();
	device = get_hif_device(func);
	if (!IS_ERR(device->async_task)) {
		init_completion(&device->async_completion);
		device->async_shutdown = 1;
		up(&device->sem_async);
		wait_for_completion(&device->async_completion);
		device->async_task = NULL;
		sema_init(&device->sem_async, 0);
	}
	/* Disable the card */
	sdio_claim_host(device->func);
	ret = sdio_disable_func(device->func);
	if (ret)
		status = QDF_STATUS_E_FAILURE;

	if (reset_sdio_on_unload && status == QDF_STATUS_SUCCESS) {
		/* reset the SDIO interface. It's useful in automated testing
		 * where the card does not need to be removed at the end
		 * of the test. It is expected that the user will also
		 * un/reload the host controller driver to force the bus
		 * driver to re-enumerate the slot
		 */
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s: reseting SDIO card",
				__func__));

		/* sdio_f0_writeb() cannot be used here, this allows access
		 * to undefined registers in the range of: 0xF0-0xFF
		 */

		ret =
			func0_cmd52_write_byte(device->func->card,
						SDIO_CCCR_ABORT,
					       (1 << 3));
		if (ret) {
			status = QDF_STATUS_E_FAILURE;
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: reset failed : %d\n",
				__func__, ret));
		}
	}

	sdio_release_host(device->func);

	if (status == QDF_STATUS_SUCCESS)
		device->is_disabled = true;
	cleanup_hif_scatter_resources(device);

	HIF_EXIT();

	return status;
}

static QDF_STATUS hif_enable_func(struct hif_sdio_dev *device,
				struct sdio_func *func)
{
	struct task_struct *task;
	const char *task_name = NULL;
	int (*taskFunc)(void *) = NULL;
	int ret = QDF_STATUS_SUCCESS;

	HIF_ENTER("sdio_func 0x%pK", func);

	device = get_hif_device(func);

	if (!device) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("HIF device is NULL\n"));
		return QDF_STATUS_E_INVAL;
	}

	if (device->is_disabled) {
		int setAsyncIRQ = 0;
		__u16 manufacturer_id =
			device->id->device & MANUFACTURER_ID_AR6K_BASE_MASK;
		/* enable the SDIO function */
		sdio_claim_host(func);
		/* enable 4-bit ASYNC interrupt on AR6003x or later devices */
		if (manufacturer_id == MANUFACTURER_ID_AR6003_BASE) {
			setAsyncIRQ = 1;
			ret =
				func0_cmd52_write_byte(func->card,
					CCCR_SDIO_IRQ_MODE_REG_AR6003,
					SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6003);
		} else if (manufacturer_id == MANUFACTURER_ID_AR6320_BASE ||
			   manufacturer_id == MANUFACTURER_ID_QCA9377_BASE ||
			   manufacturer_id == MANUFACTURER_ID_QCA9379_BASE) {
			unsigned char data = 0;

			setAsyncIRQ = 1;
			ret =
				func0_cmd52_read_byte(func->card,
					      CCCR_SDIO_IRQ_MODE_REG_AR6320,
						      &data);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s: failed to read irq reg %d\n",
						 __func__, ret));
				sdio_release_host(func);
				return QDF_STATUS_E_FAILURE;
			}
			data |= SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320;
			ret =
				func0_cmd52_write_byte(func->card,
					       CCCR_SDIO_IRQ_MODE_REG_AR6320,
						       data);
		}
		if (setAsyncIRQ) {
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: failed to enable ASYNC IRQ mode %d\n",
						 __func__, ret));
				sdio_release_host(func);
				return QDF_STATUS_E_FAILURE;
			}
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
				("%s: 4-bit ASYNC IRQ mode enabled\n",
				 __func__));
		}

		/* set CCCR 0xF0[7:6] to increase async interrupt delay clock to
		 * fix interrupt missing issue on dell 8460p
		 */
		if (asyncintdelay != 0) {
			unsigned char data = 0;

			ret = func0_cmd52_read_byte(func->card,
					      CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					      &data);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: failed to read CCCR %d, val is %d\n",
					__func__,
					 CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					 ret));
				sdio_release_host(func);
				return QDF_STATUS_E_FAILURE;
			}
			data = (data & ~CCCR_SDIO_ASYNC_INT_DELAY_MASK) |
			       ((asyncintdelay <<
				 CCCR_SDIO_ASYNC_INT_DELAY_LSB) &
				CCCR_SDIO_ASYNC_INT_DELAY_MASK);
			ret =
				func0_cmd52_write_byte(func->card,
					      CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					      data);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: failed to write CCCR %d, val is %d\n",
					__func__,
					 CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS,
					 ret));
				sdio_release_host(func);
				return QDF_STATUS_E_FAILURE;
			}
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			   ("%s: Set async interrupt delay clock as %d.\n",
			   __func__,
			   asyncintdelay));
		}
		/* give us some time to enable, in ms */
		func->enable_timeout = 100;
		ret = sdio_enable_func(func);
		if (ret) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Unable to enable AR6K: 0x%X\n",
				 __func__, ret));
			sdio_release_host(func);
			return QDF_STATUS_E_FAILURE;
		}
		ret = sdio_set_block_size(func, HIF_MBOX_BLOCK_SIZE);

		if (modstrength) {
			unsigned int address = WINDOW_DATA_ADDRESS;
			unsigned int value = 0x0FFF;

			ret = sdio_memcpy_toio(device->func, address,
						&value, 4);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
					("memcpy_toio 0x%x 0x%x error:%d\n",
				       address, value, ret));
			} else {
				AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
					("memcpy_toio, 0x%x 0x%x OK\n", address,
				       value));
				address = WINDOW_WRITE_ADDR_ADDRESS;
				value = 0x50F8;
				ret =
					sdio_memcpy_toio(device->func, address,
							 &value, 4);
				if (ret)
					AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
					   ("memcpy_toio 0x%x 0x%x error:%d\n",
						address, value, ret));
				else
					AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
						("memcpy_toio, 0x%x 0x%x OK\n",
					       address, value));
			}
		};
		sdio_release_host(func);
		if (ret) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("%s: can't set block size 0x%x  AR6K: 0x%X\n",
				 __func__, HIF_MBOX_BLOCK_SIZE,
				 ret));
			return QDF_STATUS_E_FAILURE;
		}
		device->is_disabled = false;
		/* create async I/O thread */
		if (!device->async_task) {
			device->async_shutdown = 0;
			device->async_task = kthread_create(async_task,
							    (void *)device,
							    "AR6K Async");
			if (IS_ERR(device->async_task)) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: to create async task\n",
						 __func__));
				return QDF_STATUS_E_FAILURE;
			}
			AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
					("%s: start async task\n",
					__func__));
			wake_up_process(device->async_task);
		}
	}

	if (!device->claimed_ctx) {
		taskFunc = startup_task;
		task_name = "AR6K startup";
		ret = QDF_STATUS_SUCCESS;
	} else {
		taskFunc = enable_task;
		task_name = "AR6K enable";
		ret = QDF_STATUS_E_PENDING;
	}
	/* create resume thread */
	task = kthread_create(taskFunc, (void *)device, task_name);
	if (IS_ERR(task)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: to create enabel task\n",
				 __func__));
		return QDF_STATUS_E_FAILURE;
	}
	wake_up_process(task);

	/* task will call the enable func, indicate pending */
	HIF_EXIT();

	return ret;
}

int hif_device_suspend(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int ret = QDF_STATUS_SUCCESS;
#if defined(MMC_PM_KEEP_POWER)
	mmc_pm_flag_t pm_flag = 0;
	enum HIF_DEVICE_POWER_CHANGE_TYPE config;
	struct mmc_host *host = NULL;
#endif

	struct hif_sdio_dev *device = get_hif_device(func);

#if defined(MMC_PM_KEEP_POWER)
	if (device && device->func)
		host = device->func->card->host;
#endif

	HIF_ENTER();
	if (device && device->claimed_ctx
	    && osdrv_callbacks.device_suspend_handler) {
		device->is_suspend = true;
		status = osdrv_callbacks.device_suspend_handler(
							device->claimed_ctx);
#if defined(MMC_PM_KEEP_POWER)
		switch (forcesleepmode) {
		case 0: /* depend on sdio host pm capbility */
			pm_flag = sdio_get_host_pm_caps(func);
			break;
		case 1: /* force WOW */
			pm_flag |= MMC_PM_KEEP_POWER | MMC_PM_WAKE_SDIO_IRQ;
			break;
		case 2: /* force DeepSleep */
			pm_flag &= ~MMC_PM_WAKE_SDIO_IRQ;
			pm_flag |= MMC_PM_KEEP_POWER;
			break;
		case 3: /* force CutPower */
			pm_flag &=
				~(MMC_PM_WAKE_SDIO_IRQ | MMC_PM_WAKE_SDIO_IRQ);
			break;
		}
		if (!(pm_flag & MMC_PM_KEEP_POWER)) {
			/* cut power support */
			/* setting power_config before hif_configure_device to
			 * skip sdio r/w when suspending with cut power
			 */
			AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
				("hif_device_suspend: cut power enter\n"));
			config = HIF_DEVICE_POWER_CUT;
			device->power_config = config;
			if ((device->claimed_ctx != NULL)
			    && osdrv_callbacks.device_removed_handler) {
				status = osdrv_callbacks.
						device_removed_handler(device->
								claimed_ctx,
								device);
			}
			ret = hif_configure_device(device,
					   HIF_DEVICE_POWER_STATE_CHANGE,
					   &config,
					   sizeof
					   (enum HIF_DEVICE_POWER_CHANGE_TYPE));
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				   ("%s: hif config device failed: %d\n",
					 __func__, ret));
				return ret;
			}

			hif_mask_interrupt(device);
			device->device_state = HIF_DEVICE_STATE_CUTPOWER;
			AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
				("hif_device_suspend: cut power success\n"));
			return ret;
		}
		ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
		if (ret) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			  ("%s: set sdio pm flags failed %d\n",
				 __func__, ret));
			return ret;
		}

		/* TODO:WOW support */
		if (pm_flag & MMC_PM_WAKE_SDIO_IRQ) {
			AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
				("hif_device_suspend: wow enter\n"));
			config = HIF_DEVICE_POWER_DOWN;
			ret = hif_configure_device(device,
				   HIF_DEVICE_POWER_STATE_CHANGE,
				   &config,
				   sizeof
				   (enum HIF_DEVICE_POWER_CHANGE_TYPE));

			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: hif config dev failed: %d\n",
					 __func__, ret));
				return ret;
			}
			ret = sdio_set_host_pm_flags(func,
						     MMC_PM_WAKE_SDIO_IRQ);
			if (ret) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("%s: set sdio pm flags %d\n",
						 __func__, ret));
				return ret;
			}
			hif_mask_interrupt(device);
			device->device_state = HIF_DEVICE_STATE_WOW;
			AR_DEBUG_PRINTF(ATH_DEBUG_INFO,
				("hif_device_suspend: wow success\n"));
			return ret;
		}
		/* deep sleep support */
		AR_DEBUG_PRINTF(ATH_DEBUG_INFO, ("%s: deep sleep enter\n",
						 __func__));

		/*
		 * Wait for some async clean handler finished.
		 * These handlers are part of vdev disconnect.
		 * As handlers are async,sleep is not suggested,
		 * some blocking method may be a good choice.
		 * But before adding callback function to these
		 * handler, sleep wait is a simple method.
		 */
		msleep(100);
		hif_mask_interrupt(device);
		device->device_state = HIF_DEVICE_STATE_DEEPSLEEP;
		AR_DEBUG_PRINTF(ATH_DEBUG_INFO, ("%s: deep sleep done\n",
						 __func__));
		return ret;
#endif
	}

	HIF_EXIT();

	switch (status) {
	case QDF_STATUS_SUCCESS:
#if defined(MMC_PM_KEEP_POWER)
		if (host) {
			host->pm_flags &=
				~(MMC_PM_KEEP_POWER | MMC_PM_WAKE_SDIO_IRQ);
		}
#endif
		return 0;
	case QDF_STATUS_E_BUSY:
#if defined(MMC_PM_KEEP_POWER)
		if (host) {
			/* WAKE_SDIO_IRQ in order to wake up by DAT1 */
			host->pm_flags |=
				(MMC_PM_KEEP_POWER | MMC_PM_WAKE_SDIO_IRQ);
			host->pm_flags &= host->pm_caps;
		}
		return 0;
#else
		return -EBUSY; /* Hack to support deep sleep and wow */
#endif
	default:
		device->is_suspend = false;

		return QDF_STATUS_E_FAILURE;
	}
}

int hif_device_resume(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	enum HIF_DEVICE_POWER_CHANGE_TYPE config;
	struct hif_sdio_dev *device;

	device = get_hif_device(func);
	if (!device) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR, ("get hif device failed\n"));
		return QDF_STATUS_E_FAILURE;
	}

	if (device->device_state == HIF_DEVICE_STATE_CUTPOWER) {
		config = HIF_DEVICE_POWER_UP;
		status = hif_configure_device(device,
					      HIF_DEVICE_POWER_STATE_CHANGE,
					      &config,
					      sizeof(enum
						 HIF_DEVICE_POWER_CHANGE_TYPE));
		if (status) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: hif_configure_device failed\n",
				 __func__));
			return status;
		}
	} else if (device->device_state == HIF_DEVICE_STATE_DEEPSLEEP) {
		hif_un_mask_interrupt(device);
	} else if (device->device_state == HIF_DEVICE_STATE_WOW) {
		/*TODO:WOW support */
		hif_un_mask_interrupt(device);
	}

	/*
	 * device_resume_handler do nothing now. If some operation
	 * should be added to this handler in power cut
	 * resume flow, do make sure those operation is not
	 * depent on what startup_task has done,or the resume
	 * flow will block.
	 */
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: +hif_device_resume\n",
			 __func__));
	if (device->claimed_ctx
	    && osdrv_callbacks.device_suspend_handler) {
		status =
		osdrv_callbacks.device_resume_handler(device->claimed_ctx);
		device->is_suspend = false;
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: -hif_device_resume\n",
			 __func__));
	device->device_state = HIF_DEVICE_STATE_ON;

	return QDF_IS_STATUS_SUCCESS(status) ? 0 : status;
}

static void hif_device_removed(struct sdio_func *func)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct hif_sdio_dev *device;
	int i;

	AR_DEBUG_ASSERT(func != NULL);
	HIF_ENTER();
	device = get_hif_device(func);

	if (device->power_config == HIF_DEVICE_POWER_CUT) {
		device->func = NULL;    /* func will be free by mmc stack */
		return;         /* Just return for cut-off mode */
	}
	for (i = 0; i < MAX_HIF_DEVICES; ++i) {
		if (hif_devices[i] == device)
			hif_devices[i] = NULL;
	}

	if (device->claimed_ctx != NULL)
		status =
		osdrv_callbacks.device_removed_handler(device->claimed_ctx,
							    device);

	hif_mask_interrupt(device);

	if (device->is_disabled)
		device->is_disabled = false;
	else
		status = hif_disable_func(device, func);


	del_hif_device(device);
	if (status != QDF_STATUS_SUCCESS)
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
		  ("%s: Unable to disable sdio func\n",
		   __func__));

	HIF_EXIT();
}

/*
 * This should be moved to AR6K HTC layer.
 */
QDF_STATUS hif_wait_for_pending_recv(struct hif_sdio_dev *device)
{
	int32_t cnt = 10;
	uint8_t host_int_status;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	do {
		while (atomic_read(&device->irq_handling)) {
			/* wait until irq handler finished all the jobs */
			schedule_timeout_interruptible(HZ / 10);
		}
		/* check if there is any pending irq due to force done */
		host_int_status = 0;
		status = hif_read_write(device, HOST_INT_STATUS_ADDRESS,
					(uint8_t *) &host_int_status,
					sizeof(host_int_status),
					HIF_RD_SYNC_BYTE_INC, NULL);
		host_int_status =
			QDF_IS_STATUS_SUCCESS(status) ?
				(host_int_status & (1 << 0)) : 0;
		if (host_int_status)
			/* wait until irq handler finishs its job */
			schedule_timeout_interruptible(1);
	} while (host_int_status && --cnt > 0);

	if (host_int_status && cnt == 0)
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Unable clear up pending IRQ\n",
				 __func__));

	return QDF_STATUS_SUCCESS;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0)) && \
		 !defined(WITH_BACKPORTS)
/**
 * hif_sdio_set_drvdata() - set driver data
 * @func: pointer to sdio function
 * @hifdevice: pointer to hif device
 *
 * Return: non zero for success.
 */
static inline int hif_sdio_set_drvdata(struct sdio_func *func,
					struct hif_sdio_dev *hifdevice)
{
	return sdio_set_drvdata(func, hifdevice);
}
#else
static inline int hif_sdio_set_drvdata(struct sdio_func *func,
					struct hif_sdio_dev *hifdevice)
{
	sdio_set_drvdata(func, hifdevice);
	return 0;
}
#endif

static struct hif_sdio_dev *add_hif_device(struct sdio_func *func)
{
	struct hif_sdio_dev *hifdevice = NULL;
	int ret = 0;

	HIF_ENTER();
	AR_DEBUG_ASSERT(func != NULL);
	hifdevice = (struct hif_sdio_dev *) qdf_mem_malloc(sizeof(
							struct hif_sdio_dev));
	AR_DEBUG_ASSERT(hifdevice != NULL);
	if (hifdevice == NULL) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR, ("Alloc hif device fail\n"));
		return NULL;
	}
#if HIF_USE_DMA_BOUNCE_BUFFER
	hifdevice->dma_buffer = qdf_mem_malloc(HIF_DMA_BUFFER_SIZE);
	AR_DEBUG_ASSERT(hifdevice->dma_buffer != NULL);
	if (hifdevice->dma_buffer == NULL) {
		qdf_mem_free(hifdevice);
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR, ("Alloc dma buffer fail\n"));
		return NULL;
	}
#endif
	hifdevice->func = func;
	hifdevice->power_config = HIF_DEVICE_POWER_UP;
	hifdevice->device_state = HIF_DEVICE_STATE_ON;
	ret = hif_sdio_set_drvdata(func, hifdevice);
	HIF_EXIT("status %d", ret);

	return hifdevice;
}

static struct hif_sdio_dev *get_hif_device(struct sdio_func *func)
{
	AR_DEBUG_ASSERT(func != NULL);

	return (struct hif_sdio_dev *) sdio_get_drvdata(func);
}

static void del_hif_device(struct hif_sdio_dev *device)
{
	AR_DEBUG_ASSERT(device != NULL);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRACE,
			("%s: deleting hif device 0x%pK\n",
				__func__, device));
	if (device->dma_buffer != NULL)
		qdf_mem_free(device->dma_buffer);

	qdf_mem_free(device);
}

static void reset_all_cards(void)
{
}

QDF_STATUS hif_attach_htc(struct hif_sdio_dev *device,
				struct htc_callbacks *callbacks)
{
	if (device->htc_callbacks.context != NULL)
		/* already in use! */
		return QDF_STATUS_E_FAILURE;
	device->htc_callbacks = *callbacks;

	return QDF_STATUS_SUCCESS;
}

void hif_detach_htc(struct hif_opaque_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	qdf_mem_zero(&hif_device->htc_callbacks,
			  sizeof(hif_device->htc_callbacks));
}

#define SDIO_SET_CMD52_ARG(arg, rw, func, raw, address, writedata) \
			((arg) = (((rw) & 1) << 31) | \
			((func & 0x7) << 28) | \
			(((raw) & 1) << 27) | \
			(1 << 26) | \
			(((address) & 0x1FFFF) << 9) | \
			(1 << 8) | \
			((writedata) & 0xFF))

#define SDIO_SET_CMD52_READ_ARG(arg, func, address) \
	SDIO_SET_CMD52_ARG(arg, 0, (func), 0, address, 0x00)
#define SDIO_SET_CMD52_WRITE_ARG(arg, func, address, value) \
	SDIO_SET_CMD52_ARG(arg, 1, (func), 0, address, value)

static int func0_cmd52_write_byte(struct mmc_card *card,
				  unsigned int address,
				  unsigned char byte)
{
	struct mmc_command io_cmd;
	unsigned long arg;
	int status = 0;

	memset(&io_cmd, 0, sizeof(io_cmd));
	SDIO_SET_CMD52_WRITE_ARG(arg, 0, address, byte);
	io_cmd.opcode = SD_IO_RW_DIRECT;
	io_cmd.arg = arg;
	io_cmd.flags = MMC_RSP_R5 | MMC_CMD_AC;
	status = mmc_wait_for_cmd(card->host, &io_cmd, 0);

	if (status)
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: mmc_wait_for_cmd returned %d\n",
				 __func__, status));

	return status;
}

static int func0_cmd52_read_byte(struct mmc_card *card,
				 unsigned int address,
				 unsigned char *byte)
{
	struct mmc_command io_cmd;
	unsigned long arg;
	int32_t err;

	memset(&io_cmd, 0, sizeof(io_cmd));
	SDIO_SET_CMD52_READ_ARG(arg, 0, address);
	io_cmd.opcode = SD_IO_RW_DIRECT;
	io_cmd.arg = arg;
	io_cmd.flags = MMC_RSP_R5 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &io_cmd, 0);

	if ((!err) && (byte))
		*byte = io_cmd.resp[0] & 0xFF;

	if (err)
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: mmc_wait_for_cmd returned %d\n",
				 __func__, err));

	return err;
}

void hif_dump_cccr(struct hif_sdio_dev *hif_device)
{
	int i;
	uint8_t cccr_val;
	uint32_t err;

	if (!hif_device || !hif_device->func ||
				!hif_device->func->card) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
			("hif_dump_cccr incorrect input arguments\n"));
		return;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_ERROR, ("hif_dump_cccr "));
	for (i = 0; i <= 0x16; i++) {
		err = func0_cmd52_read_byte(hif_device->func->card,
						i, &cccr_val);
		if (err) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("Reading CCCR 0x%02X failed: %d\n",
			       (unsigned int)i, (unsigned int)err));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%X(%X) ", (unsigned int)i,
			       (unsigned int)cccr_val));
		}
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_ERROR, ("\n"));
}

int hif_sdio_device_inserted(struct device *dev,
					const struct sdio_device_id *id)
{
	struct sdio_func *func = dev_to_sdio_func(dev);

	return hif_device_inserted(func, id);
}

void hif_sdio_device_removed(struct sdio_func *func)
{
	hif_device_removed(func);
}

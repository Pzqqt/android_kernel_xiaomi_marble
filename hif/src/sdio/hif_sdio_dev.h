/*
 * Copyright (c) 2013-2016, 2018-2019 The Linux Foundation. All rights reserved.
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

#ifndef HIF_SDIO_DEV_H_
#define HIF_SDIO_DEV_H_

#include "qdf_net_types.h"
#include "a_types.h"
#include "athdefs.h"
#include "a_osapi.h"
#include <hif.h>
#include "athstartpack.h"
#include "hif_internal.h"
#include "if_sdio.h"

struct hif_sdio_device *hif_dev_from_hif(struct hif_sdio_dev *hif_device);

struct hif_sdio_device *hif_dev_create(struct hif_sdio_dev *hif_device,
				struct hif_msg_callbacks *callbacks,
				void *target);

void hif_dev_destroy(struct hif_sdio_device *htc_sdio_device);

QDF_STATUS hif_dev_setup(struct hif_sdio_device *htc_sdio_device);

QDF_STATUS hif_dev_enable_interrupts(struct hif_sdio_device *htc_sdio_device);

QDF_STATUS hif_dev_disable_interrupts(struct hif_sdio_device *htc_sdio_device);

QDF_STATUS hif_dev_send_buffer(struct hif_sdio_device *htc_sdio_device,
			     unsigned int transfer_id, uint8_t pipe,
			     unsigned int nbytes, qdf_nbuf_t buf);

QDF_STATUS hif_dev_process_pending_irqs(struct hif_sdio_device *pdev,
					bool *done,
					bool *async_processing);

void hif_dev_mask_interrupts(struct hif_sdio_device *pdev);

QDF_STATUS hif_dev_map_service_to_pipe(struct hif_sdio_dev *pdev,
				       uint16_t service_id,
				       uint8_t *ul_pipe,
				       uint8_t *dl_pipe);

void hif_dev_unmask_interrupts(struct hif_sdio_device *pdev);

int hif_dev_setup_device(struct hif_sdio_device *pdev);

int hif_dev_get_fifo_address(struct hif_sdio_dev *pdev,
			     void *config,
			     uint32_t config_len);

void hif_dev_get_block_size(void *config);

void hif_dev_set_mailbox_swap(struct hif_sdio_dev *pdev);

bool hif_dev_get_mailbox_swap(struct hif_sdio_dev *pdev);

QDF_STATUS hif_read_write(struct hif_sdio_dev *device, unsigned long address,
			  char *buffer,	uint32_t length, uint32_t request,
			  void *context);

#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
static inline struct hif_sdio_dev *get_hif_device(struct hif_softc *hif_ctx,
						  struct sdio_func *func)
{
	qdf_assert(func);
	return (struct hif_sdio_dev *)sdio_get_drvdata(func);
}

/**
 * hif_sdio_set_drvdata() - set wlan driver data into upper layer private
 * @hif_ctx: HIF object
 * @func: pointer to sdio function
 * @hifdevice: pointer to hif device
 *
 * Return: zero for success.
 */
static inline int hif_sdio_set_drvdata(struct hif_softc *hif_ctx,
				       struct sdio_func *func,
				       struct hif_sdio_dev *hifdevice)
{
	sdio_set_drvdata(func, hifdevice);
	return 0;
}

static inline int hif_dev_configure_pipes(struct hif_sdio_dev *pdev,
					  struct sdio_func *func)
{
	return 0;
}

static inline int hif_dev_register_channels(struct hif_sdio_dev *dev,
					    struct sdio_func *func)
{
	return 0;
}

static inline void hif_dev_unregister_channels(struct hif_sdio_dev *dev,
					       struct sdio_func *func)
{
}
#else
static inline struct hif_sdio_dev *get_hif_device(struct hif_softc *hif_ctx,
						  struct sdio_func *func)
{
	struct hif_sdio_softc *scn = (struct hif_sdio_softc *)hif_ctx;

	return (struct hif_sdio_dev *)scn->hif_handle;
}

/**
 * hif_sdio_set_drvdata() - set wlan driver data into upper layer private
 * @hif_ctx: HIF object
 * @func: pointer to sdio function
 * @hifdevice: pointer to hif device
 *
 * Return: zero for success.
 */
static inline int hif_sdio_set_drvdata(struct hif_softc *hif_ctx,
				       struct sdio_func *func,
				       struct hif_sdio_dev *hifdevice)
{
	struct hif_sdio_softc *sc = (struct hif_sdio_softc *)hif_ctx;

	sc->hif_handle = hifdevice;

	return 0;
}

int hif_dev_configure_pipes(struct hif_sdio_dev *pdev,
			    struct sdio_func *func);

int hif_dev_register_channels(struct hif_sdio_dev *dev,
			      struct sdio_func *func);

void hif_dev_unregister_channels(struct hif_sdio_dev *dev,
				 struct sdio_func *func);
#endif /* SDIO_TRANSFER */
QDF_STATUS hif_enable_func(struct hif_softc *ol_sc, struct hif_sdio_dev *device,
			   struct sdio_func *func, bool resume);
QDF_STATUS hif_disable_func(struct hif_sdio_dev *device,
			    struct sdio_func *func,
			    bool reset);
A_STATUS hif_sdio_probe(struct hif_softc *ol_sc,
			struct sdio_func *func,
			struct hif_sdio_dev *device);
#endif /* HIF_SDIO_DEV_H_ */

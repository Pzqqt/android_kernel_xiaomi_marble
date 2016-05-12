/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef HIF_SDIO_DEV_H_
#define HIF_SDIO_DEV_H_

#include "qdf_net_types.h"
#include "a_types.h"
#include "athdefs.h"
#include "a_osapi.h"
#include <hif.h>
#include "athstartpack.h"
#include "hif_internal.h"

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

QDF_STATUS hif_dev_map_service_to_pipe(struct hif_sdio_device *pdev,
				       uint16_t service_id,
				       uint8_t *ul_pipe,
				       uint8_t *dl_pipe,
				       bool swap_mapping);

#endif /* HIF_SDIO_DEV_H_ */

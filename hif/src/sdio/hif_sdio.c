/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_timer.h>
#include <qdf_time.h>
#include <qdf_lock.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include <qdf_defer.h>
#include <qdf_atomic.h>
#include <qdf_nbuf.h>
#include <athdefs.h>
#include "qdf_net_types.h"
#include "a_types.h"
#include "athdefs.h"
#include "a_osapi.h"
#include <hif.h>
#include <htc_services.h>
#include <a_debug.h>
#include "hif_sdio_dev.h"
#include "if_sdio.h"
#include "regtable_sdio.h"

#define ATH_MODULE_NAME hif_sdio

/**
 * hif_start() - start hif bus interface.
 * @hif_ctx: HIF context
 *
 * Enables hif device interrupts
 *
 * Return: int
 */
uint32_t hif_start(struct hif_opaque_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct hif_sdio_device *htc_sdio_device = hif_dev_from_hif(hif_device);

	HIF_ENTER();
	hif_dev_enable_interrupts(htc_sdio_device);
	HIF_EXIT();
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_flush_surprise_remove() - remove hif bus interface.
 * @hif_ctx: HIF context
 *
 *
 * Return: none
 */
void hif_flush_surprise_remove(struct hif_opaque_softc *hif_ctx)
{

}

/**
 * hif_sdio_stop() - stop hif bus interface.
 * @hif_ctx: HIF context
 *
 * Disable hif device interrupts and destroy hif context
 *
 * Return: none
 */
void hif_sdio_stop(struct hif_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct hif_sdio_device *htc_sdio_device = hif_dev_from_hif(hif_device);

	HIF_ENTER();
	if (htc_sdio_device) {
		hif_dev_disable_interrupts(htc_sdio_device);
		hif_dev_destroy(htc_sdio_device);
	}
	HIF_EXIT();
}

/**
 * hif_send_head() - send data on hif bus interface.
 * @hif_ctx: HIF context
 *
 * send tx data on a given pipe id
 *
 * Return: int
 */
QDF_STATUS hif_send_head(struct hif_opaque_softc *hif_ctx, uint8_t pipe,
		uint32_t transfer_id, uint32_t nbytes, qdf_nbuf_t buf,
		uint32_t data_attr)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct hif_sdio_device *htc_sdio_device = hif_dev_from_hif(hif_device);

	return hif_dev_send_buffer(htc_sdio_device,
				transfer_id, pipe,
				nbytes, buf);
}

/**
 * hif_map_service_to_pipe() - maps ul/dl pipe to service id.
 * @hif_ctx: HIF hdl
 * @ServiceId: sevice index
 * @ULPipe: uplink pipe id
 * @DLPipe: down-linklink pipe id
 * @ul_is_polled: if ul is polling based
 * @ul_is_polled: if dl is polling based
 *
 * Return: int
 */
int hif_map_service_to_pipe(struct hif_opaque_softc *hif_hdl,
			    uint16_t service_id, uint8_t *ul_pipe,
			    uint8_t *dl_pipe, int *ul_is_polled,
			    int *dl_is_polled)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_hdl);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	return hif_dev_map_service_to_pipe(hif_device,
					   service_id, ul_pipe, dl_pipe);
}

/**
 * hif_map_service_to_pipe() - maps ul/dl pipe to service id.
 * @scn: HIF context
 * @ServiceId: sevice index
 * @ULPipe: uplink pipe id
 * @DLPipe: down-linklink pipe id
 * @ul_is_polled: if ul is polling based
 * @ul_is_polled: if dl is polling based
 *
 * Return: int
 */
void hif_get_default_pipe(struct hif_opaque_softc *scn, uint8_t *ul_pipe,
			  uint8_t *dl_pipe)
{
	hif_map_service_to_pipe(scn, HTC_CTRL_RSVD_SVC,
				ul_pipe, dl_pipe, NULL, NULL);
}

/**
 * hif_post_init() - create hif device after probe.
 * @hif_ctx: HIF context
 * @target: HIF target
 * @callbacks: htc callbacks
 *
 *
 * Return: int
 */
void hif_post_init(struct hif_opaque_softc *hif_ctx, void *target,
		   struct hif_msg_callbacks *callbacks)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct hif_sdio_device *htc_sdio_device = hif_dev_from_hif(hif_device);

	HIF_ENTER();

	if (!htc_sdio_device)
		htc_sdio_device = hif_dev_create(hif_device, callbacks, target);

	if (htc_sdio_device)
		hif_dev_setup(htc_sdio_device);

	HIF_EXIT();
}

/**
 * hif_get_free_queue_number() - create hif device after probe.
 * @hif_ctx: HIF context
 * @pipe: pipe id
 *
 * SDIO uses credit based flow control at the HTC layer
 * so transmit resource checks are bypassed
 * Return: int
 */
uint16_t hif_get_free_queue_number(struct hif_opaque_softc *hif_ctx,
				   uint8_t pipe)
{
	uint16_t rv;

	rv = 1;
	return rv;
}

/**
 * hif_send_complete_check() - check tx complete on a given pipe.
 * @hif_ctx: HIF context
 * @pipe: HIF target
 * @force: check if need to pool for completion
 * Decide whether to actually poll for completions, or just
 * wait for a later chance.
 *
 * Return: int
 */
void hif_send_complete_check(struct hif_opaque_softc *hif_ctx, uint8_t pipe,
				int force)
{

}


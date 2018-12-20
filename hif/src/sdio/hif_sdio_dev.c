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

#define ATH_MODULE_NAME hif
#include "a_debug.h"

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
#include <qdf_net_types.h>
#include <a_types.h>
#include <athdefs.h>
#include <a_osapi.h>
#include <hif.h>
#include <htc_services.h>
#include "hif_sdio_internal.h"
#include "if_sdio.h"
#include "regtable_sdio.h"

/**
 * hif_dev_alloc_rx_buffer() - allocate rx buffer.
 * @pDev: sdio device context
 *
 *
 * Return: htc buffer pointer
 */
HTC_PACKET *hif_dev_alloc_rx_buffer(struct hif_sdio_device *pdev)
{
	HTC_PACKET *packet;
	qdf_nbuf_t netbuf;
	uint32_t bufsize = 0, headsize = 0;

	bufsize = HIF_SDIO_RX_BUFFER_SIZE + HIF_SDIO_RX_DATA_OFFSET;
	headsize = sizeof(HTC_PACKET);
	netbuf = qdf_nbuf_alloc(NULL, bufsize + headsize, 0, 4, false);
	if (!netbuf) {
		hif_err_rl("Allocate netbuf failed");
		return NULL;
	}
	packet = (HTC_PACKET *) qdf_nbuf_data(netbuf);
	qdf_nbuf_reserve(netbuf, headsize);

	SET_HTC_PACKET_INFO_RX_REFILL(packet,
				      pdev,
				      qdf_nbuf_data(netbuf),
				      bufsize, ENDPOINT_0);
	SET_HTC_PACKET_NET_BUF_CONTEXT(packet, netbuf);
	return packet;
}

/**
 * hif_dev_create() - create hif device after probe.
 * @hif_device: HIF context
 * @callbacks: htc callbacks
 * @target: HIF target
 *
 *
 * Return: int
 */
struct hif_sdio_device *hif_dev_create(struct hif_sdio_dev *hif_device,
			struct hif_msg_callbacks *callbacks, void *target)
{

	QDF_STATUS status;
	struct hif_sdio_device *pdev;

	HIF_ENTER();
	pdev = qdf_mem_malloc(sizeof(struct hif_sdio_device));
	if (!pdev) {
		A_ASSERT(false);
		return NULL;
	}

	qdf_spinlock_create(&pdev->Lock);
	qdf_spinlock_create(&pdev->TxLock);
	qdf_spinlock_create(&pdev->RxLock);

	pdev->HIFDevice = hif_device;
	pdev->pTarget = target;
	status = hif_configure_device(NULL, hif_device,
				      HIF_DEVICE_SET_HTC_CONTEXT,
				      (void *)pdev, sizeof(pdev));
	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: set context failed", __func__);

	A_MEMCPY(&pdev->hif_callbacks, callbacks, sizeof(*callbacks));

	HIF_EXIT();
	return pdev;
}

/**
 * hif_dev_destroy() - destroy hif device.
 * @pDev: sdio device context
 *
 *
 * Return: none
 */
void hif_dev_destroy(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;

	status = hif_configure_device(NULL, pdev->HIFDevice,
				      HIF_DEVICE_SET_HTC_CONTEXT,
				      (void *)NULL, 0);
	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: set context failed", __func__);

	qdf_mem_free(pdev);
}

/**
 * hif_dev_from_hif() - get sdio device from hif device.
 * @pDev: hif device context
 *
 *
 * Return: hif sdio device context
 */
struct hif_sdio_device *hif_dev_from_hif(struct hif_sdio_dev *hif_device)
{
	struct hif_sdio_device *pdev = NULL;
	QDF_STATUS status;

	status = hif_configure_device(NULL, hif_device,
				      HIF_DEVICE_GET_HTC_CONTEXT,
				      (void **)&pdev,
				      sizeof(struct hif_sdio_device));
	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: set context failed", __func__);

	return pdev;
}

/**
 * hif_dev_disable_interrupts() - disable hif device interrupts.
 * @pDev: sdio device context
 *
 *
 * Return: int
 */
QDF_STATUS hif_dev_disable_interrupts(struct hif_sdio_device *pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_ENTER();

	hif_dev_mask_interrupts(pdev);

	/* To Do mask the host controller interrupts */
	hif_mask_interrupt(pdev->HIFDevice);

	HIF_EXIT();
	return status;
}

/**
 * hif_dev_enable_interrupts() - enables hif device interrupts.
 * @pDev: sdio device context
 *
 *
 * Return: int
 */
QDF_STATUS hif_dev_enable_interrupts(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;

	HIF_ENTER();

	/* for good measure, make sure interrupt are disabled
	 * before unmasking at the HIF layer.
	 * The rationale here is that between device insertion
	 * (where we clear the interrupts the first time)
	 * and when HTC is finally ready to handle interrupts,
	 * other software can perform target "soft" resets.
	 */
	status = hif_dev_disable_interrupts(pdev);

	/* Unmask the host controller interrupts */
	hif_un_mask_interrupt(pdev->HIFDevice);

	hif_dev_unmask_interrupts(pdev);

	HIF_EXIT();

	return status;
}

/**
 * hif_dev_setup() - set up sdio device.
 * @pDev: sdio device context
 *
 *
 * Return: int
 */
QDF_STATUS hif_dev_setup(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;
	struct htc_callbacks htc_cbs;
	struct hif_sdio_dev *hif_device = pdev->HIFDevice;

	HIF_ENTER();

	status = hif_dev_setup_device(pdev);

	if (status != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: device specific setup failed", __func__);
		return QDF_STATUS_E_INVAL;
	}

	pdev->BlockMask = pdev->BlockSize - 1;
	A_ASSERT((pdev->BlockSize & pdev->BlockMask) == 0);

	/* assume we can process HIF interrupt events asynchronously */
	pdev->HifIRQProcessingMode = HIF_DEVICE_IRQ_ASYNC_SYNC;

	/* see if the HIF layer overrides this assumption */
	hif_configure_device(NULL, hif_device,
			     HIF_DEVICE_GET_IRQ_PROC_MODE,
			     &pdev->HifIRQProcessingMode,
			     sizeof(pdev->HifIRQProcessingMode));

	switch (pdev->HifIRQProcessingMode) {
	case HIF_DEVICE_IRQ_SYNC_ONLY:
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
			("HIF Interrupt processing is SYNC ONLY\n"));
		/* see if HIF layer wants HTC to yield */
		hif_configure_device(NULL, hif_device,
				     HIF_DEVICE_GET_IRQ_YIELD_PARAMS,
				     &pdev->HifIRQYieldParams,
				     sizeof(pdev->HifIRQYieldParams));

		if (pdev->HifIRQYieldParams.recv_packet_yield_count > 0) {
			AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("HIF req of DSR yield per %d RECV packets\n",
				 pdev->HifIRQYieldParams.
				 recv_packet_yield_count));
			pdev->DSRCanYield = true;
		}
		break;
	case HIF_DEVICE_IRQ_ASYNC_SYNC:
		AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
			("HIF Interrupt processing is ASYNC and SYNC\n"));
		break;
	default:
		A_ASSERT(false);
		break;
	}

	pdev->HifMaskUmaskRecvEvent = NULL;

	/* see if the HIF layer implements the mask/unmask recv
	 * events function
	 */
	hif_configure_device(NULL, hif_device,
			     HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC,
			     &pdev->HifMaskUmaskRecvEvent,
			     sizeof(pdev->HifMaskUmaskRecvEvent));

	status = hif_dev_disable_interrupts(pdev);

	qdf_mem_zero(&htc_cbs, sizeof(struct htc_callbacks));
	/* the device layer handles these */
	htc_cbs.rw_compl_handler = hif_dev_rw_completion_handler;
	htc_cbs.dsr_handler = hif_dev_dsr_handler;
	htc_cbs.context = pdev;
	status = hif_attach_htc(pdev->HIFDevice, &htc_cbs);

	HIF_EXIT();
	return status;
}

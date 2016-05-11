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

#define ATH_MODULE_NAME hif
#include "a_debug.h"
#include "hif_usb_internal.h"
#include "if_usb.h"
#include "cds_api.h"
#include "hif_debug.h"

#define IS_BULK_EP(attr) (((attr) & 3) == 0x02)
#define IS_INT_EP(attr) (((attr) & 3) == 0x03)
#define IS_ISOC_EP(attr) (((attr) & 3) == 0x01)
#define IS_DIR_IN(addr) ((addr) & 0x80)

#define IS_FW_CRASH_DUMP(x)(((x == FW_ASSERT_PATTERN) || \
				(x == FW_REG_PATTERN) || \
				((x & FW_RAMDUMP_PATTERN_MASK) ==  \
						FW_RAMDUMP_PATTERN)) ? 1 : 0)

static void usb_hif_post_recv_transfers(HIF_USB_PIPE *recv_pipe,
					int buffer_length);
static void usb_hif_post_recv_bundle_transfers
						(HIF_USB_PIPE *recv_pipe,
						int buffer_length);
static void usb_hif_cleanup_recv_urb(HIF_URB_CONTEXT *urb_context);


/**
 * usb_hif_free_urb_to_pipe() - add urb back to urb list of a pipe
 * @pipe: pointer to HIF_USB_PIPE
 * @urb_context: pointer to HIF_URB_CONTEXT
 *
 * Return: none
 */
static void usb_hif_free_urb_to_pipe(HIF_USB_PIPE *pipe,
						HIF_URB_CONTEXT *urb_context)
{
	qdf_spin_lock_irqsave(&pipe->device->cs_lock);
	pipe->urb_cnt++;
	DL_ListAdd(&pipe->urb_list_head, &urb_context->link);
	qdf_spin_unlock_irqrestore(&pipe->device->cs_lock);
}

/**
 * usb_hif_alloc_urb_from_pipe() - remove urb back from urb list of a pipe
 * @pipe: pointer to HIF_USB_PIPE
 *
 * Return: HIF_URB_CONTEXT urb context removed from the urb list
 */
HIF_URB_CONTEXT *usb_hif_alloc_urb_from_pipe(HIF_USB_PIPE *pipe)
{
	HIF_URB_CONTEXT *urb_context = NULL;
	DL_LIST *item;

	qdf_spin_lock_irqsave(&pipe->device->cs_lock);
	item = dl_list_remove_item_from_head(&pipe->urb_list_head);
	if (item != NULL) {
		urb_context = A_CONTAINING_STRUCT(item, HIF_URB_CONTEXT, link);
		pipe->urb_cnt--;
	}
	qdf_spin_unlock_irqrestore(&pipe->device->cs_lock);

	return urb_context;
}

/**
 * usb_hif_dequeue_pending_transfer() - remove urb from pending xfer list
 * @pipe: pointer to HIF_USB_PIPE
 *
 * Return: HIF_URB_CONTEXT urb context removed from the pending xfer list
 */
static HIF_URB_CONTEXT *usb_hif_dequeue_pending_transfer
							(HIF_USB_PIPE *pipe)
{
	HIF_URB_CONTEXT *urb_context = NULL;
	DL_LIST *item;

	qdf_spin_lock_irqsave(&pipe->device->cs_lock);
	item = dl_list_remove_item_from_head(&pipe->urb_pending_list);
	if (item != NULL)
		urb_context = A_CONTAINING_STRUCT(item, HIF_URB_CONTEXT, link);
	qdf_spin_unlock_irqrestore(&pipe->device->cs_lock);

	return urb_context;
}

/**
 * usb_hif_enqueue_pending_transfer() - add urb to pending xfer list
 * @pipe: pointer to HIF_USB_PIPE
 * @urb_context: pointer to HIF_URB_CONTEXT to be added to the xfer list
 *
 * Return: none
 */
void usb_hif_enqueue_pending_transfer(HIF_USB_PIPE *pipe,
						HIF_URB_CONTEXT *urb_context)
{
	qdf_spin_lock_irqsave(&pipe->device->cs_lock);
	dl_list_insert_tail(&pipe->urb_pending_list, &urb_context->link);
	qdf_spin_unlock_irqrestore(&pipe->device->cs_lock);
}


/**
 * usb_hif_remove_pending_transfer() - remove urb from its own list
 * @urb_context: pointer to HIF_URB_CONTEXT to be removed
 *
 * Return: none
 */
void
usb_hif_remove_pending_transfer(HIF_URB_CONTEXT *urb_context)
{
	qdf_spin_lock_irqsave(&urb_context->pipe->device->cs_lock);
	dl_list_remove(&urb_context->link);
	qdf_spin_unlock_irqrestore(&urb_context->pipe->device->cs_lock);
}

/**
 * usb_hif_alloc_pipe_resources() - allocate urb_cnt urbs to a HIF pipe
 * @pipe: pointer to HIF_USB_PIPE to which resources will be allocated
 * @urb_cnt: number of urbs to be added to the HIF pipe
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
static QDF_STATUS usb_hif_alloc_pipe_resources
					(HIF_USB_PIPE *pipe, int urb_cnt)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;
	HIF_URB_CONTEXT *urb_context;

	DL_LIST_INIT(&pipe->urb_list_head);
	DL_LIST_INIT(&pipe->urb_pending_list);

	for (i = 0; i < urb_cnt; i++) {
		urb_context = qdf_mem_malloc(sizeof(*urb_context));
		if (NULL == urb_context) {
			status = QDF_STATUS_E_NOMEM;
			HIF_ERROR("urb_context is null");
			break;
		}
		urb_context->pipe = pipe;
		urb_context->urb = usb_alloc_urb(0, GFP_KERNEL);

		if (NULL == urb_context->urb) {
			status = QDF_STATUS_E_NOMEM;
			qdf_mem_free(urb_context);
			HIF_ERROR("urb_context->urb is null");
			break;
		}

		/* note we are only allocate the urb contexts here, the actual
		 * URB is
		 * allocated from the kernel as needed to do a transaction
		 */
		pipe->urb_alloc++;

		usb_hif_free_urb_to_pipe(pipe, urb_context);
	}

	HIF_DBG("athusb: alloc resources lpipe:%d hpipe:0x%X urbs:%d",
		pipe->logical_pipe_num,
		pipe->usb_pipe_handle,
		pipe->urb_alloc);
	return status;
}

/**
 * usb_hif_free_pipe_resources() - free urb resources allocated to a HIF pipe
 * @pipe: pointer to HIF_USB_PIPE
 *
 * Return: none
 */
static void usb_hif_free_pipe_resources(HIF_USB_PIPE *pipe)
{
	HIF_URB_CONTEXT *urb_context;

	if (NULL == pipe->device) {
		/* nothing allocated for this pipe */
		HIF_ERROR("pipe->device is null");
		return;
	}

	HIF_TRACE("athusb: free resources lpipe:%d hpipe:0x%X urbs:%d avail:%d",
			 pipe->logical_pipe_num,
			 pipe->usb_pipe_handle, pipe->urb_alloc,
			 pipe->urb_cnt);

	if (pipe->urb_alloc != pipe->urb_cnt) {
		HIF_ERROR("athusb: urb leak! lpipe:%d hpipe:0x%X urbs:%d avail:%d",
				 pipe->logical_pipe_num,
				 pipe->usb_pipe_handle, pipe->urb_alloc,
				 pipe->urb_cnt);
	}

	while (true) {
		urb_context = usb_hif_alloc_urb_from_pipe(pipe);
		if (NULL == urb_context)
			break;

		if (urb_context->buf) {
			qdf_nbuf_free(urb_context->buf);
			urb_context->buf = NULL;
		}

		usb_free_urb(urb_context->urb);
		urb_context->urb = NULL;
		qdf_mem_free(urb_context);
	}

}

/**
 * usb_hif_get_logical_pipe_num() - get pipe number for a particular enpoint
 * @device: pointer to HIF_DEVICE_USB structure
 * @ep_address: endpoint address
 * @urb_count: number of urb resources to be allocated to the pipe
 *
 * Return: uint8_t pipe number corresponding to ep_address
 */
static uint8_t usb_hif_get_logical_pipe_num
					(HIF_DEVICE_USB *device,
					uint8_t ep_address,
					int *urb_count)
{
	uint8_t pipe_num = HIF_USB_PIPE_INVALID;

	switch (ep_address) {
	case USB_EP_ADDR_APP_CTRL_IN:
		pipe_num = HIF_RX_CTRL_PIPE;
		*urb_count = RX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_DATA_IN:
		pipe_num = HIF_RX_DATA_PIPE;
		*urb_count = RX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_INT_IN:
		pipe_num = HIF_RX_INT_PIPE;
		*urb_count = RX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_DATA2_IN:
		pipe_num = HIF_RX_DATA2_PIPE;
		*urb_count = RX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_CTRL_OUT:
		pipe_num = HIF_TX_CTRL_PIPE;
		*urb_count = TX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_DATA_LP_OUT:
		pipe_num = HIF_TX_DATA_LP_PIPE;
		*urb_count = TX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_DATA_MP_OUT:
		pipe_num = HIF_TX_DATA_MP_PIPE;
		*urb_count = TX_URB_COUNT;
		break;
	case USB_EP_ADDR_APP_DATA_HP_OUT:
		pipe_num = HIF_TX_DATA_HP_PIPE;
		*urb_count = TX_URB_COUNT;
		break;
	default:
		/* note: there may be endpoints not currently used */
		break;
	}

	return pipe_num;
}

/**
 * usb_hif_get_logical_pipe_num() - setup urb resources for all pipes
 * @device: pointer to HIF_DEVICE_USB structure
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS usb_hif_setup_pipe_resources(HIF_DEVICE_USB *device)
{
	struct usb_interface *interface = device->interface;
	struct usb_host_interface *iface_desc = interface->cur_altsetting;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	int urbcount;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HIF_USB_PIPE *pipe;
	uint8_t pipe_num;

	/* walk decriptors and setup pipes */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (IS_BULK_EP(endpoint->bmAttributes)) {
			HIF_DBG("%s Bulk Ep:0x%2.2X " "maxpktsz:%d",
				IS_DIR_IN(endpoint->bEndpointAddress) ?
								"RX" : "TX",
				endpoint->bEndpointAddress,
				qdf_le16_to_cpu(endpoint->wMaxPacketSize));
		} else if (IS_INT_EP(endpoint->bmAttributes)) {
			HIF_DBG("%s Int Ep:0x%2.2X maxpktsz:%d interval:%d",
				IS_DIR_IN(endpoint->bEndpointAddress) ?
								"RX" : "TX",
				endpoint->bEndpointAddress,
				qdf_le16_to_cpu(endpoint->wMaxPacketSize),
				endpoint->bInterval);
		} else if (IS_ISOC_EP(endpoint->bmAttributes)) {
			/* TODO for ISO */
			HIF_DBG("%s ISOC Ep:0x%2.2X maxpktsz:%d interval:%d",
				IS_DIR_IN(endpoint->bEndpointAddress) ?
								"RX" : "TX",
				endpoint->bEndpointAddress,
				qdf_le16_to_cpu(endpoint->wMaxPacketSize),
				endpoint->bInterval);
		}
		urbcount = 0;

		pipe_num = usb_hif_get_logical_pipe_num(device,
						endpoint->bEndpointAddress,
						&urbcount);
		if (HIF_USB_PIPE_INVALID == pipe_num)
			continue;

		pipe = &device->pipes[pipe_num];
		if (pipe->device != NULL) {
			/*pipe was already setup */
			continue;
		}

		pipe->device = device;
		pipe->logical_pipe_num = pipe_num;
		pipe->ep_address = endpoint->bEndpointAddress;
		pipe->max_packet_size =
			qdf_le16_to_cpu(endpoint->wMaxPacketSize);

		if (IS_BULK_EP(endpoint->bmAttributes)) {
			if (IS_DIR_IN(pipe->ep_address)) {
				pipe->usb_pipe_handle =
					usb_rcvbulkpipe(device->udev,
							pipe->ep_address);
			} else {
				pipe->usb_pipe_handle =
					usb_sndbulkpipe(device->udev,
						pipe->ep_address);
			}
		} else if (IS_INT_EP(endpoint->bmAttributes)) {
			if (IS_DIR_IN(pipe->ep_address)) {
				pipe->usb_pipe_handle =
					usb_rcvintpipe(device->udev,
						pipe->ep_address);
			} else {
				pipe->usb_pipe_handle =
					usb_sndintpipe(device->udev,
						pipe->ep_address);
			}
		} else if (IS_ISOC_EP(endpoint->bmAttributes)) {
			/* TODO for ISO */
			if (IS_DIR_IN(pipe->ep_address)) {
				pipe->usb_pipe_handle =
					usb_rcvisocpipe(device->udev,
						pipe->ep_address);
			} else {
				pipe->usb_pipe_handle =
					usb_sndisocpipe(device->udev,
						pipe->ep_address);
			}
		}
		pipe->ep_desc = endpoint;

		if (!IS_DIR_IN(pipe->ep_address))
			pipe->flags |= HIF_USB_PIPE_FLAG_TX;

		status = usb_hif_alloc_pipe_resources(pipe, urbcount);

		if (!QDF_IS_STATUS_SUCCESS(status))
			break;

	}

	return status;
}


/**
 * usb_hif_cleanup_pipe_resources() - free urb resources for all pipes
 * @device: pointer to HIF_DEVICE_USB structure
 *
 * Return: none
 */
void usb_hif_cleanup_pipe_resources(HIF_DEVICE_USB *device)
{
	int i;

	for (i = 0; i < HIF_USB_PIPE_MAX; i++)
		usb_hif_free_pipe_resources(&device->pipes[i]);
}

/**
 * usb_hif_flush_pending_transfers() - kill pending urbs for a pipe
 * @pipe: pointer to HIF_USB_PIPE structure
 *
 * Return: none
 */
static void usb_hif_flush_pending_transfers(HIF_USB_PIPE *pipe)
{
	HIF_URB_CONTEXT *urb_context;

	HIF_TRACE("+%s pipe : %d", __func__, pipe->logical_pipe_num);

	while (1) {
		urb_context = usb_hif_dequeue_pending_transfer(pipe);
		if (NULL == urb_context) {
			HIF_WARN("urb_context is NULL");
			break;
		}
		HIF_TRACE("  pending urb ctxt: 0x%p", urb_context);
		if (urb_context->urb != NULL) {
			HIF_TRACE("  killing urb: 0x%p", urb_context->urb);
			/* killing the URB will cause the completion routines to
			 * run
			 */
			usb_kill_urb(urb_context->urb);
		}
	}
	HIF_TRACE("-%s", __func__);
}

/**
 * usb_hif_flush_all() - flush pending transfers for all pipes for a usb bus
 * @device: pointer to HIF_DEVICE_USB structure
 *
 * Return: none
 */
void usb_hif_flush_all(HIF_DEVICE_USB *device)
{
	int i;
	HIF_USB_PIPE *pipe;
	HIF_TRACE("+%s", __func__);

	for (i = 0; i < HIF_USB_PIPE_MAX; i++) {
		if (device->pipes[i].device != NULL) {
			usb_hif_flush_pending_transfers(&device->pipes[i]);
			pipe = &device->pipes[i];

		HIF_USB_FLUSH_WORK(pipe);
		}
	}

	HIF_TRACE("-%s", __func__);
}

/**
 * usb_hif_cleanup_recv_urb() - cleanup recv urb
 * @urb_context: pointer to HIF_URB_CONTEXT structure
 *
 * Return: none
 */
static void usb_hif_cleanup_recv_urb(HIF_URB_CONTEXT *urb_context)
{
	HIF_TRACE("+%s", __func__);

	if (urb_context->buf != NULL) {
		qdf_nbuf_free(urb_context->buf);
		urb_context->buf = NULL;
	}

	usb_hif_free_urb_to_pipe(urb_context->pipe, urb_context);
	HIF_TRACE("-%s", __func__);
}

/**
 * usb_hif_cleanup_transmit_urb() - cleanup transmit urb
 * @urb_context: pointer to HIF_URB_CONTEXT structure
 *
 * Return: none
 */
void usb_hif_cleanup_transmit_urb(HIF_URB_CONTEXT *urb_context)
{
	usb_hif_free_urb_to_pipe(urb_context->pipe, urb_context);
}

/**
 * usb_hif_usb_recv_prestart_complete() - completion routine for prestart rx urb
 * @urb: urb for which the completion routine is being called
 *
 * Return: none
 */
static void usb_hif_usb_recv_prestart_complete
							(struct urb *urb)
{
	HIF_URB_CONTEXT *urb_context = (HIF_URB_CONTEXT *) urb->context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_nbuf_t buf = NULL;
	HIF_USB_PIPE *pipe = urb_context->pipe;

	HIF_DBG("+%s: recv pipe: %d, stat:%d,len:%d urb:0x%p",
		__func__,
		pipe->logical_pipe_num,
		urb->status, urb->actual_length,
		urb);

	/* this urb is not pending anymore */
	usb_hif_remove_pending_transfer(urb_context);

	do {
		if (urb->status != 0) {
			status = A_ECOMM;
			switch (urb->status) {
			case -ECONNRESET:
			case -ENOENT:
			case -ESHUTDOWN:
				/* NOTE: no need to spew these errors when
				 * device is removed
				 * or urb is killed due to driver shutdown
				 */
				status = A_ECANCELED;
				break;
			default:
				HIF_ERROR("%s recv pipe: %d (ep:0x%2.2X), failed:%d",
					__func__,
					pipe->logical_pipe_num,
					pipe->ep_address,
					urb->status);
				break;
			}
			break;
		}

		if (urb->actual_length == 0)
			break;

		buf = urb_context->buf;
		/* we are going to pass it up */
		urb_context->buf = NULL;
		qdf_nbuf_put_tail(buf, urb->actual_length);

		if (AR_DEBUG_LVL_CHECK(USB_HIF_DEBUG_DUMP_DATA)) {
			uint8_t *data;
			uint32_t len;
			qdf_nbuf_peek_header(buf, &data, &len);
			debug_dump_bytes(data, len, "hif recv data");
		}
		/* note: queue implements a lock */
		skb_queue_tail(&pipe->io_comp_queue, buf);

		HIF_USB_SCHEDULE_WORK(pipe);
	} while (false);

	usb_hif_cleanup_recv_urb(urb_context);

	/* Prestart URBs runs out and now start working receive pipe. */
	if (--pipe->urb_prestart_cnt == 0)
		usb_hif_start_recv_pipes(pipe->device);

	HIF_DBG("-%s", __func__);
}

/**
 * usb_hif_usb_recv_complete() - completion routine for rx urb
 * @urb: urb for which the completion routine is being called
 *
 * Return: none
 */
static void usb_hif_usb_recv_complete(struct urb *urb)
{
	HIF_URB_CONTEXT *urb_context = (HIF_URB_CONTEXT *) urb->context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_nbuf_t buf = NULL;
	HIF_USB_PIPE *pipe = urb_context->pipe;
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(pipe->device);

	HIF_DBG("+%s: recv pipe: %d, stat:%d,len:%d urb:0x%p",
		__func__,
		pipe->logical_pipe_num,
		urb->status, urb->actual_length,
		urb);

	/* this urb is not pending anymore */
	usb_hif_remove_pending_transfer(urb_context);

	do {

		if (urb->status != 0) {
			status = A_ECOMM;
			switch (urb->status) {
#ifdef RX_SG_SUPPORT
			case -EOVERFLOW:
				urb->actual_length = HIF_USB_RX_BUFFER_SIZE;
				status = QDF_STATUS_SUCCESS;
				break;
#endif
			case -ECONNRESET:
			case -ENOENT:
			case -ESHUTDOWN:
				/* NOTE: no need to spew these errors when
				 * device is removed
				 * or urb is killed due to driver shutdown
				 */
				status = A_ECANCELED;
				break;
			default:
				HIF_ERROR("%s recv pipe: %d (ep:0x%2.2X), failed:%d",
					__func__,
					pipe->logical_pipe_num,
					pipe->ep_address,
					urb->status);
				break;
			}
			break;
		}

		if (urb->actual_length == 0)
			break;

		buf = urb_context->buf;
		/* we are going to pass it up */
		urb_context->buf = NULL;
		qdf_nbuf_put_tail(buf, urb->actual_length);
		if (AR_DEBUG_LVL_CHECK(USB_HIF_DEBUG_DUMP_DATA)) {
			uint8_t *data;
			uint32_t len;
			qdf_nbuf_peek_header(buf, &data, &len);
			debug_dump_bytes(data, len, "hif recv data");
		}

		/* note: queue implements a lock */
		skb_queue_tail(&pipe->io_comp_queue, buf);
		HIF_USB_SCHEDULE_WORK(pipe);
	} while (false);

	usb_hif_cleanup_recv_urb(urb_context);

	/* Only re-submit URB when STATUS is success and HIF is not at the
	   suspend state.
	 */
	if (QDF_IS_STATUS_SUCCESS(status) && !sc->suspend_state) {
		if (pipe->urb_cnt >= pipe->urb_cnt_thresh) {
			/* our free urbs are piling up, post more transfers */
			usb_hif_post_recv_transfers(pipe,
						HIF_USB_RX_BUFFER_SIZE);
		}
	} else {
		HIF_ERROR("%s:  pipe: %d, fail to post URB: status(%d) suspend (%d)",
				__func__,
				pipe->logical_pipe_num,
				urb->status,
				sc->suspend_state);
	}

	HIF_DBG("-%s", __func__);
}

/**
 * usb_hif_usb_recv_bundle_complete() - completion routine for rx bundling urb
 * @urb: urb for which the completion routine is being called
 *
 * Return: none
 */
static void usb_hif_usb_recv_bundle_complete(struct urb *urb)
{
	HIF_URB_CONTEXT *urb_context = (HIF_URB_CONTEXT *) urb->context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_nbuf_t buf = NULL;
	HIF_USB_PIPE *pipe = urb_context->pipe;
	uint8_t *netdata, *netdata_new;
	uint32_t netlen, netlen_new;
	HTC_FRAME_HDR *HtcHdr;
	uint16_t payloadLen;
	qdf_nbuf_t new_skb = NULL;

	HIF_DBG("+%s: recv pipe: %d, stat:%d,len:%d urb:0x%p",
		__func__,
		pipe->logical_pipe_num,
		urb->status, urb->actual_length,
		urb);

	/* this urb is not pending anymore */
	usb_hif_remove_pending_transfer(urb_context);

	do {

		if (urb->status != 0) {
			status = A_ECOMM;
			switch (urb->status) {
			case -ECONNRESET:
			case -ENOENT:
			case -ESHUTDOWN:
				/* NOTE: no need to spew these errors when
				 * device is removed
				 * or urb is killed due to driver shutdown
				 */
				status = A_ECANCELED;
				break;
			default:
				HIF_ERROR("%s recv pipe: %d (ep:0x%2.2X), failed:%d",
					__func__,
					pipe->logical_pipe_num,
					pipe->ep_address,
					urb->status);
				break;
			}
			break;
		}

		if (urb->actual_length == 0)
			break;

		buf = urb_context->buf;

		if (AR_DEBUG_LVL_CHECK(USB_HIF_DEBUG_DUMP_DATA)) {
			uint8_t *data;
			uint32_t len;
			qdf_nbuf_peek_header(buf, &data, &len);
			debug_dump_bytes(data, len, "hif recv data");
		}

		qdf_nbuf_peek_header(buf, &netdata, &netlen);
		netlen = urb->actual_length;

		do {
			uint16_t frame_len;

			if (IS_FW_CRASH_DUMP(*(uint32_t *) netdata))
				frame_len = netlen;
			else {
				/* Hack into HTC header for bundle processing */
				HtcHdr = (HTC_FRAME_HDR *) netdata;
				if (HtcHdr->EndpointID >= ENDPOINT_MAX) {
					HIF_ERROR("athusb: Rx: invalid EndpointID=%d",
						HtcHdr->EndpointID);
					break;
				}

				payloadLen = HtcHdr->PayloadLen;
				payloadLen = qdf_le16_to_cpu(payloadLen);

				if (payloadLen > HIF_USB_RX_BUFFER_SIZE) {
					HIF_ERROR("athusb: payloadLen too long %u",
						payloadLen);
					break;
				}
				frame_len = (HTC_HDR_LENGTH + payloadLen);
			}

			if (netlen < frame_len) {
				HIF_ERROR("athusb: subframe length %d not fitted into bundle packet length %d"
					, netlen, frame_len);
				break;
			}

			/* allocate a new skb and copy */
			new_skb =
				qdf_nbuf_alloc(NULL, frame_len, 0, 4, false);
			if (new_skb == NULL) {
				HIF_ERROR("athusb: allocate skb (len=%u) failed"
						, frame_len);
				break;
			}

			qdf_nbuf_peek_header(new_skb, &netdata_new,
						&netlen_new);
			qdf_mem_copy(netdata_new, netdata, frame_len);
			qdf_nbuf_put_tail(new_skb, frame_len);
			skb_queue_tail(&pipe->io_comp_queue, new_skb);
			new_skb = NULL;
			netdata += frame_len;
			netlen -= frame_len;
		} while (netlen);
		HIF_USB_SCHEDULE_WORK(pipe);
	} while (false);

	if (urb_context->buf == NULL)
		HIF_ERROR("athusb: buffer in urb_context is NULL");

	/* reset urb_context->buf ==> seems not necessary */
	usb_hif_free_urb_to_pipe(urb_context->pipe, urb_context);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (pipe->urb_cnt >= pipe->urb_cnt_thresh) {
			/* our free urbs are piling up, post more transfers */
			usb_hif_post_recv_bundle_transfers(pipe,
					pipe->device->rx_bundle_buf_len);
		}
	}

	HIF_DBG("-%s", __func__);
}

/**
 * usb_hif_post_recv_prestart_transfers() - post prestart recv urbs for a pipe
 * @recv_pipe: rx data pipe
 * @prestart_urb: number of prestart recv urbs to be posted
 *
 * Return: none
 */
static void usb_hif_post_recv_prestart_transfers
						(HIF_USB_PIPE *recv_pipe,
						int prestart_urb)
{
	HIF_URB_CONTEXT *urb_context;
	uint8_t *data;
	uint32_t len;
	struct urb *urb;
	int i, usb_status, buffer_length = HIF_USB_RX_BUFFER_SIZE;

	HIF_TRACE("+%s", __func__);

	for (i = 0; i < prestart_urb; i++) {
		urb_context = usb_hif_alloc_urb_from_pipe(recv_pipe);
		if (NULL == urb_context)
			break;

		urb_context->buf =
			qdf_nbuf_alloc(NULL, buffer_length, 0, 4, false);
		if (NULL == urb_context->buf) {
			usb_hif_cleanup_recv_urb(urb_context);
			break;
		}

		qdf_nbuf_peek_header(urb_context->buf, &data, &len);

		urb = urb_context->urb;

		usb_fill_bulk_urb(urb,
				recv_pipe->device->udev,
				recv_pipe->usb_pipe_handle,
				data,
				buffer_length,
				usb_hif_usb_recv_prestart_complete,
				urb_context);

		HIF_DBG("athusb bulk recv submit:%d, 0x%X (ep:0x%2.2X), %d bytes, buf:0x%p",
			recv_pipe->logical_pipe_num,
			recv_pipe->usb_pipe_handle,
			recv_pipe->ep_address, buffer_length,
			urb_context->buf);

		usb_hif_enqueue_pending_transfer(recv_pipe, urb_context);

		usb_status = usb_submit_urb(urb, GFP_ATOMIC);

		if (usb_status) {
			HIF_ERROR("athusb : usb bulk recv failed %d",
				usb_status);
			usb_hif_remove_pending_transfer(urb_context);
			usb_hif_cleanup_recv_urb(urb_context);
			break;
		} else
			recv_pipe->urb_prestart_cnt++;

	}

	HIF_TRACE("-%s", __func__);
}

/**
 * usb_hif_post_recv_transfers() - post recv urbs for a given pipe
 * @recv_pipe: recv pipe for which urbs need to be posted
 * @buffer_length: buffer length of the recv urbs
 *
 * Return: none
 */
static void usb_hif_post_recv_transfers(HIF_USB_PIPE *recv_pipe,
							int buffer_length)
{
	HIF_URB_CONTEXT *urb_context;
	uint8_t *data;
	uint32_t len;
	struct urb *urb;
	int usb_status;

	HIF_TRACE("+%s", __func__);

	while (1) {

		urb_context = usb_hif_alloc_urb_from_pipe(recv_pipe);
		if (NULL == urb_context)
			break;

		urb_context->buf = qdf_nbuf_alloc(NULL, buffer_length, 0,
						4, false);
		if (NULL == urb_context->buf) {
			usb_hif_cleanup_recv_urb(urb_context);
			break;
		}

		qdf_nbuf_peek_header(urb_context->buf, &data, &len);

		urb = urb_context->urb;

		usb_fill_bulk_urb(urb,
				recv_pipe->device->udev,
				recv_pipe->usb_pipe_handle,
				data,
				buffer_length,
				usb_hif_usb_recv_complete, urb_context);

		HIF_DBG("athusb bulk recv submit:%d, 0x%X (ep:0x%2.2X), %d bytes, buf:0x%p",
			recv_pipe->logical_pipe_num,
			recv_pipe->usb_pipe_handle,
			recv_pipe->ep_address, buffer_length,
			urb_context->buf);

		usb_hif_enqueue_pending_transfer(recv_pipe, urb_context);

		usb_status = usb_submit_urb(urb, GFP_ATOMIC);

		if (usb_status) {
			HIF_ERROR("athusb : usb bulk recv failed %d",
				usb_status);
			usb_hif_remove_pending_transfer(urb_context);
			usb_hif_cleanup_recv_urb(urb_context);
			break;
		}
	}

	HIF_TRACE("-%s", __func__);

}

/**
 * usb_hif_post_recv_bundle_transfers() - post recv urbs for a given pipe
 * @recv_pipe: recv pipe for which urbs need to be posted
 * @buffer_length: maximum length of rx bundle
 *
 * Return: none
 */
static void usb_hif_post_recv_bundle_transfers
						(HIF_USB_PIPE *recv_pipe,
						int buffer_length)
{
	HIF_URB_CONTEXT *urb_context;
	uint8_t *data;
	uint32_t len;
	struct urb *urb;
	int usb_status;

	HIF_TRACE("+%s", __func__);

	while (1) {

		urb_context = usb_hif_alloc_urb_from_pipe(recv_pipe);
		if (NULL == urb_context)
			break;

		if (NULL == urb_context->buf) {
			urb_context->buf =
			qdf_nbuf_alloc(NULL, buffer_length, 0, 4, false);
			if (NULL == urb_context->buf) {
				usb_hif_cleanup_recv_urb(urb_context);
				break;
			}
		}

		qdf_nbuf_peek_header(urb_context->buf, &data, &len);

		urb = urb_context->urb;
		usb_fill_bulk_urb(urb,
				recv_pipe->device->udev,
				recv_pipe->usb_pipe_handle,
				data,
				buffer_length,
				usb_hif_usb_recv_bundle_complete,
				urb_context);

		HIF_DBG("athusb bulk recv submit:%d, 0x%X (ep:0x%2.2X), %d bytes, buf:0x%p",
			recv_pipe->logical_pipe_num,
			recv_pipe->usb_pipe_handle,
			recv_pipe->ep_address, buffer_length,
			urb_context->buf);

		usb_hif_enqueue_pending_transfer(recv_pipe, urb_context);

		usb_status = usb_submit_urb(urb, GFP_ATOMIC);

		if (usb_status) {
			HIF_ERROR("athusb : usb bulk recv failed %d",
				usb_status);
			usb_hif_remove_pending_transfer(urb_context);
			usb_hif_free_urb_to_pipe(urb_context->pipe,
						urb_context);
			break;
		}

	}

	HIF_TRACE("-%s", __func__);

}

/**
 * usb_hif_prestart_recv_pipes() - post prestart recv urbs
 * @device: HIF device for which prestart recv urbs need to be posted
 *
 * Return: none
 */
void usb_hif_prestart_recv_pipes(HIF_DEVICE_USB *device)
{
	HIF_USB_PIPE *pipe = &device->pipes[HIF_RX_DATA_PIPE];

	/*
	 * USB driver learn to support bundle or not until the firmware
	 * download and ready. Only allocate some URBs for control message
	 * communication during the initial phase then start the final
	 * working pipe after all information understood.
	 */
	usb_hif_post_recv_prestart_transfers(pipe, 8);
}

/**
 * usb_hif_start_recv_pipes() - start recv urbs
 * @device: HIF device for which recv urbs need to be posted
 *
 * This function is called after all prestart recv urbs are exhausted
 *
 * Return: none
 */
void usb_hif_start_recv_pipes(HIF_DEVICE_USB *device)
{
	HIF_USB_PIPE *pipe;
	uint32_t buf_len;

	HIF_ENTER();
	pipe = &device->pipes[HIF_RX_DATA_PIPE];
	pipe->urb_cnt_thresh = pipe->urb_alloc / 2;

	HIF_TRACE("Post URBs to RX_DATA_PIPE: %d",
		device->pipes[HIF_RX_DATA_PIPE].urb_cnt);
	if (device->is_bundle_enabled) {
		usb_hif_post_recv_bundle_transfers(pipe,
					pipe->device->rx_bundle_buf_len);
	} else {
		buf_len = HIF_USB_RX_BUFFER_SIZE;
		usb_hif_post_recv_transfers(pipe, buf_len);
	}

	HIF_DBG("athusb bulk recv len %d", buf_len);

	if (!hif_usb_disable_rxdata2) {
		HIF_TRACE("Post URBs to RX_DATA2_PIPE: %d",
			device->pipes[HIF_RX_DATA2_PIPE].urb_cnt);

		pipe = &device->pipes[HIF_RX_DATA2_PIPE];
		pipe->urb_cnt_thresh = pipe->urb_alloc / 2;
		usb_hif_post_recv_transfers(pipe, HIF_USB_RX_BUFFER_SIZE);
	}

	HIF_EXIT();
}

/**
 * usb_hif_submit_ctrl_out() - send out a ctrl urb
 * @device: HIF device for which urb needs to be posted
 * @req: request value for the ctrl message
 * @value: USB message value
 * @index: USB message index value
 * @data: pointer to data containing ctrl message to send
 * @size: size of the control message to send
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS usb_hif_submit_ctrl_out(HIF_DEVICE_USB *device,
						uint8_t req,
						uint16_t value,
						uint16_t index,
						void *data,
						uint32_t size)
{
	int32_t result = 0;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t *buf = NULL;

	do {

		if (size > 0) {
			buf = qdf_mem_malloc(size);
			if (NULL == buf) {
				ret = QDF_STATUS_E_NOMEM;
				break;
			}
			qdf_mem_copy(buf, (uint8_t *) data, size);
		}

		HIF_DBG("ctrl-out req:0x%2.2X, value:0x%4.4X index:0x%4.4X, datasize:%d",
				req, value, index, size);

		result = usb_control_msg(device->udev,
					usb_sndctrlpipe(device->udev, 0),
					req,
					USB_DIR_OUT | USB_TYPE_VENDOR |
					USB_RECIP_DEVICE, value, index, buf,
					size, 2 * HZ);

		if (result < 0) {
			HIF_ERROR("%s failed,result = %d", __func__, result);
			ret = QDF_STATUS_E_FAILURE;
		}

	} while (false);

	if (buf != NULL)
		qdf_mem_free(buf);

	return ret;
}

/**
 * usb_hif_submit_ctrl_in() - recv a resonse to the ctrl message sent out
 * @device: HIF device for which urb needs to be received
 * @req: request value for the ctrl message
 * @value: USB message value
 * @index: USB message index value
 * @data: pointer to data containing ctrl message to be received
 * @size: size of the control message to be received
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS usb_hif_submit_ctrl_in(HIF_DEVICE_USB *device,
						uint8_t req,
						uint16_t value,
						uint16_t index,
						void *data,
						uint32_t size)
{
	int32_t result = 0;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t *buf = NULL;

	do {

		if (size > 0) {
			buf = qdf_mem_malloc(size);
			if (NULL == buf) {
				ret = QDF_STATUS_E_NOMEM;
				break;
			}
		}

		HIF_DBG("ctrl-in req:0x%2.2X, value:0x%4.4X index:0x%4.4X, datasize:%d",
				 req, value, index, size);

		result = usb_control_msg(device->udev,
					usb_rcvctrlpipe(device->udev, 0),
					req,
					USB_DIR_IN | USB_TYPE_VENDOR |
					USB_RECIP_DEVICE, value, index, buf,
					size, 2 * HZ);

		if (result < 0) {
			HIF_ERROR("%s failed, result = %d", __func__, result);
			ret = QDF_STATUS_E_FAILURE;
			break;
		}

		qdf_mem_copy((uint8_t *) data, buf, size);

	} while (false);

	if (buf != NULL)
		qdf_mem_free(buf);

	return ret;
}

/**
 * usb_hif_io_complete() - transmit call back for tx urb
 * @pipe: pointer to HIF_USB_PIPE
 *
 * Return: none
 */
void usb_hif_io_complete(HIF_USB_PIPE *pipe)
{
	qdf_nbuf_t buf;
	HIF_DEVICE_USB *device;
	HTC_FRAME_HDR *HtcHdr;
	uint8_t *data;
	uint32_t len;
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(pipe->device);
	device = pipe->device;

	HIF_ENTER();

	while ((buf = skb_dequeue(&pipe->io_comp_queue))) {
		if (pipe->flags & HIF_USB_PIPE_FLAG_TX) {
			HIF_DBG("+athusb xmit callback " "buf:0x%p", buf);
			HtcHdr = (HTC_FRAME_HDR *)
					qdf_nbuf_get_frag_vaddr(buf, 0);

#ifdef ATH_11AC_TXCOMPACT
/* ATH_11AC_TXCOMPACT does not support High Latency mode */
#else
			device->htc_callbacks.txCompletionHandler(device->
								htc_callbacks.
								Context, buf,
								HtcHdr->
								EndpointID, 0);
#endif
			HIF_DBG("-athusb xmit callback");
		} else {
			HIF_DBG("+athusb recv callback buf:" "0x%p", buf);
			qdf_nbuf_peek_header(buf, &data, &len);

			if (IS_FW_CRASH_DUMP(*((uint32_t *) data))) {
				sc->fw_data = data;
				sc->fw_data_len = len;
				device->htc_callbacks.fwEventHandler(
					device->htc_callbacks.Context,
					QDF_STATUS_E_USB_ERROR);
				qdf_nbuf_free(buf);
			} else {
				device->htc_callbacks.rxCompletionHandler(
				device->htc_callbacks.Context, buf,
				pipe->logical_pipe_num);
			}
			HIF_DBG("-athusb recv callback");
		}
	}

	HIF_EXIT();
}

#ifdef HIF_USB_TASKLET
/**
 * usb_hif_io_comp_tasklet() - per pipe tasklet routine
 * @context: pointer to HIF USB pipe
 *
 * Return: none
 */
void usb_hif_io_comp_tasklet(long unsigned int context)
{
	HIF_USB_PIPE *pipe = (HIF_USB_PIPE *) context;
	usb_hif_io_complete(pipe);
}

#else
/**
 * usb_hif_io_comp_work() - per pipe work queue
 * @work: pointer to struct work_struct
 *
 * Return: none
 */
void usb_hif_io_comp_work(struct work_struct *work)
{
	HIF_USB_PIPE *pipe = container_of(work, HIF_USB_PIPE, io_complete_work);
	usb_hif_io_complete(pipe);
}
#endif

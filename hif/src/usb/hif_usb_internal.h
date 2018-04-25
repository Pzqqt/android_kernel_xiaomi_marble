/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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

#ifndef _HIF_USB_INTERNAL_H
#define _HIF_USB_INTERNAL_H

#include <qdf_nbuf.h>
#include "a_types.h"
#include "athdefs.h"
#include "a_osapi.h"
#include "a_usb_defs.h"
#include <ol_if_athvar.h>
#include <linux/usb.h>
#include "hif.h"
#include "if_usb.h"

#define TX_URB_COUNT    32
#define RX_URB_COUNT    32

#define HIF_USB_RX_BUFFER_SIZE  (1792 + 8)
#define HIF_USB_RX_BUNDLE_ONE_PKT_SIZE  (1792 + 8)

#ifdef HIF_USB_TASKLET
#define HIF_USB_SCHEDULE_WORK(pipe)\
	tasklet_schedule(&pipe->io_complete_tasklet)

#define HIF_USB_INIT_WORK(pipe)\
		tasklet_init(&pipe->io_complete_tasklet,\
				usb_hif_io_comp_tasklet,\
				(unsigned long)pipe)

#define HIF_USB_FLUSH_WORK(pipe) flush_work(&pipe->io_complete_work)
#else
#define HIF_USB_SCHEDULE_WORK(pipe) schedule_work(&pipe->io_complete_work)
#define HIF_USB_INIT_WORK(pipe)\
		INIT_WORK(&pipe->io_complete_work,\
				usb_hif_io_comp_work)
#define HIF_USB_FLUSH_WORK(pipe)
#endif

/* debug masks */
#define USB_HIF_DEBUG_CTRL_TRANS            ATH_DEBUG_MAKE_MODULE_MASK(0)
#define USB_HIF_DEBUG_BULK_IN               ATH_DEBUG_MAKE_MODULE_MASK(1)
#define USB_HIF_DEBUG_BULK_OUT              ATH_DEBUG_MAKE_MODULE_MASK(2)
#define USB_HIF_DEBUG_ENUM                  ATH_DEBUG_MAKE_MODULE_MASK(3)
#define USB_HIF_DEBUG_DUMP_DATA             ATH_DEBUG_MAKE_MODULE_MASK(4)
#define USB_HIF_SUSPEND                     ATH_DEBUG_MAKE_MODULE_MASK(5)
#define USB_HIF_ISOC_SUPPORT                ATH_DEBUG_MAKE_MODULE_MASK(6)

struct HIF_USB_PIPE;

struct HIF_URB_CONTEXT {
	DL_LIST link;
	struct HIF_USB_PIPE *pipe;
	qdf_nbuf_t buf;
	struct urb *urb;
	struct hif_usb_send_context *send_context;
};

#define HIF_USB_PIPE_FLAG_TX    (1 << 0)

/*
 * Data structure to record required sending context data
 */
struct hif_usb_send_context {
	A_BOOL new_alloc;
	struct HIF_DEVICE_USB *hif_usb_device;
	qdf_nbuf_t netbuf;
	unsigned int transfer_id;
	unsigned int head_data_len;
};

extern unsigned int hif_usb_disable_rxdata2;

extern QDF_STATUS usb_hif_submit_ctrl_in(struct HIF_DEVICE_USB *macp,
				uint8_t req,
				uint16_t value,
				uint16_t index,
				void *data, uint32_t size);

extern QDF_STATUS usb_hif_submit_ctrl_out(struct HIF_DEVICE_USB *macp,
					uint8_t req,
					uint16_t value,
					uint16_t index,
					void *data, uint32_t size);

QDF_STATUS usb_hif_setup_pipe_resources(struct HIF_DEVICE_USB *device);
void usb_hif_cleanup_pipe_resources(struct HIF_DEVICE_USB *device);
void usb_hif_prestart_recv_pipes(struct HIF_DEVICE_USB *device);
void usb_hif_start_recv_pipes(struct HIF_DEVICE_USB *device);
void usb_hif_flush_all(struct HIF_DEVICE_USB *device);
void usb_hif_cleanup_transmit_urb(struct HIF_URB_CONTEXT *urb_context);
void usb_hif_enqueue_pending_transfer(struct HIF_USB_PIPE *pipe,
					struct HIF_URB_CONTEXT *urb_context);
void usb_hif_remove_pending_transfer(struct HIF_URB_CONTEXT *urb_context);
struct HIF_URB_CONTEXT *usb_hif_alloc_urb_from_pipe(struct HIF_USB_PIPE *pipe);
void hif_usb_device_deinit(struct hif_usb_softc *sc);
QDF_STATUS hif_usb_device_init(struct hif_usb_softc *sc);
#ifdef HIF_USB_TASKLET
void usb_hif_io_comp_tasklet(unsigned long context);
#else
void usb_hif_io_comp_work(struct work_struct *work);
#endif
QDF_STATUS hif_diag_write_warm_reset(struct usb_interface *interface,
			uint32_t address, uint32_t data);
#endif

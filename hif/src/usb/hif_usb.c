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

#include <qdf_time.h>
#include <qdf_lock.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include <qdf_defer.h>
#include <qdf_atomic.h>
#include <qdf_nbuf.h>
#include "qdf_net_types.h"
#include <hif_usb_internal.h>
#include <htc_services.h>
#include <hif_debug.h>
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "qdf_module.h"
#include "hif_usb_internal.h"
#include "if_usb.h"
#include "usb_api.h"
#include "target_type.h"

#if defined(WLAN_DEBUG) || defined(DEBUG)
static ATH_DEBUG_MASK_DESCRIPTION g_hif_debug_description[] = {
	{USB_HIF_DEBUG_CTRL_TRANS, "Control Transfers"},
	{USB_HIF_DEBUG_BULK_IN, "BULK In Transfers"},
	{USB_HIF_DEBUG_BULK_OUT, "BULK Out Transfers"},
	{USB_HIF_DEBUG_DUMP_DATA, "Dump data"},
	{USB_HIF_DEBUG_ENUM, "Enumeration"},
};

ATH_DEBUG_INSTANTIATE_MODULE_VAR(hif,
				 "hif",
				 "USB Host Interface",
				 ATH_DEBUG_MASK_DEFAULTS | ATH_DEBUG_INFO |
				 USB_HIF_DEBUG_ENUM,
				 ATH_DEBUG_DESCRIPTION_COUNT
				 (g_hif_debug_description),
				 g_hif_debug_description);

#endif

#ifdef USB_ISOC_SUPPORT
unsigned int hif_usb_isoch_vo = 1;
#else
unsigned int hif_usb_isoch_vo;
#endif
unsigned int hif_usb_disable_rxdata2 = 1;

/**
 * usb_hif_usb_transmit_complete() - completion routing for tx urb's
 * @urb: pointer to urb for which tx completion is called
 *
 * Return: none
 */
static void usb_hif_usb_transmit_complete(struct urb *urb)
{
	struct HIF_URB_CONTEXT *urb_context =
		(struct HIF_URB_CONTEXT *)urb->context;
	qdf_nbuf_t buf;
	struct HIF_USB_PIPE *pipe = urb_context->pipe;
	struct hif_usb_send_context *send_context;

	HIF_DBG("+%s: pipe: %d, stat:%d, len:%d", __func__,
		pipe->logical_pipe_num, urb->status, urb->actual_length);

	/* this urb is not pending anymore */
	usb_hif_remove_pending_transfer(urb_context);

	if (urb->status != 0) {
		hif_err("pipe: %d, failed: %d", pipe->logical_pipe_num,
			urb->status);
	}

	buf = urb_context->buf;
	send_context = urb_context->send_context;

	if (send_context->new_alloc)
		qdf_mem_free(send_context);
	else
		qdf_nbuf_pull_head(buf, send_context->head_data_len);

	urb_context->buf = NULL;
	usb_hif_cleanup_transmit_urb(urb_context);

	/* note: queue implements a lock */
	skb_queue_tail(&pipe->io_comp_queue, buf);
	HIF_USB_SCHEDULE_WORK(pipe);

	HIF_DBG("-%s", __func__);
}

/**
 * hif_send_internal() - HIF internal routine to prepare and submit tx urbs
 * @hif_usb_device: pointer to HIF_DEVICE_USB structure
 * @pipe_id: HIF pipe on which data is to be sent
 * @hdr_buf: any header buf to be prepended, currently ignored
 * @buf: qdf_nbuf_t containing data to be transmitted
 * @nbytes: number of bytes to be transmitted
 *
 * Return: QDF_STATUS_SUCCESS on success and error QDF status on failure
 */
static QDF_STATUS hif_send_internal(struct HIF_DEVICE_USB *hif_usb_device,
				    uint8_t pipe_id,
				    qdf_nbuf_t hdr_buf,
				    qdf_nbuf_t buf, unsigned int nbytes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct HIF_DEVICE_USB *device = hif_usb_device;
	struct HIF_USB_PIPE *pipe = &device->pipes[pipe_id];
	struct HIF_URB_CONTEXT *urb_context;
	uint8_t *data;
	uint32_t len;
	struct urb *urb;
	int usb_status;
	int i;
	struct hif_usb_send_context *send_context;
	uint8_t frag_count;
	uint32_t head_data_len, tmp_frag_count = 0;
	unsigned char *data_ptr;

	HIF_DBG("+%s pipe : %d, buf:0x%pK nbytes %u",
		__func__, pipe_id, buf, nbytes);

	frag_count = qdf_nbuf_get_num_frags(buf);
	if (frag_count == 1) {
		/*
		 * | hif_usb_send_context | netbuf->data
		 */
		head_data_len = sizeof(struct hif_usb_send_context);
	} else if ((frag_count - 1) <= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS) {
		/*
		 * means have extra fragment buf in skb
		 * header data length should be total sending length subtract
		 * internal data length of netbuf
		 * | hif_usb_send_context | fragments except internal buffer |
		 * netbuf->data
		 */
		head_data_len = sizeof(struct hif_usb_send_context);
		while (tmp_frag_count < (frag_count - 1)) {
			head_data_len =
				head_data_len + qdf_nbuf_get_frag_len(buf,
						tmp_frag_count);
			tmp_frag_count = tmp_frag_count + 1;
		}
	} else {
		/* Extra fragments overflow */
		hif_err("Extra fragments count overflow : %d", frag_count);
		status = QDF_STATUS_E_RESOURCES;
		goto err;
	}

	/* Check whether head room is enough to save extra head data */
	if (head_data_len <= qdf_nbuf_headroom(buf)) {
		send_context = (struct hif_usb_send_context *)
		    qdf_nbuf_push_head(buf, head_data_len);
		send_context->new_alloc = false;
	} else {
		send_context =
		    qdf_mem_malloc(sizeof(struct hif_usb_send_context)
				   + head_data_len + nbytes);
		if (!send_context) {
			status = QDF_STATUS_E_NOMEM;
			goto err;
		}
		send_context->new_alloc = true;
	}
	send_context->netbuf = buf;
	send_context->hif_usb_device = hif_usb_device;
	send_context->transfer_id = pipe_id;
	send_context->head_data_len = head_data_len;
	/*
	 * Copy data to head part of netbuf or head of allocated buffer.
	 * if buffer is new allocated, the last buffer should be copied also.
	 * It assume last fragment is internal buffer of netbuf
	 * sometime total length of fragments larger than nbytes
	 */
	data_ptr = (unsigned char *)send_context +
				sizeof(struct hif_usb_send_context);
	for (i = 0;
	     i < (send_context->new_alloc ? frag_count : frag_count - 1); i++) {
		int frag_len = qdf_nbuf_get_frag_len(buf, i);
		unsigned char *frag_addr = qdf_nbuf_get_frag_vaddr(buf, i);

		qdf_mem_copy(data_ptr, frag_addr, frag_len);
		data_ptr += frag_len;
	}
	/* Reset pData pointer and send out */
	data_ptr = (unsigned char *)send_context +
				sizeof(struct hif_usb_send_context);

	urb_context = usb_hif_alloc_urb_from_pipe(pipe);
	if (!urb_context) {
		/* TODO : note, it is possible to run out of urbs if 2
		 * endpoints map to the same pipe ID
		 */
		hif_err("pipe: %d no urbs left. URB Cnt: %d",
			pipe_id, pipe->urb_cnt);
		status = QDF_STATUS_E_RESOURCES;
		goto err;
	}
	urb_context->send_context = send_context;
	urb = urb_context->urb;
	urb_context->buf = buf;
	data = data_ptr;
	len = nbytes;

	usb_fill_bulk_urb(urb,
			  device->udev,
			  pipe->usb_pipe_handle,
			  data,
			  (len % pipe->max_packet_size) ==
			  0 ? (len + 1) : len,
			  usb_hif_usb_transmit_complete, urb_context);

	if ((len % pipe->max_packet_size) == 0)
		/* hit a max packet boundary on this pipe */

	HIF_DBG
	    ("athusb bulk send submit:%d, 0x%X (ep:0x%2.2X), %d bytes",
	     pipe->logical_pipe_num, pipe->usb_pipe_handle,
	     pipe->ep_address, nbytes);

	usb_hif_enqueue_pending_transfer(pipe, urb_context);
	usb_status = usb_submit_urb(urb, GFP_ATOMIC);
	if (usb_status) {
		if (send_context->new_alloc)
			qdf_mem_free(send_context);
		else
			qdf_nbuf_pull_head(buf, head_data_len);
		urb_context->buf = NULL;
		hif_err("athusb: usb bulk transmit failed %d", usb_status);
		usb_hif_remove_pending_transfer(urb_context);
		usb_hif_cleanup_transmit_urb(urb_context);
		status = QDF_STATUS_E_FAILURE;
		goto err;
	}

err:
	if (!QDF_IS_STATUS_SUCCESS(status) &&
				(status != QDF_STATUS_E_RESOURCES)) {
		hif_err("athusb send failed %d", status);
	}

	HIF_DBG("-%s pipe : %d", __func__, pipe_id);

	return status;
}

/**
 * hif_send_head() - HIF routine exposed to upper layers to send data
 * @scn: pointer to hif_opaque_softc structure
 * @pipe_id: HIF pipe on which data is to be sent
 * @transfer_id: endpoint ID on which data is to be sent
 * @nbytes: number of bytes to be transmitted
 * @wbuf: qdf_nbuf_t containing data to be transmitted
 * @hdr_buf: any header buf to be prepended, currently ignored
 * @data_attr: data_attr field from cvg_nbuf_cb of wbuf
 *
 * Return: QDF_STATUS_SUCCESS on success and error QDF status on failure
 */
QDF_STATUS hif_send_head(struct hif_opaque_softc *scn, uint8_t pipe_id,
				uint32_t transfer_id, uint32_t nbytes,
				qdf_nbuf_t wbuf, uint32_t data_attr)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);

	status = hif_send_internal(device, pipe_id, NULL, wbuf, nbytes);
	return status;
}

/**
 * hif_get_free_queue_number() - get # of free TX resources in a given HIF pipe
 * @scn: pointer to hif_opaque_softc structure
 * @pipe_id: HIF pipe which is being polled for free resources
 *
 * Return: # of free resources in pipe_id
 */
uint16_t hif_get_free_queue_number(struct hif_opaque_softc *scn,
				   uint8_t pipe_id)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);
	struct HIF_USB_PIPE *pipe = &device->pipes[pipe_id];
	u16 urb_cnt;

	qdf_spin_lock_irqsave(&pipe->device->cs_lock);
	urb_cnt =  pipe->urb_cnt;
	qdf_spin_unlock_irqrestore(&pipe->device->cs_lock);

	return urb_cnt;
}

/**
 * hif_post_init() - copy HTC callbacks to HIF
 * @scn: pointer to hif_opaque_softc structure
 * @target: pointer to HTC_TARGET structure
 * @callbacks: htc callbacks
 *
 * Return: none
 */
void hif_post_init(struct hif_opaque_softc *scn, void *target,
		struct hif_msg_callbacks *callbacks)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);

	qdf_mem_copy(&device->htc_callbacks, callbacks,
			sizeof(device->htc_callbacks));
}

/**
 * hif_detach_htc() - remove HTC callbacks from HIF
 * @scn: pointer to hif_opaque_softc structure
 *
 * Return: none
 */
void hif_detach_htc(struct hif_opaque_softc *scn)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);

	usb_hif_flush_all(device);
	qdf_mem_zero(&device->htc_callbacks, sizeof(device->htc_callbacks));
}

/**
 * hif_usb_device_deinit() - de- init  HIF_DEVICE_USB, cleanup pipe resources
 * @sc: pointer to hif_usb_softc structure
 *
 * Return: None
 */
void hif_usb_device_deinit(struct hif_usb_softc *sc)
{
	struct HIF_DEVICE_USB *device = &sc->hif_hdl;

	HIF_TRACE("+%s", __func__);

	usb_hif_cleanup_pipe_resources(device);

	if (device->diag_cmd_buffer)
		qdf_mem_free(device->diag_cmd_buffer);

	if (device->diag_resp_buffer)
		qdf_mem_free(device->diag_resp_buffer);

	HIF_TRACE("-%s", __func__);
}

/**
 * hif_usb_device_init() - init  HIF_DEVICE_USB, setup pipe resources
 * @sc: pointer to hif_usb_softc structure
 *
 * Return: QDF_STATUS_SUCCESS on success or a QDF error
 */
QDF_STATUS hif_usb_device_init(struct hif_usb_softc *sc)
{
	int i;
	struct HIF_DEVICE_USB *device = &sc->hif_hdl;
	struct usb_interface *interface = sc->interface;
	struct usb_device *dev = interface_to_usbdev(interface);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct HIF_USB_PIPE *pipe;

	HIF_TRACE("+%s", __func__);

	do {

		qdf_spinlock_create(&(device->cs_lock));
		qdf_spinlock_create(&(device->rx_lock));
		qdf_spinlock_create(&(device->tx_lock));
		qdf_spinlock_create(&device->rx_prestart_lock);
		device->udev = dev;
		device->interface = interface;

		hif_err("device %pK device->udev %pK device->interface %pK",
			device,
			device->udev,
			device->interface);

		for (i = 0; i < HIF_USB_PIPE_MAX; i++) {
			pipe = &device->pipes[i];

			HIF_USB_INIT_WORK(pipe);
			skb_queue_head_init(&pipe->io_comp_queue);
		}

		device->diag_cmd_buffer =
			qdf_mem_malloc(USB_CTRL_MAX_DIAG_CMD_SIZE);
		if (!device->diag_cmd_buffer) {
			status = QDF_STATUS_E_NOMEM;
			break;
		}
		device->diag_resp_buffer =
			qdf_mem_malloc(USB_CTRL_MAX_DIAG_RESP_SIZE);
		if (!device->diag_resp_buffer) {
			status = QDF_STATUS_E_NOMEM;
			break;
		}

		status = usb_hif_setup_pipe_resources(device);

	} while (false);

	if (hif_is_supported_rx_ctrl_pipe(HIF_GET_SOFTC(sc)))
		device->rx_ctrl_pipe_supported = 1;

	if (status != QDF_STATUS_SUCCESS)
		hif_err("abnormal condition (status=%d)", status);

	HIF_TRACE("+%s", __func__);
	return status;
}

/**
 * hif_start() - Enable HIF TX and RX
 * @scn: pointer to hif_opaque_softc structure
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS hif_start(struct hif_opaque_softc *scn)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);
	int i;

	HIF_TRACE("+%s", __func__);
	usb_hif_prestart_recv_pipes(device);

	/* set the TX resource avail threshold for each TX pipe */
	for (i = HIF_TX_CTRL_PIPE; i <= HIF_TX_DATA_HP_PIPE; i++) {
		device->pipes[i].urb_cnt_thresh =
		    device->pipes[i].urb_alloc / 2;
	}

	HIF_TRACE("-%s", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_usb_stop_device() - Stop/flush all HIF communication
 * @scn: pointer to hif_opaque_softc structure
 *
 * Return: none
 */
void hif_usb_stop_device(struct hif_softc *hif_sc)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(hif_sc);

	HIF_TRACE("+%s", __func__);

	usb_hif_flush_all(device);

	HIF_TRACE("-%s", __func__);
}

/**
 * hif_get_default_pipe() - get default pipes for HIF TX/RX
 * @scn: pointer to hif_opaque_softc structure
 * @ul_pipe: pointer to TX pipe
 * @ul_pipe: pointer to TX pipe
 *
 * Return: none
 */
void hif_get_default_pipe(struct hif_opaque_softc *scn, uint8_t *ul_pipe,
			  uint8_t *dl_pipe)
{
	*ul_pipe = HIF_TX_CTRL_PIPE;
	*dl_pipe = HIF_RX_CTRL_PIPE;
}

#if defined(USB_MULTI_IN_TEST) || defined(USB_ISOC_TEST)
/**
 * hif_map_service_to_pipe() - maps ul/dl pipe to service id.
 * @scn: HIF context
 * @svc_id: sevice index
 * @ul_pipe: pointer to uplink pipe id
 * @dl_pipe: pointer to down-linklink pipe id
 * @ul_is_polled: if ul is polling based
 * @ul_is_polled: if dl is polling based
 *
 * Return: status
 */
int hif_map_service_to_pipe(struct hif_opaque_softc *scn, uint16_t svc_id,
			    uint8_t *ul_pipe, uint8_t *dl_pipe,
			    int *ul_is_polled, int *dl_is_polled)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (svc_id) {
	case HTC_CTRL_RSVD_SVC:
	case WMI_CONTROL_SVC:
	case HTC_RAW_STREAMS_SVC:
		*ul_pipe = HIF_TX_CTRL_PIPE;
		*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	case WMI_DATA_BE_SVC:
		*ul_pipe = HIF_TX_DATA_LP_PIPE;
		*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	case WMI_DATA_BK_SVC:
		*ul_pipe = HIF_TX_DATA_MP_PIPE;
		*dl_pipe = HIF_RX_DATA2_PIPE;
		break;
	case WMI_DATA_VI_SVC:
		*ul_pipe = HIF_TX_DATA_HP_PIPE;
		*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	case WMI_DATA_VO_SVC:
		*ul_pipe = HIF_TX_DATA_LP_PIPE;
		*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	default:
		status = QDF_STATUS_E_FAILURE;
		break;
	}

	return qdf_status_to_os_return(status);
}
#else

#ifdef QCA_TX_HTT2_SUPPORT
#define USB_TX_CHECK_HTT2_SUPPORT 1
#else
#define USB_TX_CHECK_HTT2_SUPPORT 0
#endif

/**
 * hif_map_service_to_pipe() - maps ul/dl pipe to service id.
 * @scn: HIF context
 * @svc_id: sevice index
 * @ul_pipe: pointer to uplink pipe id
 * @dl_pipe: pointer to down-linklink pipe id
 * @ul_is_polled: if ul is polling based
 * @ul_is_polled: if dl is polling based
 *
 * Return: status
 */
int hif_map_service_to_pipe(struct hif_opaque_softc *scn, uint16_t svc_id,
			    uint8_t *ul_pipe, uint8_t *dl_pipe,
			    int *ul_is_polled, int *dl_is_polled)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);

	switch (svc_id) {
	case HTC_CTRL_RSVD_SVC:
	case WMI_CONTROL_SVC:
		*ul_pipe = HIF_TX_CTRL_PIPE;
		if (device->rx_ctrl_pipe_supported)
			*dl_pipe = HIF_RX_CTRL_PIPE;
		else
			*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	case WMI_DATA_BE_SVC:
	case WMI_DATA_BK_SVC:
		*ul_pipe = HIF_TX_DATA_LP_PIPE;
		if (hif_usb_disable_rxdata2)
			*dl_pipe = HIF_RX_DATA_PIPE;
		else
			*dl_pipe = HIF_RX_DATA2_PIPE;
		break;
	case WMI_DATA_VI_SVC:
		*ul_pipe = HIF_TX_DATA_MP_PIPE;
		if (hif_usb_disable_rxdata2)
			*dl_pipe = HIF_RX_DATA_PIPE;
		else
			*dl_pipe = HIF_RX_DATA2_PIPE;
		break;
	case WMI_DATA_VO_SVC:
		*ul_pipe = HIF_TX_DATA_HP_PIPE;
		if (hif_usb_disable_rxdata2)
			*dl_pipe = HIF_RX_DATA_PIPE;
		else
			*dl_pipe = HIF_RX_DATA2_PIPE;
		break;
	case HTC_RAW_STREAMS_SVC:
		*ul_pipe = HIF_TX_CTRL_PIPE;
		*dl_pipe = HIF_RX_DATA_PIPE;
		break;
	case HTT_DATA_MSG_SVC:
		*ul_pipe = HIF_TX_DATA_LP_PIPE;
		if (hif_usb_disable_rxdata2)
			*dl_pipe = HIF_RX_DATA_PIPE;
		else
			*dl_pipe = HIF_RX_DATA2_PIPE;
		break;
	case HTT_DATA2_MSG_SVC:
		if (USB_TX_CHECK_HTT2_SUPPORT) {
			*ul_pipe = HIF_TX_DATA_HP_PIPE;
			if (hif_usb_disable_rxdata2)
				*dl_pipe = HIF_RX_DATA_PIPE;
			else
				*dl_pipe = HIF_RX_DATA2_PIPE;
			}
		break;
	default:
		status = QDF_STATUS_E_FAILURE;
		break;
	}

	return qdf_status_to_os_return(status);
}
#endif

/**
 * hif_ctrl_msg_exchange() - send usb ctrl message and receive response
 * @macp: pointer to HIF_DEVICE_USB
 * @send_req_val: USB send message request value
 * @send_msg: pointer to data to send
 * @len: length in bytes of the data to send
 * @response_req_val: USB response message request value
 * @response_msg: pointer to response msg
 * @response_len: length of the response message
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
static QDF_STATUS hif_ctrl_msg_exchange(struct HIF_DEVICE_USB *macp,
					uint8_t send_req_val,
					uint8_t *send_msg,
					uint32_t len,
					uint8_t response_req_val,
					uint8_t *response_msg,
					uint32_t *response_len)
{
	QDF_STATUS status;

	do {

		/* send command */
		status = usb_hif_submit_ctrl_out(macp, send_req_val, 0, 0,
						 send_msg, len);

		if (!QDF_IS_STATUS_SUCCESS(status))
			break;

		if (!response_msg) {
			/* no expected response */
			break;
		}

		/* get response */
		status = usb_hif_submit_ctrl_in(macp, response_req_val, 0, 0,
						response_msg, *response_len);

		if (!QDF_IS_STATUS_SUCCESS(status))
			break;

	} while (false);

	return status;
}

#ifdef WLAN_FEATURE_BMI
/**
 * hif_exchange_bmi_msg() - send/recev ctrl message of type BMI_CMD/BMI_RESP
 * @scn: pointer to hif_opaque_softc
 * @bmi_request: pointer to data to send
 * @request_length: length in bytes of the data to send
 * @bmi_response: pointer to response msg
 * @bmi_response_length: length of the response message
 * @timeout_ms: timeout to wait for response (ignored in current implementation)
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */

QDF_STATUS hif_exchange_bmi_msg(struct hif_opaque_softc *scn,
				qdf_dma_addr_t cmd, qdf_dma_addr_t rsp,
				uint8_t *bmi_request,
				uint32_t request_length,
				uint8_t *bmi_response,
				uint32_t *bmi_response_lengthp,
				uint32_t timeout_ms)
{
	struct HIF_DEVICE_USB *macp = HIF_GET_USB_DEVICE(scn);

	return hif_ctrl_msg_exchange(macp,
				USB_CONTROL_REQ_SEND_BMI_CMD,
				bmi_request,
				request_length,
				USB_CONTROL_REQ_RECV_BMI_RESP,
				bmi_response, bmi_response_lengthp);
}

void hif_register_bmi_callbacks(struct hif_opaque_softc *hif_ctx)
{
}
#endif /* WLAN_FEATURE_BMI */

/**
 * hif_diag_read_access() - Read data from target memory or register
 * @scn: pointer to hif_opaque_softc
 * @address: register address to read from
 * @data: pointer to buffer to store the value read from the register
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS hif_diag_read_access(struct hif_opaque_softc *scn, uint32_t address,
					uint32_t *data)
{
	struct HIF_DEVICE_USB *macp = HIF_GET_USB_DEVICE(scn);
	QDF_STATUS status;
	USB_CTRL_DIAG_CMD_READ *cmd;
	uint32_t respLength;

	cmd = (USB_CTRL_DIAG_CMD_READ *) macp->diag_cmd_buffer;

	qdf_mem_zero(cmd, sizeof(*cmd));
	cmd->Cmd = USB_CTRL_DIAG_CC_READ;
	cmd->Address = address;
	respLength = sizeof(USB_CTRL_DIAG_RESP_READ);

	status = hif_ctrl_msg_exchange(macp,
				USB_CONTROL_REQ_DIAG_CMD,
				(uint8_t *) cmd,
				sizeof(*cmd),
				USB_CONTROL_REQ_DIAG_RESP,
				macp->diag_resp_buffer, &respLength);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		USB_CTRL_DIAG_RESP_READ *pResp =
			(USB_CTRL_DIAG_RESP_READ *) macp->diag_resp_buffer;
		*data = pResp->ReadValue;
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * hif_diag_write_access() - write data to target memory or register
 * @scn: pointer to hif_opaque_softc
 * @address: register address to write to
 * @data: value to be written to the address
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS hif_diag_write_access(struct hif_opaque_softc *scn,
					uint32_t address,
					uint32_t data)
{
	struct HIF_DEVICE_USB *macp = HIF_GET_USB_DEVICE(scn);
	USB_CTRL_DIAG_CMD_WRITE *cmd;

	cmd = (USB_CTRL_DIAG_CMD_WRITE *) macp->diag_cmd_buffer;

	qdf_mem_zero(cmd, sizeof(*cmd));
	cmd->Cmd = USB_CTRL_DIAG_CC_WRITE;
	cmd->Address = address;
	cmd->Value = data;

	return hif_ctrl_msg_exchange(macp,
				USB_CONTROL_REQ_DIAG_CMD,
				(uint8_t *) cmd,
				sizeof(*cmd), 0, NULL, 0);
}

/**
 * hif_dump_info() - dump info about all HIF pipes and endpoints
 * @scn: pointer to hif_opaque_softc
 *
 * Return: none
 */
void hif_dump_info(struct hif_opaque_softc *scn)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);
	struct HIF_USB_PIPE *pipe = NULL;
	struct usb_host_interface *iface_desc = NULL;
	struct usb_endpoint_descriptor *ep_desc;
	uint8_t i = 0;

	for (i = 0; i < HIF_USB_PIPE_MAX; i++) {
		pipe = &device->pipes[i];
		hif_err("PipeIndex: %d URB Cnt: %d PipeHandle: %x",
			i, pipe->urb_cnt,
			pipe->usb_pipe_handle);
		if (usb_pipeisoc(pipe->usb_pipe_handle))
			HIF_INFO("Pipe Type ISOC");
		else if (usb_pipebulk(pipe->usb_pipe_handle))
			HIF_INFO("Pipe Type BULK");
		else if (usb_pipeint(pipe->usb_pipe_handle))
			HIF_INFO("Pipe Type INT");
		else if (usb_pipecontrol(pipe->usb_pipe_handle))
			HIF_INFO("Pipe Type control");
	}

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		ep_desc = &iface_desc->endpoint[i].desc;
		if (ep_desc) {
			HIF_INFO(
				"ep_desc : %pK Index : %d: DescType : %d Addr : %d Maxp : %d Atrrib : %d",
				ep_desc, i, ep_desc->bDescriptorType,
				ep_desc->bEndpointAddress,
				ep_desc->wMaxPacketSize,
				ep_desc->bmAttributes);
			if ((ep_desc) && (usb_endpoint_type(ep_desc) ==
						USB_ENDPOINT_XFER_ISOC)) {
				HIF_INFO("ISOC EP Detected");
			}
		}
	}

}

/**
 * hif_flush_surprise_remove() - Cleanup residual buffers for device shutdown
 * @scn: HIF context
 *
 * Not applicable to USB bus
 *
 * Return: none
 */
void hif_flush_surprise_remove(struct hif_opaque_softc *scn)
{
/* TO DO... */
}

/**
 * hif_diag_read_mem() -read nbytes of data from target memory or register
 * @scn: pointer to hif_opaque_softc
 * @address: register address to read from
 * @data: buffer to store the value read
 * @nbytes: number of bytes to be read from 'address'
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS hif_diag_read_mem(struct hif_opaque_softc *scn,
					 uint32_t address, uint8_t *data,
					 int nbytes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_TRACE("+%s", __func__);

	if ((address & 0x3) || ((uintptr_t)data & 0x3))
		return QDF_STATUS_E_IO;

	while ((nbytes >= 4) &&
		QDF_IS_STATUS_SUCCESS(status =
					hif_diag_read_access(scn,
							address,
							(uint32_t *)data))) {

		nbytes -= sizeof(uint32_t);
		address += sizeof(uint32_t);
		data += sizeof(uint32_t);

	}
	HIF_TRACE("-%s", __func__);
	return status;
}
qdf_export_symbol(hif_diag_read_mem);

/**
 * hif_diag_write_mem() -write  nbytes of data to target memory or register
 * @scn: pointer to hif_opaque_softc
 * @address: register address to write to
 * @data: buffer containing data to be written
 * @nbytes: number of bytes to be written
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
QDF_STATUS hif_diag_write_mem(struct hif_opaque_softc *scn,
					   uint32_t address,
					   uint8_t *data, int nbytes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_TRACE("+%s", __func__);
	if ((address & 0x3) || ((uintptr_t)data & 0x3))
		return QDF_STATUS_E_IO;

	while (nbytes >= 4 &&
		QDF_IS_STATUS_SUCCESS(status =
					hif_diag_write_access(scn,
						address,
						*((uint32_t *)data)))) {

		nbytes -= sizeof(uint32_t);
		address += sizeof(uint32_t);
		data += sizeof(uint32_t);

	}
	HIF_TRACE("-%s", __func__);
	return status;
}

void hif_send_complete_check(struct hif_opaque_softc *scn,
						uint8_t PipeID, int force)
{
	/* NO-OP*/
}

/* diagnostic command defnitions */
#define USB_CTRL_DIAG_CC_READ       0
#define USB_CTRL_DIAG_CC_WRITE      1
#define USB_CTRL_DIAG_CC_WARM_RESET 2

void hif_suspend_wow(struct hif_opaque_softc *scn)
{
	HIF_INFO("HIFsuspendwow - TODO");
}

/**
 * hif_usb_set_bundle_mode() - enable bundling and set default rx bundle cnt
 * @scn: pointer to hif_opaque_softc structure
 * @enabled: flag to enable/disable bundling
 * @rx_bundle_cnt: bundle count to be used for RX
 *
 * Return: none
 */
void hif_usb_set_bundle_mode(struct hif_softc *scn,
					bool enabled, int rx_bundle_cnt)
{
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(scn);

	device->is_bundle_enabled = enabled;
	device->rx_bundle_cnt = rx_bundle_cnt;
	if (device->is_bundle_enabled && (device->rx_bundle_cnt == 0))
		device->rx_bundle_cnt = 1;

	device->rx_bundle_buf_len = device->rx_bundle_cnt *
					HIF_USB_RX_BUNDLE_ONE_PKT_SIZE;

	HIF_DBG("athusb bundle %s cnt %d", enabled ? "enabled" : "disabled",
			rx_bundle_cnt);
}

/**
 * hif_is_supported_rx_ctrl_pipe() - return true if device supports exclusive
 * control pipe in the RX direction.
 * @scn: hif context
 *
 * Return: true if device supports RX control pipe.
 */
bool hif_is_supported_rx_ctrl_pipe(struct hif_softc *scn)
{
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct hif_target_info *tgt_info = hif_get_target_info_handle(hif_hdl);

	switch (tgt_info->target_type) {
	case TARGET_TYPE_QCN7605:
		return true;
	default:
		return false;
	}
}

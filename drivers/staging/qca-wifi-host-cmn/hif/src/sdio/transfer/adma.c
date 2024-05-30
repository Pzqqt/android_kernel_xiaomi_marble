/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#include <qdf_lock.h>
#include "adma.h"
#include "hif_sdio_internal.h"
#include "pld_sdio.h"
#include "if_sdio.h"

/**
 * hif_dev_get_fifo_address() - get the fifo addresses for dma
 * @pdev:  SDIO HIF object
 * @c : FIFO address config pointer
 *
 * Return : 0 for success, non-zero for error
 */
int hif_dev_get_fifo_address(struct hif_sdio_dev *pdev,
			     void *c,
			     uint32_t config_len)
{
	/* SDIO AL handles DMA Addresses */
	return 0;
}

/**
 * hif_dev_get_block_size() - get the adma block size for dma
 * @config : block size config pointer
 *
 * Return : NONE
 */
void hif_dev_get_block_size(void *config)
{
	/* TODO Get block size used by AL Layer in Mission ROM Mode */
	*((uint32_t *)config) = HIF_BLOCK_SIZE; /* QCN_SDIO_MROM_BLK_SZ TODO */
}

/**
 * hif_dev_configure_pipes() - configure pipes
 * @pdev: SDIO HIF object
 * @func: sdio function object
 *
 * Return : 0 for success, non-zero for error
 */
int hif_dev_configure_pipes(struct hif_sdio_dev *pdev, struct sdio_func *func)
{
	/* SDIO AL Configures SDIO Channels */
	return 0;
}

/** hif_dev_set_mailbox_swap() - Set the mailbox swap
 * @pdev : The HIF layer object
 *
 * Return: none
 */
void hif_dev_set_mailbox_swap(struct hif_sdio_dev *pdev)
{
	/* SDIO AL doesn't use mailbox architecture */
}

/** hif_dev_get_mailbox_swap() - Get the mailbox swap setting
 * @pdev : The HIF layer object
 *
 * Return: true or false
 */
bool hif_dev_get_mailbox_swap(struct hif_sdio_dev *pdev)
{
	/* SDIO AL doesn't use mailbox architecture */
	return false;
}

/**
 * hif_dev_dsr_handler() - Synchronous interrupt handler
 *
 * @context: hif send context
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_dsr_handler(void *context)
{
	/* SDIO AL handles interrupts */
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_dev_map_service_to_pipe() - maps ul/dl pipe to service id.
 * @pDev: SDIO HIF object
 * @ServiceId: sevice index
 * @ULPipe: uplink pipe id
 * @DLPipe: down-linklink pipe id
 *
 * Return: 0 on success, error value on invalid map
 */
QDF_STATUS hif_dev_map_service_to_pipe(struct hif_sdio_dev *pdev, uint16_t svc,
				       uint8_t *ul_pipe, uint8_t *dl_pipe)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (svc) {
	case HTT_DATA_MSG_SVC:
		*dl_pipe = 2;
		*ul_pipe = 3;
		break;

	case HTC_CTRL_RSVD_SVC:
	case HTC_RAW_STREAMS_SVC:
		*dl_pipe = 0;
		*ul_pipe = 1;
		break;

	case WMI_DATA_BE_SVC:
	case WMI_DATA_BK_SVC:
	case WMI_DATA_VI_SVC:
	case WMI_DATA_VO_SVC:
		*dl_pipe = 2;
		*ul_pipe = 3;
		break;

	case WMI_CONTROL_SVC:
		*dl_pipe = 0;
		*ul_pipe = 1;
		break;

	default:
		hif_err("Invalid service: %d", svc);
		status = QDF_STATUS_E_INVAL;
		break;
	}
	return status;
}

/**
 * hif_bus_configure() - configure the bus
 * @hif_sc: pointer to the hif context.
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_sdio_bus_configure(struct hif_softc *hif_sc)
{
	struct pld_wlan_enable_cfg cfg;
	enum pld_driver_mode mode;
	uint32_t con_mode = hif_get_conparam(hif_sc);

	if (con_mode == QDF_GLOBAL_FTM_MODE)
		mode = PLD_FTM;
	else if (con_mode == QDF_GLOBAL_COLDBOOT_CALIB_MODE)
		mode = PLD_COLDBOOT_CALIBRATION;
	else if (QDF_IS_EPPING_ENABLED(con_mode))
		mode = PLD_EPPING;
	else
		mode = PLD_MISSION;

	return pld_wlan_enable(hif_sc->qdf_dev->dev, &cfg, mode);
}

/** hif_dev_setup_device() - Setup device specific stuff here required for hif
 * @pdev : HIF layer object
 *
 * return 0 on success, error otherwise
 */
int hif_dev_setup_device(struct hif_sdio_device *pdev)
{
	hif_dev_get_block_size(&pdev->BlockSize);

	return 0;
}

/** hif_dev_mask_interrupts() - Disable the interrupts in the device
 * @pdev SDIO HIF Object
 *
 * Return: NONE
 */
void hif_dev_mask_interrupts(struct hif_sdio_device *pdev)
{
	/* SDIO AL Handles Interrupts */
}

/** hif_dev_unmask_interrupts() - Enable the interrupts in the device
 * @pdev SDIO HIF Object
 *
 * Return: NONE
 */
void hif_dev_unmask_interrupts(struct hif_sdio_device *pdev)
{
	/* SDIO AL Handles Interrupts */
}

/**
 * hif_dev_map_pipe_to_adma_chan() - maps pipe id to adma chan
 * @pdev: The pointer to the hif device object
 * @pipeid: pipe index
 *
 * Return: adma channel handle
 */
struct sdio_al_channel_handle *hif_dev_map_pipe_to_adma_chan
(
struct hif_sdio_device *dev,
uint8_t pipeid
)
{
	struct hif_sdio_dev *pdev = dev->HIFDevice;

	HIF_ENTER();

	if ((pipeid == 0) || (pipeid == 1))
		return pdev->al_chan[0];
	else if ((pipeid == 2) || (pipeid == 3))
		return pdev->al_chan[1];
	else
		return NULL;
}

/**
 * hif_dev_map_adma_chan_to_pipe() - map adma chan to htc pipe
 * @pdev: The pointer to the hif device object
 * @chan: channel number
 * @upload: boolean to decide upload or download
 *
 * Return: Invalid pipe index
 */
uint8_t hif_dev_map_adma_chan_to_pipe(struct hif_sdio_device *pdev,
				      uint8_t chan, bool upload)
{
	hif_info("chan: %u, %s", chan, upload ? "Upload" : "Download");

	if (chan == 0) /* chan 0 is mapped to HTT */
		return upload ? 1 : 0;
	else if (chan == 1) /* chan 1 is mapped to WMI */
		return upload ? 3 : 2;

	return (uint8_t)-1; /* invalid channel id */
}

/**
 * hif_get_send_address() - Get the transfer pipe address
 * @pdev: The pointer to the hif device object
 * @pipe: The pipe identifier
 *
 * Return 0 for success and non-zero for failure to map
 */
int hif_get_send_address(struct hif_sdio_device *pdev,
			 uint8_t pipe, unsigned long *addr)
{
	struct sdio_al_channel_handle *chan = NULL;

	if (!addr)
		return -EINVAL;

	*addr = 0;
	chan = hif_dev_map_pipe_to_adma_chan(pdev, pipe);

	if (!chan)
		return -EINVAL;

	*addr = (unsigned long)chan;

	return 0;
}

/**
 * hif_fixup_write_param() - Tweak the address and length parameters
 * @pdev: The pointer to the hif device object
 * @length: The length pointer
 * @addr: The addr pointer
 *
 * Return: None
 */
void hif_fixup_write_param(struct hif_sdio_dev *pdev, uint32_t req,
			   uint32_t *length, uint32_t *addr)
{
	HIF_ENTER();
	HIF_EXIT();
}

#define HIF_MAX_RX_Q_ALLOC 0 /* TODO */
#define HIF_RX_Q_ALLOC_THRESHOLD 100
QDF_STATUS hif_disable_func(struct hif_sdio_dev *device,
			    struct sdio_func *func,
			    bool reset)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
#if HIF_MAX_RX_Q_ALLOC
	qdf_list_node_t *node;
	struct rx_q_entry *rx_q_elem;
#endif
	HIF_ENTER();

#if HIF_MAX_RX_Q_ALLOC
	qdf_spin_lock_irqsave(&device->rx_q_lock);

	for (; device->rx_q.count; ) {
		qdf_list_remove_back(&device->rx_q, &node);
		rx_q_elem = container_of(node, struct rx_q_entry, entry);
		if (rx_q_elem) {
			if (rx_q_elem->nbuf)
				qdf_nbuf_free(rx_q_elem->nbuf);
			qdf_mem_free(rx_q_elem);
		}
	}
	qdf_destroy_work(0, &device->rx_q_alloc_work);

	qdf_spin_unlock_irqrestore(&device->rx_q_lock);

	qdf_spinlock_destroy(&device->rx_q_lock);
#endif

	status = hif_sdio_func_disable(device, func, reset);
	if (status == QDF_STATUS_SUCCESS)
		device->is_disabled = true;

	cleanup_hif_scatter_resources(device);

	HIF_EXIT();

	return status;
}

/**
 * hif_enable_func() - Enable SDIO function
 *
 * @ol_sc: HIF object pointer
 * @device: HIF device pointer
 * @sdio_func: SDIO function pointer
 * @resume: If this is called from resume or probe
 *
 * Return: 0 in case of success, else error value
 */
QDF_STATUS hif_enable_func(struct hif_softc *ol_sc, struct hif_sdio_dev *device,
			   struct sdio_func *func, bool resume)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (!device) {
		hif_err("HIF device is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!resume)
		ret = hif_sdio_probe(ol_sc, func, device);

#if HIF_MAX_RX_Q_ALLOC
	if (!ret) {
		qdf_list_create(&device->rx_q, HIF_MAX_RX_Q_ALLOC);
		qdf_spinlock_create(&device->rx_q_lock);
		qdf_create_work(0, &device->rx_q_alloc_work,
				hif_sdio_rx_q_alloc, (void *)device);
		device->rx_q_alloc_work_scheduled = true;
		qdf_sched_work(0, &device->rx_q_alloc_work);
	}
#endif
	return ret;
}

/**
 * hif_sdio_get_net_buf() - Get a network buffer from the rx q
 * @dev - HIF device object
 *
 * Return - NULL if out of buffers, else qdf_nbuf_t
 */
#if HIF_MAX_RX_Q_ALLOC
static qdf_nbuf_t hif_sdio_get_nbuf(struct hif_sdio_dev *dev, uint16_t buf_len)
{
	qdf_list_node_t *node;
	qdf_nbuf_t nbuf = NULL;
	qdf_list_t *q = &dev->rx_q;
	struct rx_q_entry *elem = NULL;

	/* TODO - Alloc nbuf based on buf_len */
	qdf_spin_lock_irqsave(&dev->rx_q_lock);

	if (q->count) {
		qdf_list_remove_front(q, &node);
		elem = qdf_container_of(node, struct rx_q_entry, entry);
		nbuf = elem->nbuf;
	} else {
		hif_err("no rx q elements");
	}

	if (q->count <= HIF_RX_Q_ALLOC_THRESHOLD &&
	    !dev->rx_q_alloc_work_scheduled) {
		dev->rx_q_alloc_work_scheduled = true;
		qdf_sched_work(0, &dev->rx_q_alloc_work);
	}

	qdf_spin_unlock_irqrestore(&dev->rx_q_lock);

	qdf_mem_free(elem);

	return nbuf;
}
#else
static qdf_nbuf_t hif_sdio_get_nbuf(struct hif_sdio_dev *dev, uint16_t buf_len)
{
	qdf_nbuf_t nbuf;

	if (!buf_len)
		buf_len = HIF_SDIO_RX_BUFFER_SIZE;

	nbuf = qdf_nbuf_alloc(NULL, buf_len, 0, 4, false);

	return nbuf;
}
#endif
/**
 * hif_sdio_rx_q_alloc() - Deferred work for pre-alloc rx q
 * @ctx - Pointer to context object
 *
 * Return NONE
 */
#if HIF_MAX_RX_Q_ALLOC
void hif_sdio_rx_q_alloc(void *ctx)
{
	struct rx_q_entry *rx_q_elem;
	struct hif_sdio_dev *dev = (struct hif_sdio_dev *)ctx;
	unsigned int rx_q_count = dev->rx_q.count;

	HIF_ENTER();
	qdf_spin_lock_irqsave(&dev->rx_q_lock);

	for (; rx_q_count < dev->rx_q.max_size; rx_q_count++) {
		rx_q_elem = qdf_mem_malloc(sizeof(struct rx_q_entry));
		if (!rx_q_elem) {
			hif_err("Failed to alloc rx q elem");
			break;
		}

		/* TODO - Alloc nbuf based on payload_len in HTC Header */
		rx_q_elem->nbuf = qdf_nbuf_alloc(NULL, HIF_SDIO_RX_BUFFER_SIZE,
						 0, 4, false);
		if (!rx_q_elem->nbuf) {
			hif_err("Failed to alloc nbuf for rx");
			qdf_mem_free(rx_q_elem);
			break;
		}

		qdf_list_insert_back(&dev->rx_q, &rx_q_elem->entry);
	}
	dev->rx_q_alloc_work_scheduled = false;

	qdf_spin_unlock_irqrestore(&dev->rx_q_lock);
	HIF_EXIT();
}
#else
void hif_sdio_rx_q_alloc(void *ctx)
{
}
#endif

#include <linux/qcn_sdio_al.h>

struct sdio_al_channel_data qcn7605_chan[HIF_SDIO_MAX_AL_CHANNELS] = {
	{
		.name = "SDIO_AL_WLAN_CH0", /* HTT */
		.client_data = NULL, /* populate from client handle */
		.ul_xfer_cb = ul_xfer_cb,
		.dl_xfer_cb = dl_xfer_cb,
		.dl_data_avail_cb = dl_data_avail_cb,
		.dl_meta_data_cb = NULL
	},
	{
		.name = "SDIO_AL_WLAN_CH1", /* WMI */
		.client_data = NULL, /* populate from client handle */
		.ul_xfer_cb = ul_xfer_cb,
		.dl_xfer_cb = dl_xfer_cb,
		.dl_data_avail_cb = dl_data_avail_cb,
		.dl_meta_data_cb = NULL
	}
};

/**
 * hif_dev_register_channels()- Register transport layer channels
 * @dev  : HIF device object
 * @func : SDIO function pointer
 *
 * Return : success on configuration, else failure
 */
int hif_dev_register_channels(struct hif_sdio_dev *dev, struct sdio_func *func)
{
	int ret = 0;
	unsigned int chan;
	struct sdio_al_channel_data *chan_data[HIF_ADMA_MAX_CHANS];

	HIF_ENTER();

	dev->al_client = pld_sdio_get_sdio_al_client_handle(func);
	if (ret || !dev->al_client) {
		hif_err("Failed to get get sdio al handle");
		return ret;
	}

	if ((func->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
	    MANUFACTURER_ID_QCN7605_BASE) {
		dev->adma_chans_used = 2;
		qcn7605_chan[0].client_data = dev->al_client->client_data;
		qcn7605_chan[1].client_data = dev->al_client->client_data;
		chan_data[0] = &qcn7605_chan[0];
		chan_data[1] = &qcn7605_chan[1];
	} else {
		dev->adma_chans_used = 0;
	}

	for (chan = 0; chan < dev->adma_chans_used; chan++) {
		dev->al_chan[chan] =
		pld_sdio_register_sdio_al_channel(dev->al_client,
						  chan_data[chan]);
		if (!dev->al_chan[chan] || IS_ERR(dev->al_chan[chan])) {
			ret = -EINVAL;
			hif_err("Channel registration failed");
		} else {
			dev->al_chan[chan]->priv = (void *)dev;
			hif_info("chan %s : id : %u",
				 chan_data[chan]->name,
				 dev->al_chan[chan]->channel_id);
		}
	}

	HIF_EXIT();

	return ret;
}

/**
 * hif_dev_unregister_channels()- Register transport layer channels
 * @dev  : HIF device object
 * @func : SDIO Function pointer
 *
 * Return : None
 */
void hif_dev_unregister_channels(struct hif_sdio_dev *dev,
				 struct sdio_func *func)
{
	unsigned int chan;

	if (!dev) {
		hif_err("hif_sdio_dev is null");
		return;
	}

	for (chan = 0; chan < dev->adma_chans_used; chan++) {
		dev->al_chan[chan]->priv = NULL;
		pld_sdio_unregister_sdio_al_channel(dev->al_chan[chan]);
	}
}

/**
 * hif_read_write() - queue a read/write request
 * @dev: pointer to hif device structure
 * @address: address to read, actually channel pointer
 * @buffer: buffer to hold read/write data
 * @length: length to read/write
 * @request: read/write/sync/async request
 * @context: pointer to hold calling context
 *
 * Return: 0, pending  on success, error number otherwise.
 */
QDF_STATUS
hif_read_write(struct hif_sdio_dev *dev,
	       unsigned long sdio_al_ch_handle,
	       char *cbuffer, uint32_t length,
	       uint32_t request, void *context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sdio_al_channel_handle *ch;
	struct bus_request *bus_req;
	enum sdio_al_dma_direction dir;
	struct hif_sdio_device *device;
	QDF_STATUS (*rx_comp)(void *, qdf_nbuf_t, uint8_t);
	qdf_nbuf_t nbuf;
	int ret = 0, payload_len = 0;
	unsigned char *buffer = (unsigned char *)cbuffer;

	if (!dev || !sdio_al_ch_handle) {
		hif_err("Device = %pK, addr = %lu", dev, sdio_al_ch_handle);
		return QDF_STATUS_E_INVAL;
	}

	if (!(request & HIF_ASYNCHRONOUS) &&
	    !(request & HIF_SYNCHRONOUS)) {
		hif_err("Invalid request mode: %d", request);
		return QDF_STATUS_E_INVAL;
	}

	/*sdio r/w action is not needed when suspend, so just return */
	if ((dev->is_suspend) &&
	    (dev->power_config == HIF_DEVICE_POWER_CUT)) {
		hif_info("skip in suspend");
		return QDF_STATUS_SUCCESS;
	}

	ch = (struct sdio_al_channel_handle *)sdio_al_ch_handle;

	bus_req = hif_allocate_bus_request(dev);
	if (!bus_req) {
		hif_err("Bus alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	bus_req->address = sdio_al_ch_handle;
	bus_req->length = length;
	bus_req->request = request;
	bus_req->context = context;
	bus_req->buffer = buffer;

	/* Request SDIO AL to do transfer */
	dir = (request & HIF_SDIO_WRITE) ? SDIO_AL_TX : SDIO_AL_RX;

	if (request & HIF_SYNCHRONOUS) {
		ret = sdio_al_queue_transfer(ch,
					     dir,
					     bus_req->buffer,
					     bus_req->length,
					     1); /* higher priority */
		if (ret) {
			status = QDF_STATUS_E_FAILURE;
			hif_err("SYNC REQ failed ret: %d", ret);
		} else {
			status = QDF_STATUS_SUCCESS;
		}

		hif_free_bus_request(dev, bus_req);

		if ((status == QDF_STATUS_SUCCESS) && (dir == SDIO_AL_RX)) {
			nbuf = (qdf_nbuf_t)context;
			payload_len = HTC_GET_FIELD(bus_req->buffer,
						    HTC_FRAME_HDR,
						    PAYLOADLEN);
			qdf_nbuf_set_pktlen(nbuf, payload_len + HTC_HDR_LENGTH);
			device = (struct hif_sdio_device *)dev->htc_context;
			rx_comp = device->hif_callbacks.rxCompletionHandler;
			rx_comp(device->hif_callbacks.Context, nbuf, 0);
		}
	} else {
		ret = sdio_al_queue_transfer_async(ch,
						   dir,
						   bus_req->buffer,
						   bus_req->length,
						   1, /* higher priority */
						   (void *)bus_req);
		if (ret) {
			status = QDF_STATUS_E_FAILURE;
			hif_err("ASYNC REQ fail ret: %d for len: %d ch: %d",
				ret, length, ch->channel_id);
			hif_free_bus_request(dev, bus_req);
		} else {
			status = QDF_STATUS_E_PENDING;
		}
	}
	return status;
}

/**
 * ul_xfer_cb() - Completion call back for asynchronous transfer
 * @ch_handle: The sdio al channel handle
 * @result: The result of the operation
 * @context: pointer to request context
 *
 * Return: None
 */
void ul_xfer_cb(struct sdio_al_channel_handle *ch_handle,
		struct sdio_al_xfer_result *result,
		void *ctx)
{
	struct bus_request *req = (struct bus_request *)ctx;
	struct hif_sdio_dev *dev;

	if (!ch_handle || !result) {
		hif_err("Invalid args");
		qdf_assert_always(0);
		return;
	}

	dev = (struct hif_sdio_dev *)ch_handle->priv;

	if (result->xfer_status) {
		req->status = QDF_STATUS_E_FAILURE;
		hif_err("ASYNC Tx failed status: %d", result->xfer_status);
	} else {
		req->status = QDF_STATUS_SUCCESS;
	}

	dev->htc_callbacks.rw_compl_handler(req->context, req->status);

	hif_free_bus_request(dev, req);
}

/**
 * dl_data_avail_cb() - Called when data is available on a channel
 * @ch_handle: The sdio al channel handle
 * @len: The len of data available to download
 *
 * Return: None
 */
/* Use the asynchronous method of transfer. This will help in
 * completing READ in the transfer done callback later which
 * runs in sdio al thread context. If we do the syncronous
 * transfer here, the thread context won't be available and
 * perhaps a new thread may be required here.
 */
void dl_data_avail_cb(struct sdio_al_channel_handle *ch_handle,
		      unsigned int len)
{
	struct hif_sdio_dev *dev;
	unsigned int chan;
	qdf_nbuf_t nbuf;

	if (!ch_handle || !len) {
		hif_err("Invalid args %u", len);
		qdf_assert_always(0);
		return;
	}

	dev = (struct hif_sdio_dev *)ch_handle->priv;
	chan = ch_handle->channel_id;

	if (chan > HIF_SDIO_MAX_AL_CHANNELS) {
		hif_err("Invalid Ch ID %d", chan);
		return;
	}

	/* allocate a buffer for reading the data from the chip.
	 * Note that this is raw, unparsed buffer and will be
	 * processed in the transfer done callback.
	 */
	/* TODO, use global buffer instead of runtime allocations */
	nbuf = qdf_nbuf_alloc(NULL, len, 0, 4, false);

	if (!nbuf) {
		hif_err("Unable to alloc netbuf %u bytes", len);
		return;
	}

	hif_read_write(dev, (unsigned long)ch_handle, nbuf->data, len,
		       HIF_RD_ASYNC_BLOCK_FIX, nbuf);
}

#define is_pad_block(buf)	(*((uint32_t *)buf) == 0xbabababa)
uint16_t g_dbg_payload_len;

/**
 * dl_xfer_cb() - Call from lower layer after transfer is completed
 * @ch_handle: The sdio al channel handle
 * @result: The xfer result
 * @ctx: Context passed in the transfer queuing
 *
 * Return: None
 */
void dl_xfer_cb(struct sdio_al_channel_handle *ch_handle,
		struct sdio_al_xfer_result *result,
		void *ctx)
{
	unsigned char *buf;
	qdf_nbuf_t nbuf;
	uint32_t len;
	uint16_t payload_len = 0;
	struct hif_sdio_dev *dev;
	struct hif_sdio_device *device;
	struct bus_request *bus_req = (struct bus_request *)ctx;
	QDF_STATUS (*rx_completion)(void *, qdf_nbuf_t, uint8_t);

	if (!bus_req) {
		hif_err("Bus Req NULL!!!");
		qdf_assert_always(0);
		return;
	}

	if (!ch_handle || !result) {
		hif_err("Invalid args %pK %pK", ch_handle, result);
		qdf_assert_always(0);
		return;
	}

	dev = (struct hif_sdio_dev *)ch_handle->priv;
	if (result->xfer_status) {
		hif_err("ASYNC Rx failed %d", result->xfer_status);
		qdf_nbuf_free((qdf_nbuf_t)bus_req->context);
		hif_free_bus_request(dev, bus_req);
		return;
	}

	device = (struct hif_sdio_device *)dev->htc_context;
	rx_completion = device->hif_callbacks.rxCompletionHandler;

	buf = (unsigned char *)result->buf_addr;
	len = (unsigned int)result->xfer_len;

	while (len >= sizeof(HTC_FRAME_HDR)) {
		if (is_pad_block(buf)) {
			/* End of Rx Buffer */
			break;
		}

		if (HTC_GET_FIELD(buf, HTC_FRAME_HDR, ENDPOINTID) >=
		    ENDPOINT_MAX) {
			hif_err("Invalid endpoint id: %u",
				HTC_GET_FIELD(buf, HTC_FRAME_HDR, ENDPOINTID));
			break;
		}

		/* Copy the HTC frame to the alloc'd packet buffer */
		payload_len = HTC_GET_FIELD(buf, HTC_FRAME_HDR, PAYLOADLEN);
		payload_len = qdf_le16_to_cpu(payload_len);
		if (!payload_len) {
			hif_err("Invalid Payload len %d bytes", payload_len);
			break;
		}
		if (payload_len > g_dbg_payload_len) {
			g_dbg_payload_len = payload_len;
			hif_err("Max Rx HTC Payload = %d", g_dbg_payload_len);
		}

		nbuf = hif_sdio_get_nbuf(dev, payload_len + HTC_HEADER_LEN);
		if (!nbuf) {
			hif_err("Failed to alloc rx buffer");
			break;
		}

		/* Check if payload fits in skb */
		if (qdf_nbuf_tailroom(nbuf) < payload_len + HTC_HEADER_LEN) {
			hif_err("Payload + HTC_HDR %d > skb tailroom %d",
				(payload_len + 8),
				qdf_nbuf_tailroom(nbuf));
			qdf_nbuf_free(nbuf);
			break;
		}

		qdf_mem_copy((uint8_t *)qdf_nbuf_data(nbuf), buf,
			     payload_len + HTC_HEADER_LEN);

		qdf_nbuf_put_tail(nbuf, payload_len + HTC_HDR_LENGTH);

		rx_completion(device->hif_callbacks.Context, nbuf,
			      0); /* don't care, not used */

		len -= payload_len + HTC_HDR_LENGTH;
		buf += payload_len + HTC_HDR_LENGTH;
	}

	qdf_nbuf_free((qdf_nbuf_t)bus_req->context);
	hif_free_bus_request(dev, bus_req);
}

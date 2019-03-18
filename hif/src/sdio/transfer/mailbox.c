/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
#define ATH_MODULE_NAME hif
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
#include <qdf_threads.h>
#include <athdefs.h>
#include <qdf_net_types.h>
#include <a_types.h>
#include <athdefs.h>
#include <a_osapi.h>
#include <hif.h>
#include <htc_internal.h>
#include <htc_services.h>
#include <a_debug.h>
#include "hif_sdio_internal.h"
#include "if_sdio.h"
#include "regtable.h"
#include "transfer.h"

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
	case MANUFACTURER_ID_AR6320_BASE:
	{
		uint16_t rev = manf_id & MANUFACTURER_ID_AR6K_REV_MASK;

		pinfo->mbox_prop[0].extended_address =
			HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320;
		if (rev < 4)
			pinfo->mbox_prop[0].extended_size =
				HIF_MBOX0_EXTENDED_WIDTH_AR6320;
		else
			set_extended_mbox_size(pinfo);
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
 * hif_dev_get_fifo_address() - get the fifo addresses for dma
 * @pdev:  SDIO HIF object
 * @config: mbox address config pointer
 *
 * Return : 0 for success, non-zero for error
 */
QDF_STATUS hif_dev_get_fifo_address(struct hif_sdio_dev *pdev,
				    struct hif_device_mbox_info *config,
				    uint32_t config_len)
{
	uint32_t count;

	for (count = 0; count < 4; count++)
		config->mbox_addresses[count] = HIF_MBOX_START_ADDR(count);

	if (config_len >= sizeof(struct hif_device_mbox_info)) {
		set_extended_mbox_window_info((uint16_t)pdev->func->device,
					      config);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_INVAL;
}

/**
 * hif_dev_get_block_size() - get the mbox block size for dma
 * @config : mbox size config pointer
 *
 * Return : NONE
 */
void hif_dev_get_block_size(void *config)
{
	((uint32_t *)config)[0] = HIF_MBOX0_BLOCK_SIZE;
	((uint32_t *)config)[1] = HIF_MBOX1_BLOCK_SIZE;
	((uint32_t *)config)[2] = HIF_MBOX2_BLOCK_SIZE;
	((uint32_t *)config)[3] = HIF_MBOX3_BLOCK_SIZE;
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
		if (hif_dev_get_mailbox_swap(pdev)) {
			*ul_pipe = 1;
			*dl_pipe = 0;
		} else {
			*ul_pipe = 3;
			*dl_pipe = 2;
		}
		break;

	case HTC_CTRL_RSVD_SVC:
	case HTC_RAW_STREAMS_SVC:
		*ul_pipe = 1;
		*dl_pipe = 0;
		break;

	case WMI_DATA_BE_SVC:
	case WMI_DATA_BK_SVC:
	case WMI_DATA_VI_SVC:
	case WMI_DATA_VO_SVC:
		*ul_pipe = 1;
		*dl_pipe = 0;
		break;

	case WMI_CONTROL_SVC:
		if (hif_dev_get_mailbox_swap(pdev)) {
			*ul_pipe = 3;
			*dl_pipe = 2;
		} else {
			*ul_pipe = 1;
			*dl_pipe = 0;
		}
		break;

	default:
		HIF_ERROR("%s: Err : Invalid service (%d)",
			  __func__, svc);
		status = QDF_STATUS_E_INVAL;
		break;
	}
	return status;
}

/** hif_dev_setup_device() - Setup device specific stuff here required for hif
 * @pdev : HIF layer object
 *
 * return 0 on success, error otherwise
 */
int hif_dev_setup_device(struct hif_sdio_device *pdev)
{
	int status = 0;
	uint32_t blocksizes[MAILBOX_COUNT];

	status = hif_configure_device(NULL, pdev->HIFDevice,
				      HIF_DEVICE_GET_FIFO_ADDR,
				      &pdev->MailBoxInfo,
				      sizeof(pdev->MailBoxInfo));

	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: HIF_DEVICE_GET_MBOX_ADDR failed", __func__);

	status = hif_configure_device(NULL, pdev->HIFDevice,
				      HIF_DEVICE_GET_BLOCK_SIZE,
				      blocksizes, sizeof(blocksizes));
	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: HIF_DEVICE_GET_MBOX_BLOCK_SIZE fail", __func__);

	pdev->BlockSize = blocksizes[MAILBOX_FOR_BLOCK_SIZE];

	return status;
}

/** hif_dev_mask_interrupts() - Disable the interrupts in the device
 * @pdev SDIO HIF Object
 *
 * Return: NONE
 */
void hif_dev_mask_interrupts(struct hif_sdio_device *pdev)
{
	int status = QDF_STATUS_SUCCESS;

	HIF_ENTER();
	/* Disable all interrupts */
	LOCK_HIF_DEV(pdev);
	mboxEnaRegs(pdev).int_status_enable = 0;
	mboxEnaRegs(pdev).cpu_int_status_enable = 0;
	mboxEnaRegs(pdev).error_status_enable = 0;
	mboxEnaRegs(pdev).counter_int_status_enable = 0;
	UNLOCK_HIF_DEV(pdev);

	/* always synchronous */
	status = hif_read_write(pdev->HIFDevice,
				INT_STATUS_ENABLE_ADDRESS,
				(char *)&mboxEnaRegs(pdev),
				sizeof(struct MBOX_IRQ_ENABLE_REGISTERS),
				HIF_WR_SYNC_BYTE_INC, NULL);

	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: Err updating intr reg: %d", __func__, status);
}

/** hif_dev_unmask_interrupts() - Enable the interrupts in the device
 * @pdev SDIO HIF Object
 *
 * Return: NONE
 */
void hif_dev_unmask_interrupts(struct hif_sdio_device *pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	LOCK_HIF_DEV(pdev);

	/* Enable all the interrupts except for the internal
	 * AR6000 CPU interrupt
	 */
	mboxEnaRegs(pdev).int_status_enable =
		INT_STATUS_ENABLE_ERROR_SET(0x01) |
		INT_STATUS_ENABLE_CPU_SET(0x01)
		| INT_STATUS_ENABLE_COUNTER_SET(0x01);

	/* enable 2 mboxs INT */
	mboxEnaRegs(pdev).int_status_enable |=
		INT_STATUS_ENABLE_MBOX_DATA_SET(0x01) |
		INT_STATUS_ENABLE_MBOX_DATA_SET(0x02);

	/* Set up the CPU Interrupt Status Register, enable
	 * CPU sourced interrupt #0, #1.
	 * #0 is used for report assertion from target
	 * #1 is used for inform host that credit arrived
	 */
	mboxEnaRegs(pdev).cpu_int_status_enable = 0x03;

	/* Set up the Error Interrupt Status Register */
	mboxEnaRegs(pdev).error_status_enable =
		(ERROR_STATUS_ENABLE_RX_UNDERFLOW_SET(0x01)
		 | ERROR_STATUS_ENABLE_TX_OVERFLOW_SET(0x01)) >> 16;

	/* Set up the Counter Interrupt Status Register
	 * (only for debug interrupt to catch fatal errors)
	 */
	mboxEnaRegs(pdev).counter_int_status_enable =
	(COUNTER_INT_STATUS_ENABLE_BIT_SET(AR6K_TARGET_DEBUG_INTR_MASK)) >> 24;

	UNLOCK_HIF_DEV(pdev);

	/* always synchronous */
	status = hif_read_write(pdev->HIFDevice,
				INT_STATUS_ENABLE_ADDRESS,
				(char *)&mboxEnaRegs(pdev),
				sizeof(struct MBOX_IRQ_ENABLE_REGISTERS),
				HIF_WR_SYNC_BYTE_INC,
				NULL);

	if (status != QDF_STATUS_SUCCESS)
		HIF_ERROR("%s: Err updating intr reg: %d", __func__, status);
}

void hif_dev_dump_registers(struct hif_sdio_device *pdev,
			    struct MBOX_IRQ_PROC_REGISTERS *irq_proc,
			    struct MBOX_IRQ_ENABLE_REGISTERS *irq_en,
			    struct MBOX_COUNTER_REGISTERS *mbox_regs)
{
	int i = 0;

	HIF_DBG("%s: Mailbox registers:", __func__);

	if (irq_proc) {
		HIF_DBG("HostIntStatus: 0x%x ", irq_proc->host_int_status);
		HIF_DBG("CPUIntStatus: 0x%x ", irq_proc->cpu_int_status);
		HIF_DBG("ErrorIntStatus: 0x%x ", irq_proc->error_int_status);
		HIF_DBG("CounterIntStat: 0x%x ", irq_proc->counter_int_status);
		HIF_DBG("MboxFrame: 0x%x ", irq_proc->mbox_frame);
		HIF_DBG("RxLKAValid: 0x%x ", irq_proc->rx_lookahead_valid);
		HIF_DBG("RxLKA0: 0x%x", irq_proc->rx_lookahead[0]);
		HIF_DBG("RxLKA1: 0x%x ", irq_proc->rx_lookahead[1]);
		HIF_DBG("RxLKA2: 0x%x ", irq_proc->rx_lookahead[2]);
		HIF_DBG("RxLKA3: 0x%x", irq_proc->rx_lookahead[3]);

		if (pdev->MailBoxInfo.gmbox_address != 0) {
			HIF_DBG("GMBOX-HostIntStatus2:  0x%x ",
				irq_proc->host_int_status2);
			HIF_DBG("GMBOX-RX-Avail: 0x%x ",
				irq_proc->gmbox_rx_avail);
		}
	}

	if (irq_en) {
		HIF_DBG("IntStatusEnable: 0x%x\n",
			irq_en->int_status_enable);
		HIF_DBG("CounterIntStatus: 0x%x\n",
			irq_en->counter_int_status_enable);
	}

	for (i = 0; mbox_regs && i < 4; i++)
		HIF_DBG("Counter[%d]: 0x%x\n", i, mbox_regs->counter[i]);
}

/* under HL SDIO, with Interface Memory support, we have
 * the following reasons to support 2 mboxs:
 * a) we need place different buffers in different
 * mempool, for example, data using Interface Memory,
 * desc and other using DRAM, they need different SDIO
 * mbox channels.
 * b) currently, tx mempool in LL case is separated from
 * main mempool, the structure (descs at the beginning
 * of every pool buffer) is different, because they only
 * need store tx desc from host. To align with LL case,
 * we also need 2 mbox support just as PCIe LL cases.
 */

/**
 * hif_dev_map_pipe_to_mail_box() - maps pipe id to mailbox.
 * @pdev: The pointer to the hif device object
 * @pipeid: pipe index
 *
 * Return: mailbox index
 */
static uint8_t hif_dev_map_pipe_to_mail_box(struct hif_sdio_device *pdev,
					    uint8_t pipeid)
{
	if (2 == pipeid || 3 == pipeid)
		return 1;
	else if (0 == pipeid || 1 == pipeid)
		return 0;

	HIF_ERROR("%s: pipeid=%d invalid", __func__, pipeid);

	qdf_assert(0);

	return INVALID_MAILBOX_NUMBER;
}

/**
 * hif_dev_map_mail_box_to_pipe() - map sdio mailbox to htc pipe.
 * @pdev: The pointer to the hif device object
 * @mboxIndex: mailbox index
 * @upload: boolean to decide mailbox index
 *
 * Return: Invalid pipe index
 */
static uint8_t hif_dev_map_mail_box_to_pipe(struct hif_sdio_device *pdev,
					    uint8_t mbox_index, bool upload)
{
	if (mbox_index == 0)
		return upload ? 1 : 0;
	else if (mbox_index == 1)
		return upload ? 3 : 2;

	HIF_ERROR("%s: mbox_index=%d, upload=%d invalid",
		  __func__, mbox_index, upload);

	qdf_assert(0);

	return INVALID_MAILBOX_NUMBER; /* invalid pipe id */
}

/**
 * hif_get_send_addr() - Get the transfer pipe address
 * @pdev: The pointer to the hif device object
 * @pipe: The pipe identifier
 *
 * Return 0 for success and non-zero for failure to map
 */
int hif_get_send_address(struct hif_sdio_device *pdev,
			 uint8_t pipe, uint32_t *addr)
{
	uint8_t mbox_index = INVALID_MAILBOX_NUMBER;

	if (!addr)
		return -EINVAL;

	mbox_index = hif_dev_map_pipe_to_mail_box(pdev, pipe);

	if (mbox_index == INVALID_MAILBOX_NUMBER)
		return -EINVAL;

	*addr = pdev->MailBoxInfo.mbox_prop[mbox_index].extended_address;

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
	struct hif_device_mbox_info mboxinfo;
	uint32_t taddr = *addr, mboxlen = 0;

	hif_configure_device(NULL, pdev, HIF_DEVICE_GET_FIFO_ADDR,
			     &mboxinfo, sizeof(mboxinfo));

	if (taddr >= 0x800 && taddr < 0xC00) {
		/* Host control register and CIS Window */
		mboxlen = 0;
	} else if (taddr == mboxinfo.mbox_addresses[0] ||
		   taddr == mboxinfo.mbox_addresses[1] ||
		   taddr == mboxinfo.mbox_addresses[2] ||
		   taddr == mboxinfo.mbox_addresses[3]) {
		mboxlen = HIF_MBOX_WIDTH;
	} else if (taddr == mboxinfo.mbox_prop[0].extended_address) {
		mboxlen = mboxinfo.mbox_prop[0].extended_size;
	} else if (taddr == mboxinfo.mbox_prop[1].extended_address) {
		mboxlen = mboxinfo.mbox_prop[1].extended_size;
	} else {
		HIF_ERROR("%s: Invalid write addr: 0x%08x\n", __func__, taddr);
		return;
	}

	if (mboxlen != 0) {
		if (*length > mboxlen) {
			HIF_ERROR("%s: Error (%u > %u)",
				  __func__, *length, mboxlen);
			return;
		}

		taddr = taddr + (mboxlen - *length);
		taddr = taddr + ((req & HIF_DUMMY_SPACE_MASK) >> 16);
		*addr = taddr;
	}
}

/**
 * hif_dev_recv_packet() - Receieve HTC packet/packet information from device
 * @pdev : HIF device object
 * @packet : The HTC packet pointer
 * @recv_length : The length of information to be received
 * @mbox_index : The mailbox that contains this information
 *
 * Return 0 for success and non zero of error
 */
static QDF_STATUS hif_dev_recv_packet(struct hif_sdio_device *pdev,
				      HTC_PACKET *packet,
				      uint32_t recv_length,
				      uint32_t mbox_index)
{
	QDF_STATUS status;
	uint32_t padded_length;
	bool sync = (packet->Completion) ? false : true;
	uint32_t req = sync ? HIF_RD_SYNC_BLOCK_FIX : HIF_RD_ASYNC_BLOCK_FIX;

	/* adjust the length to be a multiple of block size if appropriate */
	padded_length = DEV_CALC_RECV_PADDED_LEN(pdev, recv_length);

	if (padded_length > packet->BufferLength) {
		HIF_ERROR("%s: No space for padlen:%d recvlen:%d bufferlen:%d",
			  __func__, padded_length,
			  recv_length, packet->BufferLength);
		if (packet->Completion) {
			COMPLETE_HTC_PACKET(packet, QDF_STATUS_E_INVAL);
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_INVAL;
	}

	/* mailbox index is saved in Endpoint member */
	HIF_INFO("%s : hdr:0x%x, len:%d, padded length: %d Mbox:0x%x",
		 __func__, packet->PktInfo.AsRx.ExpectedHdr, recv_length,
		 padded_length, mbox_index);

	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.mbox_addresses[mbox_index],
				packet->pBuffer,
				padded_length,
				req, sync ? NULL : packet);

	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_E_PENDING)
		HIF_ERROR("%s : Failed %d", __func__, status);

	if (sync) {
		packet->Status = status;
		if (status == QDF_STATUS_SUCCESS) {
			HTC_FRAME_HDR *hdr = (HTC_FRAME_HDR *) packet->pBuffer;

			HIF_INFO("%s: EP:%d,Len:%d,Flag:%d,CB:0x%02X,0x%02X\n",
				 __func__,
				 hdr->EndpointID, hdr->PayloadLen,
				 hdr->Flags, hdr->ControlBytes0,
				 hdr->ControlBytes1);
		}
	}

	return status;
}

static QDF_STATUS hif_dev_issue_recv_packet_bundle
(
	struct hif_sdio_device *pdev,
	HTC_PACKET_QUEUE *recv_pkt_queue,
	HTC_PACKET_QUEUE *sync_completion_queue,
	uint8_t mail_box_index,
	int *num_packets_fetched,
	bool partial_bundle
)
{
	uint32_t padded_length;
	int i, total_length = 0;
	HTC_TARGET *target = NULL;
	int bundleSpaceRemaining = 0;
	unsigned char *bundle_buffer = NULL;
	HTC_PACKET *packet, *packet_rx_bundle;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	target = (HTC_TARGET *)pdev->pTarget;

	if ((HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue) -
	     HTC_MAX_MSG_PER_BUNDLE_RX) > 0) {
		partial_bundle = true;
		HIF_WARN("%s, partial bundle detected num: %d, %d\n",
			 __func__,
			 HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue),
			 HTC_MAX_MSG_PER_BUNDLE_RX);
	}

	bundleSpaceRemaining =
		HTC_MAX_MSG_PER_BUNDLE_RX * target->TargetCreditSize;
	packet_rx_bundle = allocate_htc_bundle_packet(target);
	if (!packet_rx_bundle) {
		HIF_ERROR("%s: packet_rx_bundle is NULL\n", __func__);
		qdf_sleep(NBUF_ALLOC_FAIL_WAIT_TIME);  /* 100 msec sleep */
		return QDF_STATUS_E_NOMEM;
	}
	bundle_buffer = packet_rx_bundle->pBuffer;

	for (i = 0;
	     !HTC_QUEUE_EMPTY(recv_pkt_queue) && i < HTC_MAX_MSG_PER_BUNDLE_RX;
	     i++) {
		packet = htc_packet_dequeue(recv_pkt_queue);
		A_ASSERT(packet);
		if (!packet)
			break;
		padded_length =
			DEV_CALC_RECV_PADDED_LEN(pdev, packet->ActualLength);
		if (packet->PktInfo.AsRx.HTCRxFlags &
				HTC_RX_PKT_LAST_BUNDLED_PKT_HAS_ADDTIONAL_BLOCK)
			padded_length += HIF_BLOCK_SIZE;
		if ((bundleSpaceRemaining - padded_length) < 0) {
			/* exceeds what we can transfer, put the packet back */
			HTC_PACKET_ENQUEUE_TO_HEAD(recv_pkt_queue, packet);
			break;
		}
		bundleSpaceRemaining -= padded_length;

		if (partial_bundle ||
		    HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue) > 0) {
			packet->PktInfo.AsRx.HTCRxFlags |=
				HTC_RX_PKT_IGNORE_LOOKAHEAD;
		}
		packet->PktInfo.AsRx.HTCRxFlags |= HTC_RX_PKT_PART_OF_BUNDLE;

		if (sync_completion_queue)
			HTC_PACKET_ENQUEUE(sync_completion_queue, packet);

		total_length += padded_length;
	}
#if DEBUG_BUNDLE
	qdf_print("Recv bundle count %d, length %d.",
		  sync_completion_queue ?
		  HTC_PACKET_QUEUE_DEPTH(sync_completion_queue) : 0,
		  total_length);
#endif

	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.
				mbox_addresses[(int)mail_box_index],
				bundle_buffer, total_length,
				HIF_RD_SYNC_BLOCK_FIX, NULL);

	if (status != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s, hif_send Failed status:%d\n",
			  __func__, status);
	} else {
		unsigned char *buffer = bundle_buffer;
		*num_packets_fetched = i;
		if (sync_completion_queue) {
			HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(
				sync_completion_queue, packet) {
				padded_length =
				DEV_CALC_RECV_PADDED_LEN(pdev,
							 packet->ActualLength);
				if (packet->PktInfo.AsRx.HTCRxFlags &
				HTC_RX_PKT_LAST_BUNDLED_PKT_HAS_ADDTIONAL_BLOCK)
					padded_length +=
						HIF_BLOCK_SIZE;
				A_MEMCPY(packet->pBuffer,
					 buffer, padded_length);
				buffer += padded_length;
			} HTC_PACKET_QUEUE_ITERATE_END;
		}
	}
	/* free bundle space under Sync mode */
	free_htc_bundle_packet(target, packet_rx_bundle);
	return status;
}

#define ISSUE_BUNDLE hif_dev_issue_recv_packet_bundle
static
QDF_STATUS hif_dev_recv_message_pending_handler(struct hif_sdio_device *pdev,
						uint8_t mail_box_index,
						uint32_t msg_look_aheads[],
						int num_look_aheads,
						bool *async_proc,
						int *num_pkts_fetched)
{
	int pkts_fetched;
	HTC_PACKET *pkt;
	HTC_ENDPOINT_ID id;
	bool partial_bundle;
	int total_fetched = 0;
	bool asyncProc = false;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t look_aheads[HTC_MAX_MSG_PER_BUNDLE_RX];
	HTC_PACKET_QUEUE recv_q, sync_comp_q;
	QDF_STATUS (*rxCompletion)(void *, qdf_nbuf_t,	uint8_t);

	HIF_INFO("%s: NumLookAheads: %d\n", __func__, num_look_aheads);

	if (num_pkts_fetched)
		*num_pkts_fetched = 0;

	if (IS_DEV_IRQ_PROCESSING_ASYNC_ALLOWED(pdev)) {
		/* We use async mode to get the packets if the
		 * device layer supports it. The device layer
		 * interfaces with HIF in which HIF may have
		 * restrictions on how interrupts are processed
		 */
		asyncProc = true;
	}

	if (async_proc) {
		/* indicate to caller how we decided to process this */
		*async_proc = asyncProc;
	}

	if (num_look_aheads > HTC_MAX_MSG_PER_BUNDLE_RX) {
		A_ASSERT(false);
		return QDF_STATUS_E_PROTO;
	}

	A_MEMCPY(look_aheads, msg_look_aheads,
		 (sizeof(uint32_t)) * num_look_aheads);
	while (true) {
		/* reset packets queues */
		INIT_HTC_PACKET_QUEUE(&recv_q);
		INIT_HTC_PACKET_QUEUE(&sync_comp_q);
		if (num_look_aheads > HTC_MAX_MSG_PER_BUNDLE_RX) {
			status = QDF_STATUS_E_PROTO;
			A_ASSERT(false);
			break;
		}

		/* first lookahead sets the expected endpoint IDs for
		 * all packets in a bundle
		 */
		id = ((HTC_FRAME_HDR *)&look_aheads[0])->EndpointID;

		if (id >= ENDPOINT_MAX) {
			HIF_ERROR("%s: Invalid Endpoint in lookahead: %d\n",
				  __func__, id);
			status = QDF_STATUS_E_PROTO;
			break;
		}
		/* try to allocate as many HTC RX packets indicated
		 * by the lookaheads these packets are stored
		 * in the recvPkt queue
		 */
		status = hif_dev_alloc_and_prepare_rx_packets(pdev,
							      look_aheads,
							      num_look_aheads,
							      &recv_q);
		if (QDF_IS_STATUS_ERROR(status))
			break;
		total_fetched += HTC_PACKET_QUEUE_DEPTH(&recv_q);

		/* we've got packet buffers for all we can currently fetch,
		 * this count is not valid anymore
		 */
		num_look_aheads = 0;
		partial_bundle = false;

		/* now go fetch the list of HTC packets */
		while (!HTC_QUEUE_EMPTY(&recv_q)) {
			pkts_fetched = 0;
			if ((HTC_PACKET_QUEUE_DEPTH(&recv_q) > 1)) {
				/* there are enough packets to attempt a bundle
				 * transfer and recv bundling is allowed
				 */
				status = ISSUE_BUNDLE(pdev,
						      &recv_q,
						      asyncProc ? NULL :
						      &sync_comp_q,
						      mail_box_index,
						      &pkts_fetched,
						      partial_bundle);
				if (QDF_IS_STATUS_ERROR(status)) {
					hif_dev_free_recv_pkt_queue(
							&recv_q);
					break;
				}

				if (HTC_PACKET_QUEUE_DEPTH(&recv_q) !=
					0) {
					/* we couldn't fetch all packets at one,
					 * time this creates a broken
					 * bundle
					 */
					partial_bundle = true;
				}
			}

			/* see if the previous operation fetched any
			 * packets using bundling
			 */
			if (pkts_fetched == 0) {
				/* dequeue one packet */
				pkt = htc_packet_dequeue(&recv_q);
				A_ASSERT(pkt);
				if (!pkt)
					break;

				pkt->Completion = NULL;

				if (HTC_PACKET_QUEUE_DEPTH(&recv_q) >
				    0) {
					/* lookaheads in all packets except the
					 * last one in must be ignored
					 */
					pkt->PktInfo.AsRx.HTCRxFlags |=
						HTC_RX_PKT_IGNORE_LOOKAHEAD;
				}

				/* go fetch the packet */
				status =
				hif_dev_recv_packet(pdev, pkt,
						    pkt->ActualLength,
						    mail_box_index);
				while (QDF_IS_STATUS_ERROR(status) &&
				       !HTC_QUEUE_EMPTY(&recv_q)) {
					qdf_nbuf_t nbuf;

					pkt = htc_packet_dequeue(&recv_q);
					if (!pkt)
						break;
					nbuf = pkt->pNetBufContext;
					if (nbuf)
						qdf_nbuf_free(nbuf);
				}

				if (QDF_IS_STATUS_ERROR(status))
					break;
				/* sent synchronously, queue this packet for
				 * synchronous completion
				 */
				HTC_PACKET_ENQUEUE(&sync_comp_q, pkt);
			}
		}

		/* synchronous handling */
		if (pdev->DSRCanYield) {
			/* for the SYNC case, increment count that tracks
			 * when the DSR should yield
			 */
			pdev->CurrentDSRRecvCount++;
		}

		/* in the sync case, all packet buffers are now filled,
		 * we can process each packet, check lookahead , then repeat
		 */
		rxCompletion = pdev->hif_callbacks.rxCompletionHandler;

		/* unload sync completion queue */
		while (!HTC_QUEUE_EMPTY(&sync_comp_q)) {
			uint8_t pipeid;
			qdf_nbuf_t netbuf;

			pkt = htc_packet_dequeue(&sync_comp_q);
			A_ASSERT(pkt);
			if (!pkt)
				break;

			num_look_aheads = 0;
			status = hif_dev_process_recv_header(pdev, pkt,
							     look_aheads,
							     &num_look_aheads);
			if (QDF_IS_STATUS_ERROR(status)) {
				HTC_PACKET_ENQUEUE_TO_HEAD(&sync_comp_q, pkt);
				break;
			}

			netbuf = (qdf_nbuf_t)pkt->pNetBufContext;
			/* set data length */
			qdf_nbuf_put_tail(netbuf, pkt->ActualLength);

			if (rxCompletion) {
				pipeid =
				hif_dev_map_mail_box_to_pipe(pdev,
							     mail_box_index,
							     true);
				rxCompletion(pdev->hif_callbacks.Context,
					     netbuf, pipeid);
			}
		}

		if (QDF_IS_STATUS_ERROR(status)) {
			if (!HTC_QUEUE_EMPTY(&sync_comp_q))
				hif_dev_free_recv_pkt_queue(
						&sync_comp_q);
			break;
		}

		if (num_look_aheads == 0) {
			/* no more look aheads */
			break;
		}
		/* check whether other OS contexts have queued any WMI
		 * command/data for WLAN. This check is needed only if WLAN
		 * Tx and Rx happens in same thread context
		 */
		/* A_CHECK_DRV_TX(); */
	}
	if (num_pkts_fetched)
		*num_pkts_fetched = total_fetched;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("-HTCRecvMessagePendingHandler\n"));
	return status;
}

/**
 * hif_dev_service_cpu_interrupt() - service fatal interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_cpu_interrupt(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;
	uint8_t reg_buffer[4];
	uint8_t cpu_int_status;

	cpu_int_status = mboxProcRegs(pdev).cpu_int_status &
			 mboxEnaRegs(pdev).cpu_int_status_enable;

	HIF_ERROR("%s: 0x%x", __func__, (uint32_t)cpu_int_status);

	/* Clear the interrupt */
	mboxProcRegs(pdev).cpu_int_status &= ~cpu_int_status;

	/*set up the register transfer buffer to hit the register
	 * 4 times , this is done to make the access 4-byte aligned
	 * to mitigate issues with host bus interconnects that
	 * restrict bus transfer lengths to be a multiple of 4-bytes
	 * set W1C value to clear the interrupt, this hits the register
	 * first
	 */
	reg_buffer[0] = cpu_int_status;
	/* the remaining 4 values are set to zero which have no-effect  */
	reg_buffer[1] = 0;
	reg_buffer[2] = 0;
	reg_buffer[3] = 0;

	status = hif_read_write(pdev->HIFDevice,
				CPU_INT_STATUS_ADDRESS,
				reg_buffer, 4, HIF_WR_SYNC_BYTE_FIX, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);

	/* The Interrupt sent to the Host is generated via bit0
	 * of CPU INT register
	 */
	if (cpu_int_status & 0x1) {
		if (pdev->hif_callbacks.fwEventHandler)
			/* It calls into HTC which propagates this
			 * to ol_target_failure()
			 */
			pdev->hif_callbacks.fwEventHandler(
				pdev->hif_callbacks.Context,
				QDF_STATUS_E_FAILURE);
	} else {
		HIF_ERROR("%s: Unrecognized CPU event", __func__);
	}

	return status;
}

/**
 * hif_dev_service_error_interrupt() - service error interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_error_interrupt(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;
	uint8_t reg_buffer[4];
	uint8_t error_int_status = 0;

	error_int_status = mboxProcRegs(pdev).error_int_status & 0x0F;
	HIF_ERROR("%s: 0x%x", __func__, error_int_status);

	if (ERROR_INT_STATUS_WAKEUP_GET(error_int_status))
		HIF_ERROR("%s: Error : Wakeup", __func__);

	if (ERROR_INT_STATUS_RX_UNDERFLOW_GET(error_int_status))
		HIF_ERROR("%s: Error : Rx Underflow", __func__);

	if (ERROR_INT_STATUS_TX_OVERFLOW_GET(error_int_status))
		HIF_ERROR("%s: Error : Tx Overflow", __func__);

	/* Clear the interrupt */
	mboxProcRegs(pdev).error_int_status &= ~error_int_status;

	/* set up the register transfer buffer to hit the register
	 * 4 times , this is done to make the access 4-byte
	 * aligned to mitigate issues with host bus interconnects that
	 * restrict bus transfer lengths to be a multiple of 4-bytes
	 */

	/* set W1C value to clear the interrupt */
	reg_buffer[0] = error_int_status;
	/* the remaining 4 values are set to zero which have no-effect  */
	reg_buffer[1] = 0;
	reg_buffer[2] = 0;
	reg_buffer[3] = 0;

	status = hif_read_write(pdev->HIFDevice,
				ERROR_INT_STATUS_ADDRESS,
				reg_buffer, 4, HIF_WR_SYNC_BYTE_FIX, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);
	return status;
}

/**
 * hif_dev_service_debug_interrupt() - service debug interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_debug_interrupt(struct hif_sdio_device *pdev)
{
	uint32_t dummy;
	QDF_STATUS status;

	/* Send a target failure event to the application */
	HIF_ERROR("%s: Target debug interrupt", __func__);

	/* clear the interrupt , the debug error interrupt is counter 0
	 * read counter to clear interrupt
	 */
	status = hif_read_write(pdev->HIFDevice,
				COUNT_DEC_ADDRESS,
				(uint8_t *)&dummy,
				4, HIF_RD_SYNC_BYTE_INC, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);
	return status;
}

/**
 * hif_dev_service_counter_interrupt() - service counter interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static
QDF_STATUS hif_dev_service_counter_interrupt(struct hif_sdio_device *pdev)
{
	uint8_t counter_int_status;

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("Counter Interrupt\n"));

	counter_int_status = mboxProcRegs(pdev).counter_int_status &
			     mboxEnaRegs(pdev).counter_int_status_enable;

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("Valid interrupt source in COUNTER_INT_STATUS: 0x%x\n",
			 counter_int_status));

	/* Check if the debug interrupt is pending
	 * NOTE: other modules like GMBOX may use the counter interrupt
	 * for credit flow control on other counters, we only need to
	 * check for the debug assertion counter interrupt
	 */
	if (counter_int_status & AR6K_TARGET_DEBUG_INTR_MASK)
		return hif_dev_service_debug_interrupt(pdev);

	return QDF_STATUS_SUCCESS;
}

#define RX_LOOAHEAD_GET(pdev, i) \
	mboxProcRegs(pdev).rx_lookahead[MAILBOX_LOOKAHEAD_SIZE_IN_WORD * i]
/**
 * hif_dev_process_pending_irqs() - process pending interrupts
 * @pDev: hif sdio device context
 * @pDone: pending irq completion status
 * @pASyncProcessing: sync/async processing flag
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
QDF_STATUS hif_dev_process_pending_irqs(struct hif_sdio_device *pdev,
					bool *done,
					bool *async_processing)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t host_int_status = 0;
	uint32_t l_ahead[MAILBOX_USED_COUNT];
	int i;

	qdf_mem_zero(&l_ahead, sizeof(l_ahead));
	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("+ProcessPendingIRQs: (dev: 0x%lX)\n",
			 (unsigned long)pdev));

	/* NOTE: the HIF implementation guarantees that the context
	 * of this call allows us to perform SYNCHRONOUS I/O,
	 * that is we can block, sleep or call any API that
	 * can block or switch thread/task ontexts.
	 * This is a fully schedulable context.
	 */
	do {
		if (mboxEnaRegs(pdev).int_status_enable == 0) {
			/* interrupt enables have been cleared, do not try
			 * to process any pending interrupts that
			 * may result in more bus transactions.
			 * The target may be unresponsive at this point.
			 */
			break;
		}
		status = hif_read_write(pdev->HIFDevice,
					HOST_INT_STATUS_ADDRESS,
					(uint8_t *)&mboxProcRegs(pdev),
					sizeof(mboxProcRegs(pdev)),
					HIF_RD_SYNC_BYTE_INC, NULL);

		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_IRQ)) {
			hif_dev_dump_registers(pdev,
					       &mboxProcRegs(pdev),
					       &mboxEnaRegs(pdev),
					       &mboxCountRegs(pdev));
		}

		/* Update only those registers that are enabled */
		host_int_status = mboxProcRegs(pdev).host_int_status
				  & mboxEnaRegs(pdev).int_status_enable;

		/* only look at mailbox status if the HIF layer did not
		 * provide this function, on some HIF interfaces reading
		 * the RX lookahead is not valid to do
		 */
		for (i = 0; i < MAILBOX_USED_COUNT; i++) {
			l_ahead[i] = 0;
			if (host_int_status & (1 << i)) {
				/* mask out pending mailbox value, we use
				 * "lookAhead" as the real flag for
				 * mailbox processing below
				 */
				host_int_status &= ~(1 << i);
				if (mboxProcRegs(pdev).
				    rx_lookahead_valid & (1 << i)) {
					/* mailbox has a message and the
					 * look ahead is valid
					 */
					l_ahead[i] = RX_LOOAHEAD_GET(pdev, i);
				}
			}
		} /*end of for loop */
	} while (false);

	do {
		bool bLookAheadValid = false;
		/* did the interrupt status fetches succeed? */
		if (QDF_IS_STATUS_ERROR(status))
			break;

		for (i = 0; i < MAILBOX_USED_COUNT; i++) {
			if (l_ahead[i] != 0) {
				bLookAheadValid = true;
				break;
			}
		}

		if ((host_int_status == 0) && !bLookAheadValid) {
			/* nothing to process, the caller can use this
			 * to break out of a loop
			 */
			*done = true;
			break;
		}

		if (bLookAheadValid) {
			for (i = 0; i < MAILBOX_USED_COUNT; i++) {
				int fetched = 0;

				if (l_ahead[i] == 0)
					continue;
				AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
						("mbox[%d],lookahead:0x%X\n",
						i, l_ahead[i]));
				/* Mailbox Interrupt, the HTC layer may issue
				 * async requests to empty the mailbox...
				 * When emptying the recv mailbox we use the
				 * async handler from the completion routine of
				 * routine of the callers read request.
				 * This can improve performance by reducing
				 * the  context switching when we rapidly
				 * pull packets
				 */
				status = hif_dev_recv_message_pending_handler(
							pdev, i,
							&l_ahead
							[i], 1,
							async_processing,
							&fetched);
				if (QDF_IS_STATUS_ERROR(status))
					break;

				if (!fetched) {
					/* HTC could not pull any messages out
					 * due to lack of resources force DSR
					 * handle to ack the interrupt
					 */
					*async_processing = false;
					pdev->RecheckIRQStatusCnt = 0;
				}
			}
		}

		/* now handle the rest of them */
		AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
				("Valid source for OTHER interrupts: 0x%x\n",
				host_int_status));

		if (HOST_INT_STATUS_CPU_GET(host_int_status)) {
			/* CPU Interrupt */
			status = hif_dev_service_cpu_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

		if (HOST_INT_STATUS_ERROR_GET(host_int_status)) {
			/* Error Interrupt */
			status = hif_dev_service_error_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

		if (HOST_INT_STATUS_COUNTER_GET(host_int_status)) {
			/* Counter Interrupt */
			status = hif_dev_service_counter_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

	} while (false);

	/* an optimization to bypass reading the IRQ status registers
	 * unecessarily which can re-wake the target, if upper layers
	 * determine that we are in a low-throughput mode, we can
	 * rely on taking another interrupt rather than re-checking
	 * the status registers which can re-wake the target.
	 *
	 * NOTE : for host interfaces that use the special
	 * GetPendingEventsFunc, this optimization cannot be used due to
	 * possible side-effects.  For example, SPI requires the host
	 * to drain all messages from the mailbox before exiting
	 * the ISR routine.
	 */
	if (!(*async_processing) && (pdev->RecheckIRQStatusCnt == 0)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
				("Bypass IRQ Status re-check, forcing done\n"));
		*done = true;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("-ProcessPendingIRQs: (done:%d, async:%d) status=%d\n",
			 *done, *async_processing, status));

	return status;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0)) && \
		 !defined(WITH_BACKPORTS)
/**
 * hif_sdio_set_drvdata() - set wlan driver data into upper layer private
 * @func: pointer to sdio function
 * @hifdevice: pointer to hif device
 *
 * Return: non zero for success.
 */
int hif_sdio_set_drvdata(struct sdio_func *func,
			 struct hif_sdio_dev *hifdevice)
{
	return sdio_set_drvdata(func, hifdevice);
}
#else
int hif_sdio_set_drvdata(struct sdio_func *func,
			 struct hif_sdio_dev *hifdevice)
{
	sdio_set_drvdata(func, hifdevice);
	return 0;
}
#endif /* LINUX VERSION */

struct hif_sdio_dev *get_hif_device(struct sdio_func *func)
{
	qdf_assert(func);

	return (struct hif_sdio_dev *)sdio_get_drvdata(func);
}
#endif /* CONFIG_SDIO_TRANSFER_MAILBOX */

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

#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
#define ATH_MODULE_NAME hif
#include <linux/kthread.h>
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

/* by default setup a bounce buffer for the data packets,
 * if the underlying host controller driver
 * does not use DMA you may be able to skip this step
 * and save the memory allocation and transfer time
 */
#define HIF_USE_DMA_BOUNCE_BUFFER 1
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

/** hif_dev_set_mailbox_swap() - Set the mailbox swap from firmware
 * @pdev : The HIF layer object
 *
 * Return: none
 */
void hif_dev_set_mailbox_swap(struct hif_sdio_dev *pdev)
{
	struct hif_sdio_device *hif_device = hif_dev_from_hif(pdev);

	HIF_ENTER();

	hif_device->swap_mailbox = true;

	HIF_EXIT();
}

/** hif_dev_get_mailbox_swap() - Get the mailbox swap setting
 * @pdev : The HIF layer object
 *
 * Return: true or false
 */
bool hif_dev_get_mailbox_swap(struct hif_sdio_dev *pdev)
{
	struct hif_sdio_device *hif_device;

	HIF_ENTER();

	hif_device = hif_dev_from_hif(pdev);

	HIF_EXIT();

	return hif_device->swap_mailbox;
}

/**
 * hif_dev_get_fifo_address() - get the fifo addresses for dma
 * @pdev:  SDIO HIF object
 * @config: mbox address config pointer
 *
 * Return : 0 for success, non-zero for error
 */
int hif_dev_get_fifo_address(struct hif_sdio_dev *pdev,
			     void *config,
			     uint32_t config_len)
{
	uint32_t count;
	struct hif_device_mbox_info *cfg =
				(struct hif_device_mbox_info *)config;

	for (count = 0; count < 4; count++)
		cfg->mbox_addresses[count] = HIF_MBOX_START_ADDR(count);

	if (config_len >= sizeof(struct hif_device_mbox_info)) {
		set_extended_mbox_window_info((uint16_t)pdev->func->device,
					      cfg);
		return 0;
	}

	return -EINVAL;
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
		hif_err("%s: Err : Invalid service (%d)",
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
		hif_err("%s: HIF_DEVICE_GET_MBOX_ADDR failed", __func__);

	status = hif_configure_device(NULL, pdev->HIFDevice,
				      HIF_DEVICE_GET_BLOCK_SIZE,
				      blocksizes, sizeof(blocksizes));
	if (status != QDF_STATUS_SUCCESS)
		hif_err("%s: HIF_DEVICE_GET_MBOX_BLOCK_SIZE fail", __func__);

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
		hif_err("%s: Err updating intr reg: %d", __func__, status);
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
		hif_err("%s: Err updating intr reg: %d", __func__, status);
}

void hif_dev_dump_registers(struct hif_sdio_device *pdev,
			    struct MBOX_IRQ_PROC_REGISTERS *irq_proc,
			    struct MBOX_IRQ_ENABLE_REGISTERS *irq_en,
			    struct MBOX_COUNTER_REGISTERS *mbox_regs)
{
	int i = 0;

	hif_debug("%s: Mailbox registers:", __func__);

	if (irq_proc) {
		hif_debug("HostIntStatus: 0x%x ", irq_proc->host_int_status);
		hif_debug("CPUIntStatus: 0x%x ", irq_proc->cpu_int_status);
		hif_debug("ErrorIntStatus: 0x%x ", irq_proc->error_int_status);
		hif_debug("CounterIntStat: 0x%x ",
			  irq_proc->counter_int_status);
		hif_debug("MboxFrame: 0x%x ", irq_proc->mbox_frame);
		hif_debug("RxLKAValid: 0x%x ", irq_proc->rx_lookahead_valid);
		hif_debug("RxLKA0: 0x%x", irq_proc->rx_lookahead[0]);
		hif_debug("RxLKA1: 0x%x ", irq_proc->rx_lookahead[1]);
		hif_debug("RxLKA2: 0x%x ", irq_proc->rx_lookahead[2]);
		hif_debug("RxLKA3: 0x%x", irq_proc->rx_lookahead[3]);

		if (pdev->MailBoxInfo.gmbox_address != 0) {
			hif_debug("GMBOX-HostIntStatus2:  0x%x ",
				  irq_proc->host_int_status2);
			hif_debug("GMBOX-RX-Avail: 0x%x ",
				  irq_proc->gmbox_rx_avail);
		}
	}

	if (irq_en) {
		hif_debug("IntStatusEnable: 0x%x\n",
			  irq_en->int_status_enable);
		hif_debug("CounterIntStatus: 0x%x\n",
			  irq_en->counter_int_status_enable);
	}

	for (i = 0; mbox_regs && i < 4; i++)
		hif_debug("Counter[%d]: 0x%x\n", i, mbox_regs->counter[i]);
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

	hif_err("%s: pipeid=%d invalid", __func__, pipeid);

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

	hif_err("%s: mbox_index=%d, upload=%d invalid",
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
			 uint8_t pipe, unsigned long *addr)
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
		hif_err("%s: Invalid write addr: 0x%08x\n", __func__, taddr);
		return;
	}

	if (mboxlen != 0) {
		if (*length > mboxlen) {
			hif_err("%s: Error (%u > %u)",
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
		hif_err("%s: No space for padlen:%d recvlen:%d bufferlen:%d",
			__func__, padded_length,
			recv_length, packet->BufferLength);
		if (packet->Completion) {
			COMPLETE_HTC_PACKET(packet, QDF_STATUS_E_INVAL);
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_INVAL;
	}

	/* mailbox index is saved in Endpoint member */
	hif_debug("%s : hdr:0x%x, len:%d, padded length: %d Mbox:0x%x",
		  __func__, packet->PktInfo.AsRx.ExpectedHdr, recv_length,
		  padded_length, mbox_index);

	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.mbox_addresses[mbox_index],
				packet->pBuffer,
				padded_length,
				req, sync ? NULL : packet);

	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_E_PENDING)
		hif_err("%s : Failed %d", __func__, status);

	if (sync) {
		packet->Status = status;
		if (status == QDF_STATUS_SUCCESS) {
			HTC_FRAME_HDR *hdr = (HTC_FRAME_HDR *) packet->pBuffer;

			hif_debug("%s:EP:%d,Len:%d,Flg:%d,CB:0x%02X,0x%02X\n",
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
		hif_warn("%s, partial bundle detected num: %d, %d\n",
			 __func__,
			 HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue),
			 HTC_MAX_MSG_PER_BUNDLE_RX);
	}

	bundleSpaceRemaining =
		HTC_MAX_MSG_PER_BUNDLE_RX * target->TargetCreditSize;
	packet_rx_bundle = allocate_htc_bundle_packet(target);
	if (!packet_rx_bundle) {
		hif_err("%s: packet_rx_bundle is NULL\n", __func__);
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
		hif_err("%s, hif_send Failed status:%d\n",
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

	hif_debug("%s: NumLookAheads: %d\n", __func__, num_look_aheads);

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
			hif_err("%s: Invalid Endpoint in lookahead: %d\n",
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

	hif_err("%s: 0x%x", __func__, (uint32_t)cpu_int_status);

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
		hif_err("%s: Unrecognized CPU event", __func__);
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
	hif_err("%s: 0x%x", __func__, error_int_status);

	if (ERROR_INT_STATUS_WAKEUP_GET(error_int_status))
		hif_err("%s: Error : Wakeup", __func__);

	if (ERROR_INT_STATUS_RX_UNDERFLOW_GET(error_int_status))
		hif_err("%s: Error : Rx Underflow", __func__);

	if (ERROR_INT_STATUS_TX_OVERFLOW_GET(error_int_status))
		hif_err("%s: Error : Tx Overflow", __func__);

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
	hif_err("%s: Target debug interrupt", __func__);

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

#define DEV_CHECK_RECV_YIELD(pdev) \
	((pdev)->CurrentDSRRecvCount >= \
	 (pdev)->HifIRQYieldParams.recv_packet_yield_count)
/**
 * hif_dev_dsr_handler() - Synchronous interrupt handler
 *
 * @context: hif send context
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_dsr_handler(void *context)
{
	struct hif_sdio_device *pdev = (struct hif_sdio_device *)context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool done = false;
	bool async_proc = false;

	/* reset the recv counter that tracks when we need
	 * to yield from the DSR
	 */
	pdev->CurrentDSRRecvCount = 0;
	/* reset counter used to flag a re-scan of IRQ
	 * status registers on the target
	 */
	pdev->RecheckIRQStatusCnt = 0;

	while (!done) {
		status = hif_dev_process_pending_irqs(pdev, &done, &async_proc);
		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (pdev->HifIRQProcessingMode == HIF_DEVICE_IRQ_SYNC_ONLY) {
			/* the HIF layer does not allow async IRQ processing,
			 * override the asyncProc flag
			 */
			async_proc = false;
			/* this will cause us to re-enter ProcessPendingIRQ()
			 * and re-read interrupt status registers.
			 * This has a nice side effect of blocking us until all
			 * async read requests are completed. This behavior is
			 * required as we  do not allow ASYNC processing
			 * in interrupt handlers (like Windows CE)
			 */

			if (pdev->DSRCanYield && DEV_CHECK_RECV_YIELD(pdev))
				/* ProcessPendingIRQs() pulled enough recv
				 * messages to satisfy the yield count, stop
				 * checking for more messages and return
				 */
				break;
		}

		if (async_proc) {
			/* the function does some async I/O for performance,
			 * we need to exit the ISR immediately, the check below
			 * will prevent the interrupt from being
			 * Ack'd while we handle it asynchronously
			 */
			break;
		}
	}

	if (QDF_IS_STATUS_SUCCESS(status) && !async_proc) {
		/* Ack the interrupt only if :
		 *  1. we did not get any errors in processing interrupts
		 *  2. there are no outstanding async processing requests
		 */
		if (pdev->DSRCanYield) {
			/* if the DSR can yield do not ACK the interrupt, there
			 * could be more pending messages. The HIF layer
			 * must ACK the interrupt on behalf of HTC
			 */
			hif_info("%s:  Yield (RX count: %d)",
				 __func__, pdev->CurrentDSRRecvCount);
		} else {
			hif_ack_interrupt(pdev->HIFDevice);
		}
	}

	return status;
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
	       unsigned long address,
	       char *buffer, uint32_t length,
	       uint32_t request, void *context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct bus_request *busrequest;

	AR_DEBUG_ASSERT(device);
	AR_DEBUG_ASSERT(device->func);
	hif_debug("%s: device 0x%pK addr 0x%lX buffer 0x%pK",
		  __func__, device, address, buffer);
	hif_debug("%s: len %d req 0x%X context 0x%pK",
		  __func__, length, request, context);

	/*sdio r/w action is not needed when suspend, so just return */
	if ((device->is_suspend) &&
	    (device->power_config == HIF_DEVICE_POWER_CUT)) {
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
			if (!busrequest) {
				hif_err("%s:bus requests unavail", __func__);
				hif_err("%s, addr:0x%lX, len:%d",
					request & HIF_SDIO_READ ? "READ" :
					"WRITE", address, length);
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
						 __func__,
						 (unsigned long)busrequest));

				/* wait for completion */
				up(&device->sem_async);
				if (down_interruptible(&busrequest->sem_req) ==
				    0) {
					QDF_STATUS status = busrequest->status;

					hif_debug("%s: sync freeing 0x%lX:0x%X",
						  __func__,
						  (unsigned long)busrequest,
						  busrequest->status);
					hif_debug("%s: freeing req: 0x%X",
						  __func__,
						  (unsigned int)request);
					hif_free_bus_request(device,
							     busrequest);
					return status;
				} else {
					/* interrupted, exit */
					return QDF_STATUS_E_FAILURE;
				}
			} else {
				hif_debug("%s: queued async req: 0x%lX",
					  __func__, (unsigned long)busrequest);
				up(&device->sem_async);
				return QDF_STATUS_E_PENDING;
			}
		} else {
			hif_err("%s: Invalid execution mode: 0x%08x",
				__func__, (unsigned int)request);
			status = QDF_STATUS_E_INVAL;
			break;
		}
	} while (0);

	return status;
}

/**
 * hif_sdio_func_enable() - Handle device enabling as per device
 * @device: HIF device object
 * @func: function pointer
 *
 * Return QDF_STATUS
 */
static QDF_STATUS hif_sdio_func_enable(struct hif_softc *ol_sc,
				       struct sdio_func *func)
{
	struct hif_sdio_dev *device = get_hif_device(ol_sc, func);

	if (device->is_disabled) {
		int ret = 0;

		sdio_claim_host(func);

		ret = hif_sdio_quirk_async_intr(ol_sc, func);
		if (ret) {
			hif_err("%s: Error setting async intr:%d",
				__func__, ret);
			sdio_release_host(func);
			return QDF_STATUS_E_FAILURE;
		}

		func->enable_timeout = 100;
		ret = sdio_enable_func(func);
		if (ret) {
			hif_err("%s: Unable to enable function: %d",
				__func__, ret);
			sdio_release_host(func);
			return QDF_STATUS_E_FAILURE;
		}

		ret = sdio_set_block_size(func, HIF_BLOCK_SIZE);
		if (ret) {
			hif_err("%s: Unable to set block size 0x%X : %d\n",
				__func__, HIF_BLOCK_SIZE, ret);
			sdio_release_host(func);
			return QDF_STATUS_E_FAILURE;
		}

		ret = hif_sdio_quirk_mod_strength(ol_sc, func);
		if (ret) {
			hif_err("%s: Error setting mod strength : %d\n",
				__func__, ret);
			sdio_release_host(func);
			return QDF_STATUS_E_FAILURE;
		}

		sdio_release_host(func);
	}

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
		 uint32_t address, char *buffer,
		 uint32_t length, uint32_t request, void *context)
{
	uint8_t opcode;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int ret = A_OK;
	uint8_t *tbuffer;
	bool bounced = false;

	if (!device) {
		hif_err("%s: device null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!device->func) {
		hif_err("%s: func null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	hif_debug("%s: addr:0X%06X, len:%08d, %s, %s", __func__,
		  address, length,
		  request & HIF_SDIO_READ ? "Read " : "Write",
		  request & HIF_ASYNCHRONOUS ? "Async" : "Sync ");

	do {
		if (request & HIF_EXTENDED_IO) {
			//HIF_INFO_HI("%s: Command type: CMD53\n", __func__);
		} else {
			hif_err("%s: Invalid command type: 0x%08x\n",
				__func__, request);
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if (request & HIF_BLOCK_BASIS) {
			/* round to whole block length size */
			length =
				(length / HIF_BLOCK_SIZE) *
				HIF_BLOCK_SIZE;
			hif_debug("%s: Block mode (BlockLen: %d)\n",
				  __func__, length);
		} else if (request & HIF_BYTE_BASIS) {
			hif_debug("%s: Byte mode (BlockLen: %d)\n",
				  __func__, length);
		} else {
			hif_err("%s: Invalid data mode: 0x%08x\n",
				__func__, request);
			status = QDF_STATUS_E_INVAL;
			break;
		}
		if (request & HIF_SDIO_WRITE) {
			hif_fixup_write_param(device, request,
					      &length, &address);

			hif_debug("addr:%08X, len:0x%08X, dummy:0x%04X\n",
				  address, length,
				  (request & HIF_DUMMY_SPACE_MASK) >> 16);
		}

		if (request & HIF_FIXED_ADDRESS) {
			opcode = CMD53_FIXED_ADDRESS;
			hif_debug("%s: Addr mode: fixed 0x%X\n",
				  __func__, address);
		} else if (request & HIF_INCREMENTAL_ADDRESS) {
			opcode = CMD53_INCR_ADDRESS;
			hif_debug("%s: Address mode: Incremental 0x%X\n",
				  __func__, address);
		} else {
			hif_err("%s: Invalid address mode: 0x%08x\n",
				__func__, request);
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if (request & HIF_SDIO_WRITE) {
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (BUFFER_NEEDS_BOUNCE(buffer)) {
				AR_DEBUG_ASSERT(device->dma_buffer);
				tbuffer = device->dma_buffer;
				/* copy the write data to the dma buffer */
				AR_DEBUG_ASSERT(length <= HIF_DMA_BUFFER_SIZE);
				if (length > HIF_DMA_BUFFER_SIZE) {
					hif_err("%s: Invalid write len: %d\n",
						__func__, length);
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
			if (opcode == CMD53_FIXED_ADDRESS  && tbuffer) {
				ret = sdio_writesb(device->func, address,
						   tbuffer, length);
				hif_debug("%s:r=%d addr:0x%X, len:%d, 0x%X\n",
					  __func__, ret, address, length,
					  *(int *)tbuffer);
			} else if (tbuffer) {
				ret = sdio_memcpy_toio(device->func, address,
						       tbuffer, length);
				hif_debug("%s:r=%d addr:0x%X, len:%d, 0x%X\n",
					  __func__, ret, address, length,
					  *(int *)tbuffer);
			}
		} else if (request & HIF_SDIO_READ) {
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (BUFFER_NEEDS_BOUNCE(buffer)) {
				AR_DEBUG_ASSERT(device->dma_buffer);
				AR_DEBUG_ASSERT(length <= HIF_DMA_BUFFER_SIZE);
				if (length > HIF_DMA_BUFFER_SIZE) {
					hif_err("%s: Invalid read len: %d\n",
						__func__, length);
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
			if (opcode == CMD53_FIXED_ADDRESS && tbuffer) {
				ret = sdio_readsb(device->func, tbuffer,
						  address, length);
				hif_debug("%s:r=%d addr:0x%X, len:%d, 0x%X\n",
					  __func__, ret, address, length,
					  *(int *)tbuffer);
			} else if (tbuffer) {
				ret = sdio_memcpy_fromio(device->func,
							 tbuffer, address,
							 length);
				hif_debug("%s:r=%d addr:0x%X, len:%d, 0x%X\n",
					  __func__, ret, address, length,
					  *(int *)tbuffer);
			}
#if HIF_USE_DMA_BOUNCE_BUFFER
			if (bounced && tbuffer)
				memcpy(buffer, tbuffer, length);
#endif
		} else {
			hif_err("%s: Invalid dir: 0x%08x", __func__, request);
			status = QDF_STATUS_E_INVAL;
			return status;
		}

		if (ret) {
			hif_err("%s: SDIO bus operation failed!", __func__);
			hif_err("%s: MMC stack returned : %d", __func__, ret);
			hif_err("%s: addr:0X%06X, len:%08d, %s, %s",
				__func__, address, length,
				request & HIF_SDIO_READ ? "Read " : "Write",
				request & HIF_ASYNCHRONOUS ?
				"Async" : "Sync");
			status = QDF_STATUS_E_FAILURE;
		}
	} while (false);

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
	bool claimed = false;

	device = (struct hif_sdio_dev *)param;
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
		qdf_spin_lock_irqsave(&device->asynclock);
		/* pull the request to work on */
		while (device->asyncreq) {
			request = device->asyncreq;
			if (request->inusenext)
				device->asyncreq = request->inusenext;
			else
				device->asyncreq = NULL;
			qdf_spin_unlock_irqrestore(&device->asynclock);
			hif_debug("%s: processing req: 0x%lX",
				  __func__, (unsigned long)request);

			if (!claimed) {
				sdio_claim_host(device->func);
				claimed = true;
			}
			if (request->scatter_req) {
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

					hif_free_bus_request(device, request);
					device->htc_callbacks.
					rw_compl_handler(context, status);
				} else {
					hif_debug("%s: upping req: 0x%lX",
						  __func__,
						  (unsigned long)request);
					request->status = status;
					up(&request->sem_req);
				}
			}
			qdf_spin_lock_irqsave(&device->asynclock);
		}
		qdf_spin_unlock_irqrestore(&device->asynclock);
		if (claimed) {
			sdio_release_host(device->func);
			claimed = false;
		}
	}

	complete_and_exit(&device->async_completion, 0);

	return 0;
}

/**
 * hif_disable_func() - Disable SDIO function
 *
 * @device: HIF device pointer
 * @func: SDIO function pointer
 * @reset: If this is called from resume or probe
 *
 * Return: 0 in case of success, else error value
 */
QDF_STATUS hif_disable_func(struct hif_sdio_dev *device,
			    struct sdio_func *func,
			    bool reset)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	HIF_ENTER();
	if (!IS_ERR(device->async_task)) {
		init_completion(&device->async_completion);
		device->async_shutdown = 1;
		up(&device->sem_async);
		wait_for_completion(&device->async_completion);
		device->async_task = NULL;
		sema_init(&device->sem_async, 0);
	}

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

	HIF_ENTER();

	if (!device) {
		hif_err("%s: HIF device is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (hif_sdio_func_enable(ol_sc, func))
		return QDF_STATUS_E_FAILURE;

	/* create async I/O thread */
	if (!device->async_task && device->is_disabled) {
		device->async_shutdown = 0;
		device->async_task = kthread_create(async_task,
						    (void *)device,
						    "AR6K Async");
		if (IS_ERR(device->async_task)) {
			hif_err("%s: Error creating async task",
				__func__);
			return QDF_STATUS_E_FAILURE;
		}
		device->is_disabled = false;
		wake_up_process(device->async_task);
	}

	if (!resume)
		ret = hif_sdio_probe(ol_sc, func, device);

	HIF_EXIT();

	return ret;
}
#endif /* CONFIG_SDIO_TRANSFER_MAILBOX */

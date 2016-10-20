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
#ifndef __ATH_USB_H__
#define __ATH_USB_H__

#include <linux/reboot.h>

/*
 * There may be some pending tx frames during platform suspend.
 * Suspend operation should be delayed until those tx frames are
 * transfered from the host to target. This macro specifies how
 * long suspend thread has to sleep before checking pending tx
 * frame count.
 */
#define OL_ATH_TX_DRAIN_WAIT_DELAY     50	/* ms */
/*
 * Wait time (in unit of OL_ATH_TX_DRAIN_WAIT_DELAY) for pending
 * tx frame completion before suspend. Refer: hif_pci_suspend()
 */
#define OL_ATH_TX_DRAIN_WAIT_CNT       10

#define CONFIG_COPY_ENGINE_SUPPORT	/* TBDXXX: here for now */
#define ATH_DBG_DEFAULT   0
#include <osdep.h>
#include <ol_if_athvar.h>
#include <athdefs.h>
#include "osapi_linux.h"
#include "hif_main.h"
#include "hif.h"

#define FW_REG_DUMP_CNT       60

/* Magic patterns for FW to report crash information (Rome USB) */
#define FW_ASSERT_PATTERN       0x0000c600
#define FW_REG_PATTERN          0x0000d600
#define FW_REG_END_PATTERN      0x0000e600
#define FW_RAMDUMP_PATTERN      0x0000f600
#define FW_RAMDUMP_END_PATTERN  0x0000f601
#define FW_RAMDUMP_PATTERN_MASK 0xfffffff0
#define FW_RAMDUMP_DRAMSIZE     0x00098000
#define FW_RAMDUMP_IRAMSIZE     0x000C0000
#define FW_RAMDUMP_AXISIZE      0x00020000

/* FW RAM segments (Rome USB) */
enum {
	FW_RAM_SEG_DRAM,
	FW_RAM_SEG_IRAM,
	FW_RAM_SEG_AXI,
	FW_RAM_SEG_CNT
};

/* Allocate 384K memory to save each segment of ram dump */
#define FW_RAMDUMP_SEG_SIZE     393216

/* structure to save RAM dump information */
struct fw_ramdump {
	uint32_t start_addr;
	uint32_t length;
	uint8_t *mem;
};

/* USB Endpoint definition */
typedef enum {
	HIF_TX_CTRL_PIPE = 0,
	HIF_TX_DATA_LP_PIPE,
	HIF_TX_DATA_MP_PIPE,
	HIF_TX_DATA_HP_PIPE,
	HIF_RX_CTRL_PIPE,
	HIF_RX_DATA_PIPE,
	HIF_RX_DATA2_PIPE,
	HIF_RX_INT_PIPE,
	HIF_USB_PIPE_MAX
} HIF_USB_PIPE_ID;

#define HIF_USB_PIPE_INVALID HIF_USB_PIPE_MAX

typedef struct _HIF_USB_PIPE {
	DL_LIST urb_list_head;
	DL_LIST urb_pending_list;
	int32_t urb_alloc;
	int32_t urb_cnt;
	int32_t urb_cnt_thresh;
	unsigned int usb_pipe_handle;
	uint32_t flags;
	uint8_t ep_address;
	uint8_t logical_pipe_num;
	struct _HIF_DEVICE_USB *device;
	uint16_t max_packet_size;
#ifdef HIF_USB_TASKLET
	struct tasklet_struct io_complete_tasklet;
#else
	struct work_struct io_complete_work;
#endif
	struct sk_buff_head io_comp_queue;
	struct usb_endpoint_descriptor *ep_desc;
	int32_t urb_prestart_cnt;
} HIF_USB_PIPE;

typedef struct _HIF_DEVICE_USB {
	struct hif_softc ol_sc;
	qdf_spinlock_t cs_lock;
	qdf_spinlock_t tx_lock;
	qdf_spinlock_t rx_lock;
	struct hif_msg_callbacks htc_callbacks;
	struct usb_device *udev;
	struct usb_interface *interface;
	HIF_USB_PIPE pipes[HIF_USB_PIPE_MAX];
	uint8_t *diag_cmd_buffer;
	uint8_t *diag_resp_buffer;
	void *claimed_context;
	A_BOOL is_bundle_enabled;
	uint16_t rx_bundle_cnt;
	uint32_t rx_bundle_buf_len;
} HIF_DEVICE_USB;


struct hif_usb_softc {
	struct _HIF_DEVICE_USB hif_hdl;
	/* For efficiency, should be first in struct */
	struct device *dev;
	struct usb_dev *pdev;
	/*
	 * Guard changes to Target HW state and to software
	 * structures that track hardware state.
	 */
	u16 devid;
	struct usb_interface *interface;
	struct notifier_block reboot_notifier;  /* default mode before reboot */
	u8 suspend_state;
	u8 *fw_data;
	u32 fw_data_len;
	/* structure to save FW RAM dump (Rome USB) */
	struct fw_ramdump *ramdump[FW_RAM_SEG_CNT];
	uint8_t ramdump_index;
	bool fw_ram_dumping;
	/* enable FW self-recovery for Rome USB */
	bool enable_self_recovery;
};
#endif /* __ATH_USB_H__ */

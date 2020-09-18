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

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include "if_usb.h"
#include "hif_usb_internal.h"
#include "target_type.h"		/* TARGET_TYPE_ */
#include "regtable_usb.h"
#include "ol_fw.h"
#include "hif_debug.h"
#include "epping_main.h"
#include "hif_main.h"
#include "usb_api.h"
#ifdef CONFIG_PLD_USB_CNSS
#include "pld_common.h"
#endif

#define DELAY_FOR_TARGET_READY 200	/* 200ms */

/* Save memory addresses where we save FW ram dump, and then we could obtain
 * them by symbol table.
 */
uint32_t fw_stack_addr;
void *fw_ram_seg_addr[FW_RAM_SEG_CNT];



static int hif_usb_unload_dev_num = -1;
struct hif_usb_softc *g_usb_sc;

/**
 * hif_usb_diag_write_cold_reset() - reset SOC by sending a diag command
 * @scn: pointer to ol_softc structure
 *
 * Return: QDF_STATUS_SUCCESS if success else an appropriate QDF_STATUS error
 */
static inline QDF_STATUS
hif_usb_diag_write_cold_reset(struct hif_softc *scn)
{
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	struct hif_target_info *tgt_info = &scn->target_info;

	/* For Genoa, chip-reset is handled in CNSS driver */
	if (tgt_info->target_type == TARGET_TYPE_QCN7605)
		return QDF_STATUS_SUCCESS;

	HIF_DBG("%s: resetting SOC", __func__);

	return hif_diag_write_access(hif_hdl,
				(ROME_USB_SOC_RESET_CONTROL_COLD_RST_LSB |
				ROME_USB_RTC_SOC_BASE_ADDRESS),
				SOC_RESET_CONTROL_COLD_RST_SET(1));
}

/**
 * hif_usb_procfs_init() - create init procfs
 * @scn: pointer to hif_usb_softc structure
 *
 * Return: int 0 if success else an appropriate error number
 */
static int
hif_usb_procfs_init(struct hif_softc *scn)
{
	int ret = 0;

	HIF_ENTER();

	if (athdiag_procfs_init(scn) != 0) {
		hif_err("athdiag_procfs_init failed");
		ret = A_ERROR;
	}

	scn->athdiag_procfs_inited = true;

	HIF_EXIT();
	return ret;
}

/**
 * hif_nointrs(): disable IRQ
 * @scn: pointer to struct hif_softc
 *
 * This function stops interrupt(s)
 *
 * Return: none
 */
void hif_usb_nointrs(struct hif_softc *scn)
{

}

/**
 * hif_usb_reboot() - called at reboot time to reset WLAN SOC
 * @nb: pointer to notifier_block registered during register_reboot_notifier
 * @val: code indicating reboot reason
 * @v: unused pointer
 *
 * Return: int 0 if success else an appropriate error number
 */
static int hif_usb_reboot(struct notifier_block *nb, unsigned long val,
				void *v)
{
	struct hif_usb_softc *sc;

	HIF_ENTER();
	sc = container_of(nb, struct hif_usb_softc, reboot_notifier);
	/* do cold reset */
	hif_usb_diag_write_cold_reset(HIF_GET_SOFTC(sc));
	HIF_EXIT();
	return NOTIFY_DONE;
}

/**
 * hif_usb_disable_lpm() - Disable lpm feature of usb2.0
 * @udev: pointer to usb_device for which LPM is to be disabled
 *
 * LPM needs to be disabled to avoid usb2.0 probe timeout
 *
 * Return: int 0 if success else an appropriate error number
 */
static int hif_usb_disable_lpm(struct usb_device *udev)
{
	struct usb_hcd *hcd;
	int ret = -EPERM;

	HIF_ENTER();

	if (!udev || !udev->bus) {
		hif_err("Invalid input parameters");
		goto exit;
	}

	hcd = bus_to_hcd(udev->bus);
	if (udev->usb2_hw_lpm_enabled) {
		if (hcd->driver->set_usb2_hw_lpm) {
			ret = hcd->driver->set_usb2_hw_lpm(hcd, udev, false);
			if (!ret) {
				udev->usb2_hw_lpm_enabled = false;
				udev->usb2_hw_lpm_capable = false;
				hif_info("LPM is disabled");
			} else {
				hif_info("Fail to disable LPM");
			}
		} else {
			hif_info("hcd doesn't support LPM");
		}
	} else {
		hif_info("LPM isn't enabled");
	}
exit:
	HIF_EXIT();
	return ret;
}

/**
 * hif_usb_enable_bus() - enable usb bus
 * @ol_sc: hif_softc struct
 * @dev: device pointer
 * @bdev: bus dev pointer
 * @bid: bus id pointer
 * @type: enum hif_enable_type such as HIF_ENABLE_TYPE_PROBE
 *
 * Return: QDF_STATUS_SUCCESS on success and error QDF status on failure
 */
QDF_STATUS hif_usb_enable_bus(struct hif_softc *scn,
			struct device *dev, void *bdev,
			const struct hif_bus_id *bid,
			enum hif_enable_type type)

{
	struct usb_interface *interface = (struct usb_interface *)bdev;
	struct usb_device_id *id = (struct usb_device_id *)bid;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	struct hif_usb_softc *sc;
	struct usb_device *usbdev = interface_to_usbdev(interface);
	int vendor_id, product_id;
	struct hif_target_info *tgt_info;
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	u32 hif_type;
	u32 target_type;

	if (!scn) {
		hif_err("hif_ctx is NULL");
		goto err_usb;
	}

	sc = HIF_GET_USB_SOFTC(scn);

	hif_debug("hif_softc %pK usbdev %pK interface %pK",
		scn,
		usbdev,
		interface);

	vendor_id = qdf_le16_to_cpu(usbdev->descriptor.idVendor);
	product_id = qdf_le16_to_cpu(usbdev->descriptor.idProduct);

	hif_err("con_mode = 0x%x, vendor_id = 0x%x product_id = 0x%x",
		hif_get_conparam(scn), vendor_id, product_id);

	sc->pdev = (void *)usbdev;
	sc->dev = &usbdev->dev;
	sc->devid = id->idProduct;

	hif_get_device_type(product_id, 0, &hif_type, &target_type);
	tgt_info = hif_get_target_info_handle(hif_hdl);
	if (target_type == TARGET_TYPE_QCN7605)
		tgt_info->target_type = TARGET_TYPE_QCN7605;

	/*
	 * For Genoa, skip set_configuration, since it is handled
	 * by CNSS driver.
	 */
	if (target_type != TARGET_TYPE_QCN7605) {
		usb_get_dev(usbdev);
		if ((usb_control_msg(usbdev, usb_sndctrlpipe(usbdev, 0),
				     USB_REQ_SET_CONFIGURATION, 0, 1, 0,
				     NULL, 0, HZ)) < 0) {
			hif_err("usb_control_msg failed");
			goto err_usb;
		}
		usb_set_interface(usbdev, 0, 0);
		sc->reboot_notifier.notifier_call = hif_usb_reboot;
		register_reboot_notifier(&sc->reboot_notifier);
	}

	/* disable lpm to avoid usb2.0 probe timeout */
	hif_usb_disable_lpm(usbdev);

	/* params need to be added - TODO
	 * scn->enableuartprint = 1;
	 * scn->enablefwlog = 0;
	 * scn->max_no_of_peers = 1;
	 */

	sc->interface = interface;
	if (hif_usb_device_init(sc) != QDF_STATUS_SUCCESS) {
		hif_err("hif_usb_device_init failed");
		goto err_reset;
	}

	if (hif_usb_procfs_init(scn))
		goto err_reset;

	hif_usb_unload_dev_num = usbdev->devnum;
	g_usb_sc = sc;
	HIF_EXIT();
	return QDF_STATUS_SUCCESS;

err_reset:
	hif_usb_diag_write_cold_reset(scn);
	g_usb_sc = NULL;
	hif_usb_unload_dev_num = -1;
	if (target_type != TARGET_TYPE_QCN7605)
		unregister_reboot_notifier(&sc->reboot_notifier);
err_usb:
	ret = QDF_STATUS_E_FAILURE;
	if (target_type != TARGET_TYPE_QCN7605)
		usb_put_dev(usbdev);
	return ret;
}


/**
 * hif_usb_close(): close bus, delete hif_sc
 * @ol_sc: soft_sc struct
 *
 * Return: none
 */
void hif_usb_close(struct hif_softc *scn)
{
	g_usb_sc = NULL;
}

/**
 * hif_usb_disable_bus(): This function disables usb bus
 * @hif_ctx: pointer to struct hif_softc
 *
 * Return: none
 */
void hif_usb_disable_bus(struct hif_softc *hif_ctx)
{
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(hif_ctx);
	struct usb_interface *interface = sc->interface;
	struct usb_device *udev = interface_to_usbdev(interface);
	struct hif_target_info *tgt_info = &hif_ctx->target_info;

	hif_info("trying to remove hif_usb!");

	/* disable lpm to avoid following cold reset will
	 * cause xHCI U1/U2 timeout
	 */
	if (tgt_info->target_type != TARGET_TYPE_QCN7605)
		usb_disable_lpm(udev);

	/* wait for disable lpm */
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(msecs_to_jiffies(DELAY_FOR_TARGET_READY));
	set_current_state(TASK_RUNNING);

	/* do cold reset */
	hif_usb_diag_write_cold_reset(hif_ctx);

	if (g_usb_sc->suspend_state)
		hif_bus_resume(GET_HIF_OPAQUE_HDL(hif_ctx));

	if (tgt_info->target_type != TARGET_TYPE_QCN7605) {
		unregister_reboot_notifier(&sc->reboot_notifier);
		usb_put_dev(udev);
	}

	hif_usb_device_deinit(sc);

	hif_info("hif_usb removed !!!!!!");
}

/**
 * hif_usb_bus_suspend() - suspend the bus
 * @hif_ctx: hif_ctx
 *
 * This function suspends the bus, but usb doesn't need to suspend.
 * Therefore just remove all the pending urb transactions
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_usb_bus_suspend(struct hif_softc *hif_ctx)
{
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(hif_ctx);
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(hif_ctx);

	HIF_ENTER();
	sc->suspend_state = 1;
	usb_hif_flush_all(device);
	HIF_EXIT();
	return 0;
}

/**
 * hif_usb_bus_resume() - hif resume API
 * @hif_ctx: struct hif_opaque_softc
 *
 * This function resumes the bus. but usb doesn't need to resume.
 * Post recv urbs for RX data pipe
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_usb_bus_resume(struct hif_softc *hif_ctx)
{
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(hif_ctx);
	struct HIF_DEVICE_USB *device = HIF_GET_USB_DEVICE(hif_ctx);

	HIF_ENTER();
	sc->suspend_state = 0;
	usb_hif_start_recv_pipes(device);

	HIF_EXIT();
	return 0;
}

/**
 * hif_usb_bus_reset_resume() - resume the bus after reset
 * @scn: struct hif_opaque_softc
 *
 * This function is called to tell the driver that USB device has been resumed
 * and it has also been reset. The driver should redo any necessary
 * initialization. This function resets WLAN SOC.
 *
 * Return: int 0 for success, non zero for failure
 */
int hif_usb_bus_reset_resume(struct hif_softc *hif_ctx)
{
	int ret = 0;

	HIF_ENTER();
	if (hif_usb_diag_write_cold_reset(hif_ctx) != QDF_STATUS_SUCCESS)
		ret = 1;

	HIF_EXIT();
	return ret;
}

/**
 * hif_usb_open()- initialization routine for usb bus
 * @ol_sc: ol_sc
 * @bus_type: bus type
 *
 * Return: QDF_STATUS_SUCCESS on success and error QDF status on failure
 */
QDF_STATUS hif_usb_open(struct hif_softc *hif_ctx,
		enum qdf_bus_type bus_type)
{
	hif_ctx->bus_type = bus_type;
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_usb_disable_isr(): disable isr
 * @hif_ctx: struct hif_softc
 *
 * Return: void
 */
void hif_usb_disable_isr(struct hif_softc *hif_ctx)
{
	/* TODO */
}

/**
 * hif_usb_reg_tbl_attach()- attach hif, target register tables
 * @scn: pointer to ol_softc structure
 *
 * Attach host and target register tables based on target_type, target_version
 *
 * Return: none
 */
void hif_usb_reg_tbl_attach(struct hif_softc *scn)
{
	u_int32_t hif_type, target_type;
	int32_t ret = 0;
	uint32_t chip_id;
	QDF_STATUS rv;
	struct hif_target_info *tgt_info = &scn->target_info;
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);

	if (!scn->hostdef && !scn->targetdef) {
		switch (tgt_info->target_type) {
		case TARGET_TYPE_AR6320:
			switch (tgt_info->target_version) {
			case AR6320_REV1_VERSION:
			case AR6320_REV1_1_VERSION:
			case AR6320_REV1_3_VERSION:
				hif_type = HIF_TYPE_AR6320;
				target_type = TARGET_TYPE_AR6320;
				break;
			case AR6320_REV2_1_VERSION:
			case AR6320_REV3_VERSION:
			case QCA9377_REV1_1_VERSION:
			case QCA9379_REV1_VERSION:
				hif_type = HIF_TYPE_AR6320V2;
				target_type = TARGET_TYPE_AR6320V2;
				break;
			default:
				ret = -1;
				break;
			}
			break;
		default:
			ret = -1;
			break;
		}

		if (ret)
			return;

		/* assign target register table if we find
		 * corresponding type
		 */
		hif_register_tbl_attach(scn, hif_type);
		target_register_tbl_attach(scn, target_type);
		/* read the chip revision*/
		rv = hif_diag_read_access(hif_hdl,
					(CHIP_ID_ADDRESS |
					RTC_SOC_BASE_ADDRESS),
					&chip_id);
		if (rv != QDF_STATUS_SUCCESS) {
			hif_err("get chip id val: %d", rv);
		}
		tgt_info->target_revision =
				CHIP_ID_REVISION_GET(chip_id);
	}
}

/**
 * hif_usb_get_hw_info()- attach register table for USB
 * @hif_ctx: pointer to hif_softc structure

 * This function is used to attach the host and target register tables.
 * Ideally, we should not attach register tables as a part of this function.
 * There is scope of cleanup to move register table attach during
 * initialization for USB bus.
 *
 * The reason we are doing register table attach for USB here is that, it relies
 * on target_info->target_type and target_info->target_version,
 * which get populated during bmi_firmware_download. "hif_get_fw_info" is the
 * only initialization related call into HIF there after.
 *
 * To fix this, we can move the "get target info, functionality currently in
 * bmi_firmware_download into hif initialization functions. This change will
 * affect all buses. Can be taken up as a part of convergence.
 *
 * Return: none
 */
void hif_usb_get_hw_info(struct hif_softc *hif_ctx)
{
	hif_usb_reg_tbl_attach(hif_ctx);
}

#if defined(CONFIG_PLD_USB_CNSS) && !defined(CONFIG_BYPASS_QMI)
/**
 * hif_bus_configure() - configure the bus
 * @scn: pointer to the hif context.
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_usb_bus_configure(struct hif_softc *scn)
{
	struct pld_wlan_enable_cfg cfg;
	enum pld_driver_mode mode;
	uint32_t con_mode = hif_get_conparam(scn);

	if (QDF_GLOBAL_FTM_MODE == con_mode)
		mode = PLD_FTM;
	else if (QDF_GLOBAL_COLDBOOT_CALIB_MODE == con_mode)
		mode = PLD_COLDBOOT_CALIBRATION;
	else if (QDF_IS_EPPING_ENABLED(con_mode))
		mode = PLD_EPPING;
	else
		mode = PLD_MISSION;

	return pld_wlan_enable(scn->qdf_dev->dev, &cfg, mode);
}
#else
/**
 * hif_bus_configure() - configure the bus
 * @scn: pointer to the hif context.
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_usb_bus_configure(struct hif_softc *scn)
{
	return 0;
}
#endif

/**
 * hif_usb_irq_enable() - hif_usb_irq_enable
 * @scn: hif_softc
 * @ce_id: ce_id
 *
 * Return: void
 */
void hif_usb_irq_enable(struct hif_softc *scn, int ce_id)
{
}

/**
 * hif_usb_irq_disable() - hif_usb_irq_disable
 * @scn: hif_softc
 * @ce_id: ce_id
 *
 * Return: void
 */
void hif_usb_irq_disable(struct hif_softc *scn, int ce_id)
{
}

/**
 * hif_usb_shutdown_bus_device() - This function shuts down the device
 * @scn: hif opaque pointer
 *
 * Return: void
 */
void hif_usb_shutdown_bus_device(struct hif_softc *scn)
{
}

/**
 * hif_trigger_dump() - trigger various dump cmd
 * @scn: struct hif_opaque_softc
 * @cmd_id: dump command id
 * @start: start/stop dump
 *
 * Return: None
 */
void hif_trigger_dump(struct hif_opaque_softc *scn, uint8_t cmd_id, bool start)
{
}

/**
 * hif_wlan_disable() - call the platform driver to disable wlan
 * @scn: scn
 *
 * Return: void
 */
void hif_wlan_disable(struct hif_softc *scn)
{
}

/**
 * hif_fw_assert_ramdump_pattern() - handle firmware assert with ramdump pattern
 * @sc: pointer to hif_usb_softc structure
 *
 * Return: void
 */

void hif_fw_assert_ramdump_pattern(struct hif_usb_softc *sc)
{
	uint32_t *reg, pattern, i = 0;
	uint32_t len;
	uint8_t *data;
	uint8_t *ram_ptr = NULL;
	char *fw_ram_seg_name[FW_RAM_SEG_CNT] = {"DRAM", "IRAM", "AXI"};
	size_t fw_ram_reg_size[FW_RAM_SEG_CNT] = {
				  FW_RAMDUMP_DRAMSIZE,
				  FW_RAMDUMP_IRAMSIZE,
				  FW_RAMDUMP_AXISIZE };

	data = sc->fw_data;
	len = sc->fw_data_len;
	pattern = *((uint32_t *) data);

	qdf_assert(sc->ramdump_index < FW_RAM_SEG_CNT);
	i = sc->ramdump_index;
	reg = (uint32_t *) (data + 4);
	if (sc->fw_ram_dumping == 0) {
		sc->fw_ram_dumping = 1;
		hif_info("Firmware %s dump:", fw_ram_seg_name[i]);
		sc->ramdump[i] =
			qdf_mem_malloc(sizeof(struct fw_ramdump) +
					fw_ram_reg_size[i]);
		if (!sc->ramdump[i])
			QDF_BUG(0);

		(sc->ramdump[i])->mem = (uint8_t *) (sc->ramdump[i] + 1);
		fw_ram_seg_addr[i] = (sc->ramdump[i])->mem;
		hif_info("FW %s start addr = %#08x Memory addr for %s = %pK",
			fw_ram_seg_name[i], *reg,
			fw_ram_seg_name[i],
			(sc->ramdump[i])->mem);
		(sc->ramdump[i])->start_addr = *reg;
		(sc->ramdump[i])->length = 0;
	}
	reg++;
	ram_ptr = (sc->ramdump[i])->mem + (sc->ramdump[i])->length;
	(sc->ramdump[i])->length += (len - 8);
	if (sc->ramdump[i]->length <= fw_ram_reg_size[i]) {
		qdf_mem_copy(ram_ptr, (uint8_t *) reg, len - 8);
	} else {
		hif_err("memory copy overlap");
		QDF_BUG(0);
	}

	if (pattern == FW_RAMDUMP_END_PATTERN) {
		hif_err("%s memory size = %d", fw_ram_seg_name[i],
			(sc->ramdump[i])->length);
		if (i == (FW_RAM_SEG_CNT - 1))
			QDF_BUG(0);

		sc->ramdump_index++;
		sc->fw_ram_dumping = 0;
	}
}

/**
 * hif_usb_ramdump_handler(): dump bus debug registers
 * @scn: struct hif_opaque_softc
 *
 * This function is to receive information of firmware crash dump, and
 * save it in host memory. It consists of 5 parts: registers, call stack,
 * DRAM dump, IRAM dump, and AXI dump, and they are reported to host in order.
 *
 * registers: wrapped in a USB packet by starting as FW_ASSERT_PATTERN and
 *            60 registers.
 * call stack: wrapped in multiple USB packets, and each of them starts as
 *             FW_REG_PATTERN and contains multiple double-words. The tail
 *             of the last packet is FW_REG_END_PATTERN.
 * DRAM dump: wrapped in multiple USB pakcets, and each of them start as
 *            FW_RAMDUMP_PATTERN and contains multiple double-wors. The tail
 *            of the last packet is FW_RAMDUMP_END_PATTERN;
 * IRAM dump and AXI dump are with the same format as DRAM dump.
 *
 * Return: 0 for success or error code
 */

void hif_usb_ramdump_handler(struct hif_opaque_softc *scn)
{
	uint32_t *reg, pattern, i, start_addr = 0;
	uint32_t len;
	uint8_t *data;
	uint8_t str_buf[128];
	uint32_t remaining;
	struct hif_usb_softc *sc = HIF_GET_USB_SOFTC(scn);
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(scn);
	struct hif_target_info *tgt_info = &hif_ctx->target_info;

	data = sc->fw_data;
	len = sc->fw_data_len;
	pattern = *((uint32_t *) data);

	if (pattern == FW_ASSERT_PATTERN) {
		hif_err("Firmware crash detected...");
		hif_err("target_type: %d target_version: %d target_revision: %d",
			tgt_info->target_type,
			tgt_info->target_version,
			tgt_info->target_revision);

		reg = (uint32_t *) (data + 4);
		print_hex_dump(KERN_DEBUG, " ", DUMP_PREFIX_OFFSET, 16, 4, reg,
				min_t(uint32_t, len - 4, FW_REG_DUMP_CNT * 4),
				false);
		sc->fw_ram_dumping = 0;

	} else if (pattern == FW_REG_PATTERN) {
		reg = (uint32_t *) (data + 4);
		start_addr = *reg++;
		if (sc->fw_ram_dumping == 0) {
			qdf_nofl_err("Firmware stack dump:");
			sc->fw_ram_dumping = 1;
			fw_stack_addr = start_addr;
		}
		remaining = len - 8;
		/* len is in byte, but it's printed in double-word. */
		for (i = 0; i < (len - 8); i += 16) {
			if ((*reg == FW_REG_END_PATTERN) && (i == len - 12)) {
				sc->fw_ram_dumping = 0;
				qdf_nofl_err("Stack start address = %#08x",
					     fw_stack_addr);
				break;
			}
			hex_dump_to_buffer(reg, remaining, 16, 4, str_buf,
						sizeof(str_buf), false);
			qdf_nofl_err("%#08x: %s", start_addr + i, str_buf);
			remaining -= 16;
			reg += 4;
		}
	} else if ((!sc->enable_self_recovery) &&
			((pattern & FW_RAMDUMP_PATTERN_MASK) ==
						FW_RAMDUMP_PATTERN)) {
		hif_fw_assert_ramdump_pattern(sc);
	}
}

#ifndef QCA_WIFI_3_0
/**
 * hif_check_fw_reg(): hif_check_fw_reg
 * @scn: scn
 * @state:
 *
 * Return: int
 */
int hif_check_fw_reg(struct hif_opaque_softc *scn)
{
	return 0;
}
#endif

/**
 * hif_usb_needs_bmi() - return true if the soc needs bmi through the driver
 * @scn: hif context
 *
 * Return: true if soc needs driver bmi otherwise false
 */
bool hif_usb_needs_bmi(struct hif_softc *scn)
{
	struct hif_target_info *tgt_info = &scn->target_info;

	/* BMI is not supported in Genoa */
	if (tgt_info->target_type == TARGET_TYPE_QCN7605)
		return false;

	return true;
}

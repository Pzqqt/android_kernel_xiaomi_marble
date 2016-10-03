/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_HDD_DRIVER_OPS_H__
#define __WLAN_HDD_DRIVER_OPS_H__

#include "hif.h"

/**
 * DOC: wlan_hdd_driver_ops.h
 *
 * Functions to register the wlan driver.
*/
int wlan_hdd_register_driver(void);
void wlan_hdd_unregister_driver(void);

/**
 * wlan_hdd_bus_suspend() - suspend the wlan bus
 *
 * This function is called by the platform driver to suspend the
 * wlan bus
 *
 * @state: state
 *
 * Return: QDF_STATUS
 */
int wlan_hdd_bus_suspend(pm_message_t state);

/**
 * wlan_hdd_bus_suspend_noirq() - handle .suspend_noirq callback
 *
 * This function is called by the platform driver to complete the
 * bus suspend callback when device interrupts are disabled by kernel.
 * Call HIF and WMA suspend_noirq callbacks to make sure there is no
 * wake up pending from FW before allowing suspend.
 *
 * Return: 0 for success and -EBUSY if FW is requesting wake up
 */
int wlan_hdd_bus_suspend_noirq(void);

/**
 * wlan_hdd_bus_resume(): wake up the bus
 *
 * This function is called by the platform driver to resume wlan
 * bus
 *
 * Return: void
 */
int wlan_hdd_bus_resume(void);

/**
 * wlan_hdd_bus_resume_noirq(): handle bus resume no irq
 *
 * This function is called by the platform driver to do bus
 * resume no IRQ before calling resume callback. Call WMA and HIF
 * layers to complete the resume_noirq.
 *
 * Return: 0 for success and negative error code for failure
 */
int wlan_hdd_bus_resume_noirq(void);
void hdd_hif_close(void *hif_ctx);
int hdd_hif_open(struct device *dev, void *bdev, const hif_bus_id *bid,
		 enum qdf_bus_type bus_type, bool reinit);
#endif /* __WLAN_HDD_DRIVER_OPS_H__ */

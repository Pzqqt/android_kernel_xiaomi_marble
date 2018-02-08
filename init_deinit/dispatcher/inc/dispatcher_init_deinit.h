/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file provides various init/deinit trigger point for new
 * components.
 */

#if !defined(__DISPATCHER_INIT_H)
#define __DISPATCHER_INIT_H

#include <qdf_types.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>

/* Function pointer for spectral pdev open handler */
typedef QDF_STATUS (*spectral_pdev_open_handler)(
		struct wlan_objmgr_pdev *pdev);

/**
 * dispatcher_init(): API to init all new components
 *
 * This API calls all new components init APIs. This is invoked
 * from HDD/OS_If layer during:
 * 1) Driver load sequence
 * 2) before probing the attached device.
 * 3) FW is not ready
 * 4) WMI channel is not established
 *
 * A component can't communicate with FW during init stage.
 *
 * Return: none
 */
QDF_STATUS dispatcher_init(void);

/**
 * dispatcher_deinit(): API to de-init all new components
 *
 * This API calls all new components de-init APIs. This is invoked
 * from HDD/OS_If layer during:
 * 1) Driver unload sequence
 * 2) FW is dead
 * 3) WMI channel is destroyed
 * 4) all PDEV and PSOC objects are destroyed
 *
 * A component can't communicate with FW during de-init stage.
 *
 * Return: none
 */
QDF_STATUS dispatcher_deinit(void);

/**
 * dispatcher_enable(): global (above psoc) level component start
 *
 * Prepare components to service requests. Must only be called after
 * dispatcher_init().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dispatcher_enable(void);

/**
 * dispatcher_disable(): global (above psoc) level component stop
 *
 * Stop components from servicing requests. Must be called before
 * scheduler_deinit().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dispatcher_disable(void);

/**
 * dispatcher_psoc_open(): API to trigger PSOC open for all new components
 * @psoc: psoc context
 *
 * This API calls all new components PSOC OPEN APIs. This is invoked from
 * HDD/OS_If layer during:
 * 1) Driver load sequence
 * 2) PSOC object is created
 * 3) FW is not yet ready
 * 4) WMI channel is not yet established with FW
 *
 * PSOC open happens before FW WMI ready and hence a component can't
 * communicate with FW during PSOC open sequence.
 *
 * Return: none
 */
QDF_STATUS dispatcher_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * dispatcher_psoc_close(): API to trigger PSOC close for all new components
 * @psoc: psoc context
 *
 * This API calls all new components PSOC CLOSE APIs. This is invoked from
 * HDD/OS_If layer during:
 * 1) Driver unload sequence
 * 2) PSOC object is destroyed
 * 3) FW is already dead(PDEV suspended)
 * 4) WMI channel is destroyed with FW
 *
 * A component can't communicate with FW during PSOC close.
 *
 * Return: none
 */
QDF_STATUS dispatcher_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * dispatcher_psoc_enable(): API to trigger PSOC enable(start) for all new
 *                           components
 * @psoc: psoc context
 *
 * This API calls all new components PSOC enable(start) APIs. This is invoked
 * from HDD/OS_If layer during:
 * 1) Driver load sequence
 * 2) PSOC object is created
 * 3) WMI endpoint and WMI channel is ready with FW
 * 4) WMI FW ready event is also received from FW.
 *
 * FW is already ready and WMI channel is established by this time so a
 * component can communicate with FW during PSOC enable sequence.
 *
 * Return: none
 */
QDF_STATUS dispatcher_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * dispatcher_psoc_disable(): API to trigger PSOC disable(stop) for all new
 *                            components
 * @psoc: psoc context
 *
 * This API calls all new components PSOC disable(stop) APIs. This is invoked
 * from HDD/OS_If layer during:
 * 1) Driver unload sequence
 * 2) WMI channel is still available
 * 3) FW is still running and up
 * 4) PSOC object is not destroyed
 *
 * A component should abort all its ongign transaction with FW at this stage
 * for example scan component needs to abort all its ongoing scan in FW because
 * is goign to be stopped very soon.
 *
 * Return: none
 */
QDF_STATUS dispatcher_psoc_disable(struct wlan_objmgr_psoc *psoc);

/**
 * dispatcher_pdev_open(): API to trigger PDEV open for all new components
 * @pdev: pdev context
 *
 * This API calls all new components PDEV OPEN APIs. This is invoked from
 * during PDEV object is created.
 *
 * Return: none
 */
QDF_STATUS dispatcher_pdev_open(struct wlan_objmgr_pdev *pdev);

/**
 * dispatcher_pdev_close(): API to trigger PDEV close for all new components
 * @pdev: pdev context
 *
 * This API calls all new components PDEV CLOSE APIs. This is invoked from
 * during driver unload sequence.
 *
 * Return: none
 */
QDF_STATUS dispatcher_pdev_close(struct wlan_objmgr_pdev *pdev);

/**
 * dispatcher_register_spectral_pdev_open_handler():
 * API to register spectral pdev open handler
 * @handler: pdev open handler
 *
 * This API registers spectral pdev open handler.
 *
 * Return: none
 */
QDF_STATUS dispatcher_register_spectral_pdev_open_handler(QDF_STATUS (*handler)
				(struct wlan_objmgr_pdev *pdev));

#endif /* End of  !defined(__DISPATCHER_INIT_H) */

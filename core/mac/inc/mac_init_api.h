/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 *
 * mac_init_api.c - Header file for mac level init functions
 * Author:    Dinesh Upadhyay
 * Date:      04/23/2007
 * History:-
 * Date       Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */
#ifndef __MAC_INIT_API_H
#define __MAC_INIT_API_H

#include "ani_global.h"
#include "sir_types.h"

QDF_STATUS mac_start(tHalHandle hHal, void *pHalMacStartParams);

/**
 * mac_stop() - Stop all MAC modules
 * @mac_handle: Opaque handle to the MAC context
 *
 * This function is called to stop MAC. This function will stop all
 * the mac modules.
 *
 * Return: QDF_STATUS_SUCCESS if the MAC was successfully stopped. Any
 *         other value means that there was an issue with stopping the
 *         MAC, but the caller should still consider the MAC to be
 *         stopped.
 */
QDF_STATUS mac_stop(mac_handle_t mac_handle);

QDF_STATUS mac_open(struct wlan_objmgr_psoc *psoc, tHalHandle *pHalHandle,
		    hdd_handle_t hdd_handle, struct cds_config_info *cds_cfg);
QDF_STATUS mac_close(tHalHandle hHal);

#endif /* __MAC_INIT_API_H */

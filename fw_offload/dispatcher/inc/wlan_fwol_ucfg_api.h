/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: declare internal API related to the fwol component
 */

#ifndef _WLAN_FWOL_UCFG_API_H_
#define _WLAN_FWOL_UCFG_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>

/**
 * ucfg_fwol_psoc_open() - FWOL component Open
 * @psoc: pointer to psoc object
 *
 * Open the FWOL component and initialize the FWOL structure
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_fwol_psoc_close() - FWOL component close
 * @psoc: pointer to psoc object
 *
 * Close the FWOL component and clear the FWOL structures
 *
 * Return: None
 */
void ucfg_fwol_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_fwol_init() - initialize fwol_ctx context.
 *
 * This function initializes the fwol context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
QDF_STATUS ucfg_fwol_init(void);

/**
 * ucfg_fwol_deinit() - De initialize fwol_ctx context.
 *
 * This function De initializes fwol contex.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
void ucfg_fwol_deinit(void);

#endif /* _WLAN_FWOL_UCFG_API_H_ */

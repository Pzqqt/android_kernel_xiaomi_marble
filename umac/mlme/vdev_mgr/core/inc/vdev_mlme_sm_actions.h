/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Define VDEV MLME SM actions APIs
 */
#ifndef _VDEV_MLME_SM_ACTIONS_H_
#define _VDEV_MLME_SM_ACTIONS_H_

#include <include/wlan_pdev_mlme.h>
#include <include/wlan_vdev_mlme.h>
/**
 * mlme_register_cmn_ops - VDEV SM actions ops registration
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API assigns action callbacks
 *
 * Return: QDF_STATUS_SUCCESS on successful assignment
 *         QDF_STATUS_E_FAILURE, if it fails due to any
 */
QDF_STATUS mlme_register_cmn_ops(struct vdev_mlme_obj *mlme);

/**
 * mlme_restart_timer_init() - Initialize restart timer
 * @pdev: Object manager PDEV object
 *
 * API to initialize MLME VDEV restart timer
 *
 * Return: void
 */
void mlme_restart_timer_init(struct pdev_mlme_obj *pdev_mlme);

/**
 * mlme_restart_timer_delete() - Delete restart timer
 * @pdev: Object manager PDEV object
 *
 * API to free MLME VDEV restart timer
 *
 * Return: void
 */
void mlme_restart_timer_delete(struct pdev_mlme_obj *pdev_mlme);

#endif

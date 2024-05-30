/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: declare the ftm service data structure and apis
 */
#ifndef _TARGET_IF_FTM_H_
#define _TARGET_IF_FTM_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>

/**
 * target_if_ftm_register_tx_ops() - register ftm tx ops
 * @tx_ops: tx ops pointer
 *
 * Register ftm tx ops
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_ftm_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_ftm_attach() - Register FW event handler
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_ftm_attach(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_ftm_detach() - De-Register FW event handler
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_ftm_detach(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_ftm_cmd_send() - Send WMI command for FTM requests
 * @pdev: pdev pointer
 * buf: data to be sent to FW
 * len: length of the data
 * pdev_id: pdev id
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_ftm_cmd_send(struct wlan_objmgr_pdev *pdev,
				uint8_t *buf, uint32_t len, uint8_t pdev_id);
#endif /* _TARGET_IF_FTM_H_ */

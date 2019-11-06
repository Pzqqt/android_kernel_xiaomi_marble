/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_SPECTRAL_UCFG_API_H_
#define _WLAN_SPECTRAL_UCFG_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_spectral_public_structs.h>

/* Spectral specific UCFG set operations */

/**
 * ucfg_spectral_control() - Carry out Spectral control operations
 * @pdev: Pointer to pdev
 * @sscan_req: spectral related control request
 *
 * Carry out Spectral specific UCFG control get/set operations
 *
 * Return: 0 on success, negative value on failure
 */
QDF_STATUS ucfg_spectral_control(struct wlan_objmgr_pdev *pdev,
				 struct spectral_cp_request *sscan_req);

/**
 * ucfg_spectral_scan_set_ppid() - configure pid of spectral tool
 * @pdev: Pointer to pdev
 * @ppid: Spectral tool pid
 *
 * Configure pid of spectral tool
 *
 * Return: None
 */
void ucfg_spectral_scan_set_ppid(struct wlan_objmgr_pdev *pdev,
					uint32_t ppid);

/**
 * ucfg_spectral_create_cp_req() - Create Spectral control path request
 * @sscan_req: Pointer to Spectral scan request
 * @indata: pointer input data
 * @insize: Size of input data
 *
 * Create Spectral control path request structure
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS ucfg_spectral_create_cp_req(struct spectral_cp_request *sscan_req,
				       void *indata, u_int32_t insize);

/**
 * ucfg_spectral_create_cp_req() - Extract response from Spectral CP request
 * @sscan_req: Pointer to Spectral scan request
 * @outdata: pointer output data
 * @outsize: Size of output data
 *
 * Extract response from Spectral control path request
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS ucfg_spectral_extract_response(struct spectral_cp_request *sscan_req,
					  void *outdata, u_int32_t *outsize);

/**
 * ucfg_spectral_register_to_dbr() - Register spectral to DBR
 * @pdev: Pointer to pdev object
 *
 * Register spectral to Direct Buffer RX component
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS ucfg_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_SPECTRAL_UCFG_API_H_ */

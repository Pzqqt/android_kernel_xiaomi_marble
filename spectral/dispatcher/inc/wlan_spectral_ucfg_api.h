/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * @id: Spectral operation ID
 * @indata: Pointer to input data
 * @insize: Size of indata buffer
 * @outdata: Pointer to buffer where the output should be stored
 * @outsize: Size of outdata buffer
 *
 * Carry out Spectral specific UCFG control get/set operations
 *
 * Return: 0 on success, negative value on failure
 */
int ucfg_spectral_control(struct wlan_objmgr_pdev *pdev,
			  u_int id,
			  void *indata,
			  uint32_t insize, void *outdata, uint32_t *outsize);

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

#endif /* _WLAN_SPECTRAL_UCFG_API_H_ */

/*
 * Copyright (c) 2018, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: init_cmd_api.h
 *
 * Public APIs to prepare and send init command
 */

#ifndef _INIT_DEINIT_INIT_CMD_H_
#define _INIT_DEINIT_INIT_CMD_H_

/* max size if 256k */
#define HOST_MEM_CHUNK_MAX_SIZE (256 * 1024)
#define HOST_MEM_CHUNK_MAX_SIZE_POWER2 (8 + 10)
#define TXBF_CV_POOL0 2
#define TXBF_CV_POOL1 3
#define TXBF_CV_POOL2 4
#define CFR_CAPTURE_HOST_MEM_REQ_ID 9
#define HOST_CONTIGUOUS_MEM_CHUNK_REQUIRED 0x8

/**
 * init_deinit_handle_host_mem_req() - handle host memory request
 * @psoc: PSOC object
 * @tgt_info: PSOC_INFO object
 * @event: Event buffer from FW
 *
 * API to handle memory request from FW and allocate memory chunks
 *
 * Return: SUCCESS on successful memory allocation
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS init_deinit_handle_host_mem_req(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info, uint8_t *event);

/**
 * init_deinit_free_num_units() - Free allocated mem chunks
 * @psoc: PSOC object
 * @tgt_info: PSOC_INFO object
 *
 * API to free memory
 *
 * Return: SUCCESS on successful memory free
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS init_deinit_free_num_units(struct wlan_objmgr_psoc *psoc,
			struct target_psoc_info *tgt_hdl);

/**
 * init_deinit_derive_band_to_mac_param() - Derive band to mac param
 * @psoc: PSOC object
 * @tgt_info: PSOC_INFO object
 * @init_param: Pointer to init param
 *
 * API to derive band to mac param
 *
 * Return: void
 */
void init_deinit_derive_band_to_mac_param(
		struct wlan_objmgr_psoc *psoc,
		struct target_psoc_info *tgt_info,
		struct wmi_init_cmd_param *init_param);

/**
 * init_deinit_prepare_send_init_cmd() - prepare send init cmd
 * @psoc: PSOC object
 * @tgt_info: PSOC_INFO object
 *
 * API to prepare send init command
 *
 * Return: void
 */
void init_deinit_prepare_send_init_cmd(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);

#endif /* _INIT_DEINIT_INIT_CMD_H_*/

/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

struct whal_cfir_dma_hdr {
	uint16_t
		// 'BA'
		tag                 : 8,
		// '02', length of header in 4 octet units
		length              : 6,
		// 00
		reserved            : 2;
	uint16_t
		// [16]
		upload_done         : 1,
		// [17:18], 0: invalid, 1: CFR, 2: CIR, 3: DebugH
		capture_type        : 3,
		// [19:20], 0: Legacy, 1: HT, 2: VHT, 3: HE
		preamble_type       : 2,
		// [21:23], 0: 1-stream, 1: 2-stream, ..., 7: 8-stream
		nss                 : 3,
		// [24:27], 0: invalid, 1: 1-chain, 2: 2-chain, etc.
		num_chains          : 3,
		// [28:30], 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 MHz
		upload_pkt_bw       : 3,    // [31]
		sw_peer_id_valid    : 1;
	uint16_t
		sw_peer_id          : 16;   // [15:0]
	uint16_t
		phy_ppdu_id         : 16;   // [15:0]
};

/**
 * cfr_8074v2_init_pdev() - Inits cfr pdev and registers necessary handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: Registration status for necessary handlers
 */
int cfr_8074v2_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * cfr_8074v2_deinit_pdev() - De-inits corresponding pdev and handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: De-registration status for necessary handlers
 */
int cfr_8074v2_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * target_if_register_to_dbr() - Register to Direct DMA handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
QDF_STATUS
target_if_register_to_dbr(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_register_tx_completion_event_handler()
 * register TX completion handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
int
target_if_register_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_unregister_tx_completion_event_handler
 * unregister TX completion handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
int
target_if_unregister_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc);


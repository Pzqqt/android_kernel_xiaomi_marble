/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#ifndef _DP_TXRX_ME_H_
#define _DP_TXRX_ME_H_
uint16_t dp_tx_me_send_convert_ucast(struct cdp_vdev *vdev_handle,
				     qdf_nbuf_t nbuf,
				     uint8_t newmac[][QDF_MAC_ADDR_SIZE],
				     uint8_t new_mac_cnt);
void dp_tx_me_alloc_descriptor(struct cdp_pdev *pdev);

void dp_tx_me_free_descriptor(struct cdp_pdev *pdev);
void dp_tx_me_exit(struct dp_pdev *pdev);
QDF_STATUS
dp_tx_prepare_send_me(struct dp_vdev *vdev, qdf_nbuf_t nbuf);
#endif

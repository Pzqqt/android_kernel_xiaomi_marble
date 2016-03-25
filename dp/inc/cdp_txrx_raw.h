/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */
 /**
 * @file cdp_txrx_raw.h
 * @brief Define the host data path raw mode API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_RAW_H_
#define _CDP_TXRX_RAW_H_


/* TODO: adf need to be replaced with qdf */
extern int ol_txrx_get_nwifi_mode(ol_txrx_vdev_handle vdev);
#define OL_TXRX_GET_NWIFI_MODE(vdev)  ol_txrx_get_nwifi_mode(vdev)
/* Questionable -- should this be in OL AND/OR is this used? */
/* Called by ol_tx_ll_umac_raw_process() */
/**
 * @brief encap nbuf(s) from Ethernet II format to 802.11 Raw format
 * @details
 *  Note that in the case of IP fragments, the function takes two fragments at a
 *  time and creates an A-MSDU. If it has seen the first of such fragments, it
 *  returns 0 to indicate that it needs to consume one more. In this case, the
 *  caller shouldn't pass the nbuf to lower layers.
 *  The function is simple and doesn't dynamically take decisions on A-MSDU
 *  formation. It can be extended to pack more fragments into an A-MSDU if
 *  required, but in this case there can be greater losses due to the
 *  environment. The objective of the function is only to simulate regular
 *  scatter/gather.
 *
 * @param vdev - the data virtual device object
 * @param pnbuf - pointer to nbuf
 *
 * @return - 0 on success, -1 on error, 1 if more nbufs need to be consumed.
 */

int
ol_rsim_tx_encap(ol_txrx_vdev_handle vdev, qdf_nbuf_t *pnbuf);
#endif

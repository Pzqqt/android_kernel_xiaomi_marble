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
 * @file cdp_txrx_me.h
 * @brief Define the host data path mcast enhance API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_ME_H_
#define _CDP_TXRX_ME_H_

/* TODO: adf need to be replaced with qdf */

#if ATH_SUPPORT_ME_FW_BASED

extern u_int16_t
ol_tx_desc_alloc_and_mark_for_mcast_clone(struct ol_txrx_pdev_t *pdev, u_int16_t
buf_count)

extern u_int16_t
ol_tx_desc_free_and_unmark_for_mcast_clone(struct ol_txrx_pdev_t *pdev,
	u_int16_t buf_count);

extern u_int16_t
ol_tx_get_mcast_buf_allocated_marked(struct ol_txrx_pdev_t *pdev);
#else
extern void
ol_tx_me_alloc_descriptor(struct ol_txrx_pdev_t *pdev);

extern void
ol_tx_me_free_descriptor(struct ol_txrx_pdev_t *pdev);

extern uint16_t
ol_tx_me_convert_ucast(ol_txrx_vdev_handle vdev, qdf_nbuf_t wbuf,
		u_int8_t newmac[][6], uint8_t newmaccnt);
#endif
/* Should be a function pointer in ol_txrx_osif_ops{} */
#if ATH_MCAST_HOST_INSPECT
/**
 * @brief notify mcast frame indication from FW.
 * @details
 *      This notification will be used to convert
 *      multicast frame to unicast.
 *
 * @param pdev - handle to the ctrl SW's physical device object
 * @param vdev_id - ID of the virtual device received the special data
 * @param msdu - the multicast msdu returned by FW for host inspect
 */

int ol_mcast_notify(ol_pdev_handle pdev,
	u_int8_t vdev_id, qdf_nbuf_t msdu);
#endif

#endif




/*
 * Copyright (c) 2011-2016,2018 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_ctrl.h
 * @brief Define the host data path control API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_CTRL_DEF_H_
#define _CDP_TXRX_CTRL_DEF_H_
/* TODO: adf need to be replaced with qdf */
/*
 * Cleanups --  Might need cleanup
 */
#if !QCA_OL_TX_PDEV_LOCK && QCA_NSS_PLATFORM || \
	(defined QCA_PARTNER_PLATFORM && QCA_PARTNER_SUPPORT_FAST_TX)
#define VAP_TX_SPIN_LOCK(_x) spin_lock(_x)
#define VAP_TX_SPIN_UNLOCK(_x) spin_unlock(_x)
#else /* QCA_OL_TX_PDEV_LOCK */
#define VAP_TX_SPIN_LOCK(_x)
#define VAP_TX_SPIN_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if QCA_OL_TX_PDEV_LOCK
void ol_ll_pdev_tx_lock(void *);
void ol_ll_pdev_tx_unlock(void *);
#define OL_TX_LOCK(_x)  ol_ll_pdev_tx_lock(_x)
#define OL_TX_UNLOCK(_x) ol_ll_pdev_tx_unlock(_x)

#define OL_TX_PDEV_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_PDEV_UNLOCK(_x) qdf_spin_unlock_bh(_x)
#else
#define OL_TX_PDEV_LOCK(_x)
#define OL_TX_PDEV_UNLOCK(_x)

#define OL_TX_LOCK(_x)
#define OL_TX_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if !QCA_OL_TX_PDEV_LOCK
#define OL_TX_FLOW_CTRL_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OL_TX_DESC_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_DESC_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OSIF_VAP_TX_LOCK(_y, _x)  spin_lock(&((_x)->tx_lock))
#define OSIF_VAP_TX_UNLOCK(_y, _x)  spin_unlock(&((_x)->tx_lock))

#define OL_TX_PEER_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) \
	qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) \
	qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#else
#define OSIF_VAP_TX_LOCK(_y, _x)  cdp_vdev_tx_lock( \
			_y, wlan_vdev_get_dp_handle((_x)->ctrl_vdev))
#define OSIF_VAP_TX_UNLOCK(_y, _x) cdp_vdev_tx_unlock( \
			_y, wlan_vdev_get_dp_handle((_x)->ctrl_vdev))

#define OL_TX_FLOW_CTRL_LOCK(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x)

#define OL_TX_DESC_LOCK(_x)
#define OL_TX_DESC_UNLOCK(_x)

#define OL_TX_PEER_LOCK(_x, _id)
#define OL_TX_PEER_UNLOCK(_x, _id)

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->tx_lock))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->tx_lock))

#endif /* !QCA_OL_TX_PDEV_LOCK */
#endif


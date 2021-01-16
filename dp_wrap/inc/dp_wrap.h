/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef _DP_WRAP_H_
#define _DP_WRAP_H__

#if ATH_SUPPORT_WRAP
#if !WLAN_QWRAP_LEGACY

#include "dp_wrap_struct.h"
#include <dp_txrx.h>
#include <cdp_txrx_cmn.h>
#include <wlan_osif_priv.h>

/**
 * dp_wrap_get_pdev_handle() - get wrap handle from pdev handle
 *
 * @pdev: handle to the objmgr pdev
 * Return: handle to wrap_pdev
 */
static inline
dp_pdev_wrap_t *dp_wrap_get_pdev_handle(struct wlan_objmgr_pdev *pdev)
{
	ol_txrx_soc_handle soc;
	dp_txrx_pdev_handle_t *dp_hdl;

	soc = wlan_psoc_get_dp_handle(wlan_pdev_get_psoc(pdev));

	if (qdf_unlikely(!soc)) {
		qwrap_err(" soc handle is NULL");
		return NULL;
	}
	dp_hdl = (dp_txrx_pdev_handle_t *)cdp_pdev_get_dp_txrx_handle(soc,
		wlan_objmgr_pdev_get_pdev_id(pdev));

	if (qdf_unlikely(!dp_hdl)) {
		qwrap_err(" dp txrx handle is NULL");
		return NULL;
	}
	return &dp_hdl->wrap_pdev_hdl;
}

/**
 * dp_wrap_get_vdev_handle() - get wrap handle from vdev handle
 *
 * @vdev: handle to the objmgr vdev
 * Return: handle to wrap_vdev
 */
static inline
dp_vdev_wrap_t *dp_wrap_get_vdev_handle(struct wlan_objmgr_vdev *vdev)
{
	ol_txrx_soc_handle soc;
	dp_vdev_txrx_handle_t *dp_hdl;

	soc = wlan_psoc_get_dp_handle(
		wlan_pdev_get_psoc(wlan_vdev_get_pdev(vdev)));

	if (qdf_unlikely(!soc)) {
		qwrap_err(" soc handle is NULL");
		return NULL;
	}

	dp_hdl = (dp_vdev_txrx_handle_t *)cdp_vdev_get_dp_ext_txrx_handle(soc,
			wlan_vdev_get_id(vdev));

	if (qdf_unlikely(!dp_hdl)) {
		qwrap_err(" dp txrx handle is NULL");
		return NULL;
	}

	return &dp_hdl->wrap_vdev_hdl;
}

/**
 * dp_wrap_get_mpsta_vdev() - get mpsta vdev
 *
 * @pdev: handle to pdev obj mgr
 * Return: mpsta vdev object
 */

static
struct wlan_objmgr_vdev *dp_wrap_get_mpsta_vdev(struct wlan_objmgr_pdev *pdev)
{
	struct dp_wrap_pdev *wrap_pdev;

	if (!pdev)
		return NULL;

	wrap_pdev = dp_wrap_get_pdev_handle(pdev);
	if (!wrap_pdev)
		return NULL;
	return wrap_pdev->mpsta_vdev;
}

/**
 * dp_wrap_get_vdev() - get mpsta vdev
 *
 * @pdev: handle to pdev obj mgr
 * Return: wrap vdev object
 */

static
struct wlan_objmgr_vdev *dp_wrap_get_vdev(struct wlan_objmgr_pdev *pdev)
{
	struct dp_wrap_pdev *wrap_pdev;

	if (!pdev)
		return NULL;

	wrap_pdev = dp_wrap_get_pdev_handle(pdev);
	if (!wrap_pdev)
		return NULL;
	return wrap_pdev->wrap_vdev;
}

/**
 * dp_wrap_vdev_is_mat_set() - Indicates whether mat translation is set
 *
 * @vdev: handle to the objmgr vdev.
 * Return: true if mat is set, false otherwise
 */
static inline bool dp_wrap_vdev_is_mat_set(struct wlan_objmgr_vdev *vdev)
{
	if (vdev && wlan_vdev_mlme_feat_ext_cap_get(vdev, WLAN_VDEV_FEXT_MAT))
		return true;
	else
		return false;
}

/**
 * dp_wrap_vdev_is_wired_psta() - Indicates whether vdev is wired psta.
 *
 * @vdev: handle to the objmgr vdev.
 * Return: true if wired psta, false otherwise.
 */
static inline bool dp_wrap_vdev_is_wired_psta(struct wlan_objmgr_vdev *vdev)
{
	if (vdev && wlan_vdev_mlme_feat_ext_cap_get(
		vdev, WLAN_VDEV_FEXT_WIRED_PSTA))
		return true;
	else
		return false;
}

/**
 * dp_wrap_vdev_is_psta() - Indicates whether vdev is of type PSTA.
 *
 * @vdev: handle to the objmgr vdev.
 * Return: True if psta, false otherwise
 */
static inline bool dp_wrap_vdev_is_psta(struct wlan_objmgr_vdev *vdev)
{
	if (vdev && wlan_vdev_mlme_feat_ext_cap_get(vdev, WLAN_VDEV_FEXT_PSTA))
		return true;
	else
		return false;
}

/**
 * dp_wrap_vdev_is_mpsta() - Indicates whether vdev is of type MPSTA.
 *
 * @vdev: handle to the objmgr vdev.
 * Return: True if mpsta, false otherwise.
 */
static inline bool dp_wrap_vdev_is_mpsta(struct wlan_objmgr_vdev *vdev)
{
	if (vdev && wlan_vdev_mlme_feat_ext_cap_get(
		vdev, WLAN_VDEV_FEXT_MPSTA))
		return true;
	else
		return false;
}

/**
 * dp_wrap_vdev_is_wrap() - Indicates whether vdev is of type WRAP.
 *
 * @vdev: handle to the objmgr vdev.
 * Return: True if wrap, false otherwise.
 */
static bool dp_wrap_vdev_is_wrap(struct wlan_objmgr_vdev *vdev)
{
	if (vdev && wlan_vdev_mlme_feat_ext_cap_get(vdev, WLAN_VDEV_FEXT_WRAP))
		return true;
	else
		return false;
}

/**
 * dp_wrap_vdev_get_nwrapvaps() - Get number of wrap vaps
 *
 * @pdev: handle to the objmgr pdev.
 * Return: wrap vaps count
 */
static int dp_wrap_vdev_get_nwrapvaps(struct wlan_objmgr_pdev *pdev)
{
	struct dp_wrap_pdev *wpdev;

	if (pdev) {
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wpdev is NULL");
			return 0;
		}
		return wpdev->nwrapvaps;
	}
	return 0;
}

/**
 * dp_wrap_vdev_get_npstavaps() - Get number of psta vaps
 *
 * @pdev: handle to the objmgr pdev.
 * Return: psta vaps count
 */
static int dp_wrap_vdev_get_npstavaps(struct wlan_objmgr_pdev *pdev)
{
	struct dp_wrap_pdev *wpdev;

	if (pdev) {
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wpdev is NULL");
			return 0;
		}
		return wpdev->npstavaps;
	}
	return 0;
}

/**
 * dp_wrap_pdev_set_isolation() - set isolation
 *
 * @pdev: handle to the objmgr pdev.
 * @isolation: handle to set isolation param
 * Return: void
 */
static void
dp_wrap_pdev_set_isolation(struct wlan_objmgr_pdev *pdev, int isolation)
{
	struct dp_wrap_pdev *wpdev;

	if (pdev) {
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wpdev is NULL");
			return;
		}
		wpdev->wp_isolation = isolation;
	}
}

/**
 * dp_wrap_pdev_get_isolation() - Indicates whether isolation is set
 *
 * @pdev: handle to the objmgr pdev.
 * Return: isolation value
 */
static int dp_wrap_pdev_get_isolation(struct wlan_objmgr_pdev *pdev)
{
	struct dp_wrap_pdev *wpdev;

	if (pdev) {
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wpdev is NULL");
			return 0;
		}
		return wpdev->wp_isolation;
	}
	return 0;
}

/**
 * dp_wrap_vdev_get_oma() - Get OMA address of PSTA
 *
 * @pdev: handle to the objmgr vdev.
 * Return: PSTA OMA address
 */
static u_int8_t *dp_wrap_vdev_get_oma(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_vdev *wvdev;

	if (vdev) {
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wvdev is NULL");
			return NULL;
		}
		return &wvdev->wrap_dev_oma[0];
	}
	return NULL;
}

/**
 * dp_wrap_vdev_get_vma() - Get VMA address of PSTA
 *
 * @pdev: handle to the objmgr vdev.
 * Return: PSTA VMA address
 */
static u_int8_t *dp_wrap_vdev_get_vma(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_vdev *wvdev;

	if (vdev) {
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wvdev is NULL");
			return NULL;
		}
		return &wvdev->wrap_dev_vma[0];
	}
	return NULL;
}

/**
 * @brief Find vdev object based on MAC address.
 *
 * @param pdev Ptr to pdev obj mgr.
 * @param mac MAC address to look up.
 *
 * @return vdev objmgr on success
 * @return NULL on failure
 */

static inline struct wlan_objmgr_vdev *dp_wrap_vdev_vma_find(
	struct wlan_objmgr_pdev *pdev, unsigned char *mac)
{
	dp_vdev_wrap_t *wrap_vdev;
	int hash;
	rwlock_state_t lock_state;
	struct dp_wrap_pdev *wrap_pdev;
	struct dp_wrap_devt *wdt;

	if (!pdev)
		return NULL;
	wrap_pdev = dp_wrap_get_pdev_handle(pdev);
	if (!wrap_pdev)
		return NULL;
	wdt = &wrap_pdev->wp_devt;
	hash = WRAP_DEV_HASH(mac);
	OS_RWLOCK_READ_LOCK(&wdt->wdt_lock, &lock_state);
	LIST_FOREACH(wrap_vdev, &wdt->wdt_hash_vma[hash], wrap_dev_hash_vma) {
		if (qdf_is_macaddr_equal((struct qdf_mac_addr *)
		    wrap_vdev->wrap_dev_vma, (struct qdf_mac_addr *)mac)) {
			OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);
			return wrap_vdev->vdev;
		}
	}
	OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);

	return NULL;
}

int dp_wrap_attach(struct wlan_objmgr_pdev *pdev);
int dp_wrap_detach(struct wlan_objmgr_pdev *pdev);
void dp_wrap_vdev_attach(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_detach(struct wlan_objmgr_vdev *vdev);
int dp_wrap_dev_add(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_set_psta(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_set_mpsta(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_clear_psta(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_set_wrap(struct wlan_objmgr_vdev *vdev);
void dp_wrap_vdev_clear_wrap(struct wlan_objmgr_vdev *vdev);
void dp_wrap_dev_remove(struct wlan_objmgr_vdev *vdev);
void dp_wrap_dev_remove_vma(struct wlan_objmgr_vdev *vdev);
int dp_wrap_rx_process(struct net_device **dev, struct wlan_objmgr_vdev *vdev,
		       struct sk_buff *skb);
int dp_wrap_tx_process(struct net_device **dev, struct wlan_objmgr_vdev *vdev,
		       struct sk_buff **skb);
int dp_wrap_mat_tx(struct dp_wrap_vdev *wvdev, wbuf_t buf);
int dp_wrap_mat_rx(struct dp_wrap_vdev *wvdev, wbuf_t buf);
void dp_wrap_vdev_set_netdev(struct wlan_objmgr_vdev *vdev,
			  struct net_device *dev);
void dp_wrap_register_xmit_handler(struct wlan_objmgr_vdev *vdev,
				    void (*wlan_vdev_xmit_queue)
				    (struct net_device *dev, wbuf_t wbuf));
#endif
#endif
#endif

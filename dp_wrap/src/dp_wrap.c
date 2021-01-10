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

#if ATH_SUPPORT_WRAP
#if !WLAN_QWRAP_LEGACY
#include <qdf_nbuf.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#include "dp_wrap.h"

#include <wlan_utility.h>
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
#include "osif_nss_wifiol_if.h"
#include "osif_nss_wifiol_vdev_if.h"
#endif
extern void transcap_nwifi_to_8023(qdf_nbuf_t msdu);

/**
 * @brief Find wrap vdev object based on MAC address.
 *
 * @param wdt Ptr to the wrap device table.
 * @param mac MAC address to look up.
 *
 * @return wrap_vdev on success
 * @return NULL on failure
 */
dp_vdev_wrap_t *dp_wrap_wdev_find(struct dp_wrap_devt *wdt, unsigned char *mac)
{
	dp_vdev_wrap_t *wrap_vdev;
	int hash;
	rwlock_state_t lock_state;

	hash = WRAP_DEV_HASH(mac);
	OS_RWLOCK_READ_LOCK(&wdt->wdt_lock, &lock_state);
	LIST_FOREACH(wrap_vdev, &wdt->wdt_hash[hash], wrap_dev_hash) {
		if (qdf_is_macaddr_equal((struct qdf_mac_addr *)
			wrap_vdev->wrap_dev_oma, (struct qdf_mac_addr *)mac)) {
			OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);
			return wrap_vdev;
		}
	}
	OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);
	return NULL;
}

/**
 * @brief Find wrap vdev object based on MAC address.
 *
 * @param wdt Ptr to the wrap device table.
 * @param mac MAC address to look up.
 *
 * @return wrap_vdev on success
 * @return NULL on failure
 */

dp_vdev_wrap_t *dp_wrap_wdev_vma_find(struct dp_wrap_devt *wdt, unsigned char *mac)
{
	dp_vdev_wrap_t *wrap_vdev;
	int hash;
	rwlock_state_t lock_state;

	hash = WRAP_DEV_HASH(mac);
	OS_RWLOCK_READ_LOCK(&wdt->wdt_lock, &lock_state);
	LIST_FOREACH(wrap_vdev, &wdt->wdt_hash_vma[hash], wrap_dev_hash_vma) {
		if (qdf_is_macaddr_equal((struct qdf_mac_addr *)
			wrap_vdev->wrap_dev_vma, (struct qdf_mac_addr *)mac)) {
			OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);
			return wrap_vdev;
		}
	}
	OS_RWLOCK_READ_UNLOCK(&wdt->wdt_lock, &lock_state);
	return NULL;
}

/**
 * @brief Add wrap vdev object to the device table, also
 * registers bridge hooks if this the first object.
 *
 * @param vdev objmgr Pointer to wrap_vdev to add.
 *
 * @return 0 on success
 * @return -ve on failure
 */
int dp_wrap_dev_add(struct wlan_objmgr_vdev *vdev)
{
	int hash, hash_vma;
	struct dp_wrap_pdev *wrap_pdev;
	struct dp_wrap_vdev *wrap_vdev;
	struct dp_wrap_devt *wdt;
	rwlock_state_t lock_state;

	wrap_pdev = dp_wrap_get_pdev_handle(wlan_vdev_get_pdev(vdev));
	if (!wrap_pdev) {
		qwrap_err(" wrap_pdev is NULL");
		return -EINVAL;
	}
	wrap_vdev = dp_wrap_get_vdev_handle(vdev);
	if (!wrap_vdev) {
		qwrap_err(" wrap_vdev is NULL");
		return -EINVAL;
	}
	wdt = &wrap_pdev->wp_devt;
	hash = WRAP_DEV_HASH(wrap_vdev->wrap_dev_oma);
	hash_vma = WRAP_DEV_HASH(wrap_vdev->wrap_dev_vma);
	OS_RWLOCK_WRITE_LOCK_BH(&wdt->wdt_lock, &lock_state);
	LIST_INSERT_HEAD(&wdt->wdt_hash[hash], wrap_vdev, wrap_dev_hash);
	TAILQ_INSERT_TAIL(&wdt->wdt_dev, wrap_vdev, wrap_dev_list);
	LIST_INSERT_HEAD(&wdt->wdt_hash_vma[hash_vma], wrap_vdev,
			 wrap_dev_hash_vma);
	TAILQ_INSERT_TAIL(&wdt->wdt_dev_vma, wrap_vdev, wrap_dev_list_vma);
	OS_RWLOCK_WRITE_UNLOCK_BH(&wdt->wdt_lock, &lock_state);
	qwrap_info("Added vdev:%d to the list mat. pdev_id:%d",
		   vdev->vdev_objmgr.vdev_id,
		   wlan_objmgr_pdev_get_pdev_id(
		   wlan_vdev_get_pdev(vdev)));
	return 0;
}

/**
 * @brief Delete wrap dev object from the device table,
 * based on OMA address.
 *
 * @param vdev objmgr Pointer to wrap_vdev to delete.
 *
 * @return void
 */
void dp_wrap_dev_remove(struct wlan_objmgr_vdev *vdev)
{
	int hash;
	struct dp_wrap_vdev *wrap_vdev, *wvdev, *temp;
	struct dp_wrap_pdev *wrap_pdev;
	struct dp_wrap_devt *wdt;
	rwlock_state_t lock_state;

	wrap_pdev = dp_wrap_get_pdev_handle(wlan_vdev_get_pdev(vdev));
	if (!wrap_pdev) {
		qwrap_err(" wrap_pdev is NULL");
		return;
	}
	wrap_vdev = dp_wrap_get_vdev_handle(vdev);
	if (!wrap_vdev) {
		qwrap_err(" wrap_vdev is NULL");
		return;
	}
	wdt = &wrap_pdev->wp_devt;
	hash = WRAP_DEV_HASH(wrap_vdev->wrap_dev_oma);
	OS_RWLOCK_WRITE_LOCK_BH(&wdt->wdt_lock, &lock_state);
	LIST_FOREACH_SAFE(wvdev, &wdt->wdt_hash[hash], wrap_dev_hash, temp) {
		if ((wvdev == wrap_vdev) &&
		    qdf_is_macaddr_equal((struct qdf_mac_addr *)
			wvdev->wrap_dev_oma,
			(struct qdf_mac_addr *)wrap_vdev->wrap_dev_oma)) {
			LIST_REMOVE(wvdev, wrap_dev_hash);
			TAILQ_REMOVE(&wdt->wdt_dev, wvdev, wrap_dev_list);
			OS_RWLOCK_WRITE_UNLOCK_BH(&wdt->wdt_lock, &lock_state);
			qwrap_info("Removed vdev:%d from OMA list. pdev_id:%d",
				   vdev->vdev_objmgr.vdev_id,
				   wlan_objmgr_pdev_get_pdev_id(
				   wlan_vdev_get_pdev(vdev)));
			return;
		}
	}
	OS_RWLOCK_WRITE_UNLOCK_BH(&wdt->wdt_lock, &lock_state);
}

/**
 * @brief Delete wrap dev object from the device table,
 * based on VMA address.
 *
 * @param vdev objmgr Pointer to wrap_vdev to delete.
 *
 * @return void
 */
void dp_wrap_dev_remove_vma(struct wlan_objmgr_vdev *vdev)
{
	int hash;
	struct dp_wrap_vdev *wrap_vdev, *wvdev, *temp;
	struct dp_wrap_pdev *wrap_pdev;
	struct dp_wrap_devt *wdt;
	rwlock_state_t lock_state;

	wrap_pdev = dp_wrap_get_pdev_handle(wlan_vdev_get_pdev(vdev));
	if (!wrap_pdev) {
		qwrap_err(" wrap_pdev is NULL");
		return;
	}
	wrap_vdev = dp_wrap_get_vdev_handle(vdev);
	if (!wrap_vdev) {
		qwrap_err(" wrap_vdev is NULL");
		return;
	}
	wdt = &wrap_pdev->wp_devt;
	hash = WRAP_DEV_HASH(wrap_vdev->wrap_dev_vma);
	OS_RWLOCK_WRITE_LOCK_BH(&wdt->wdt_lock, &lock_state);
	LIST_FOREACH_SAFE(wvdev, &wdt->wdt_hash_vma[hash],
			  wrap_dev_hash_vma, temp) {
		if ((wvdev == wrap_vdev) &&
		    qdf_is_macaddr_equal((struct qdf_mac_addr *)
			wvdev->wrap_dev_vma,
			(struct qdf_mac_addr *)wrap_vdev->wrap_dev_vma)) {
			LIST_REMOVE(wvdev, wrap_dev_hash_vma);
			TAILQ_REMOVE(&wdt->wdt_dev_vma, wvdev,
				     wrap_dev_list_vma);
			OS_RWLOCK_WRITE_UNLOCK_BH(&wdt->wdt_lock, &lock_state);
			qwrap_info("Removed vdev:%d from vma list. pdev_id:%d",
				   vdev->vdev_objmgr.vdev_id,
				   wlan_objmgr_pdev_get_pdev_id(
				   wlan_vdev_get_pdev(vdev)));
			return;
		}
	}
	OS_RWLOCK_WRITE_UNLOCK_BH(&wdt->wdt_lock, &lock_state);
}

/**
 * @brief WRAP device table attach
 *
 * @param pdev objmgr Pointer.
 * @param wdt Ptr to wrap device table.
 *
 * @return void
 */
static void dp_wrap_devt_init(struct dp_wrap_pdev *wpdev, struct dp_wrap_devt *wdt)
{
	int i;

	OS_RWLOCK_INIT(&wdt->wdt_lock);
	TAILQ_INIT(&wdt->wdt_dev);
	TAILQ_INIT(&wdt->wdt_dev_vma);
	for (i = 0; i < WRAP_DEV_HASHSIZE; i++) {
		LIST_INIT(&wdt->wdt_hash[i]);
		LIST_INIT(&wdt->wdt_hash_vma[i]);
	}
	wdt->wrap_pdev = wpdev;
}

/**
 * @brief wrap device table detach
 *
 * @param pdev objmgr Pointer.
 *
 * @return
 */
static void dp_wrap_devt_detach(struct dp_wrap_pdev *wpdev)
{
	struct dp_wrap_devt *wdt = &wpdev->wp_devt;

	OS_RWLOCK_DESTROY(&wdt->wdt_lock);
	wdt->wrap_pdev = NULL;
}

/**
 * @brief wrap attach
 *
 * @param pdev objmgr Pointer.
 *
 * @return 0 on success
 * @return -ve on failure
 */
int dp_wrap_attach(struct wlan_objmgr_pdev *pdev)
{
	int ret = 0;
	struct dp_wrap_pdev *wrap_pdev = dp_wrap_get_pdev_handle(pdev);

	if (!wrap_pdev) {
		qwrap_err(" wrap_pdev is NULL");
		return -EINVAL;
	}

	OS_MEMZERO(wrap_pdev, sizeof(struct dp_wrap_pdev));
	wrap_pdev->wp_use_cnt++;
	dp_wrap_devt_init(wrap_pdev, &wrap_pdev->wp_devt);
	wrap_pdev->wp_isolation = WRAP_ISOLATION_DEFVAL;
	qwrap_info("Wrap Attached: Wrap_pdev =%pK &wrap_pdev->wp_devt=%pK",
		   wrap_pdev, &wrap_pdev->wp_devt);
	return ret;
}
qdf_export_symbol(dp_wrap_attach);

/**
 * @brief wrap vdev attach
 *
 * @param vdev objmgr Pointer.
 *
 * @return void
 */
void dp_wrap_vdev_attach(struct wlan_objmgr_vdev *vdev)
{
	if (dp_wrap_vdev_is_psta(vdev)) {
		dp_wrap_vdev_set_psta(vdev);
		if (dp_wrap_vdev_is_mpsta(vdev))
			dp_wrap_vdev_set_mpsta(vdev);
	}
	if (dp_wrap_vdev_is_wrap(vdev))
		dp_wrap_vdev_set_wrap(vdev);
}

/**
 * @brief wrap vdev detach
 *
 * @param vdev objmgr Pointer.
 *
 * @return void
 */
void dp_wrap_vdev_detach(struct wlan_objmgr_vdev *vdev)
{
	if (dp_wrap_vdev_is_psta(vdev))
		dp_wrap_vdev_clear_psta(vdev);
	if (dp_wrap_vdev_is_wrap(vdev))
		dp_wrap_vdev_clear_wrap(vdev);
}

/**
 * @brief wrap detach
 *
 * @param pdev objmgr Pointer.
 *
 * @return 0 on success
 * @return -ve on failure
 */
int dp_wrap_detach(struct wlan_objmgr_pdev *pdev)
{
	int ret = 0;
	struct dp_wrap_pdev *wrap_pdev = dp_wrap_get_pdev_handle(pdev);

	if (!wrap_pdev) {
		qwrap_err(" wrap_pdev is NULL");
		return -EINVAL;
	}
	wrap_pdev->wp_use_cnt--;
	if (wrap_pdev->wp_use_cnt == 0) {
		dp_wrap_devt_detach(wrap_pdev);
		qwrap_info("qca wrap detached\n");
	}
	return ret;
}
qdf_export_symbol(dp_wrap_detach);

/**
 * dp_wrap_vdev_set_psta() - Set psta flag
 *
 * @vdev: handle to the objmgr vdev.
 * Return: void
 */
void dp_wrap_vdev_set_psta(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_vdev *wvdev;
	struct dp_wrap_pdev *wpdev;
	struct wlan_objmgr_pdev *pdev;

	if (vdev) {
		pdev = vdev->vdev_objmgr.wlan_pdev;
		if (!pdev) {
			qwrap_err(" pdev is NULL");
			return;
		}
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wrap_pdev is NULL");
			return;
		}
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wrap_vdev is NULL");
			return;
		}
		OS_MEMZERO(wvdev, sizeof(struct dp_wrap_vdev));
		wvdev->wrap_pdev = wpdev;
		wvdev->is_psta = 1;
		wvdev->vdev = vdev;
		wpdev->npstavaps++;
		if (dp_wrap_vdev_is_wired_psta(vdev))
			wvdev->is_wired_psta = 1;
		if (dp_wrap_vdev_is_mat_set(vdev)) {
			wvdev->mat_enabled = 1;
			WLAN_ADDR_COPY(wvdev->wrap_dev_oma,
				       vdev->vdev_mlme.mataddr);
		} else {
			WLAN_ADDR_COPY(wvdev->wrap_dev_oma,
				       vdev->vdev_mlme.macaddr);
		}
		WLAN_ADDR_COPY(wvdev->wrap_dev_vma, vdev->vdev_mlme.macaddr);
		qwrap_info("set PSTA flags for vdev_id:%d pdev_id:%d OMA addr:"
				QDF_MAC_ADDR_FMT "Vma addr:" QDF_MAC_ADDR_FMT,
				vdev->vdev_objmgr.vdev_id,
				wlan_objmgr_pdev_get_pdev_id(pdev),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_oma),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_vma));
	}
}

/**
 * dp_wrap_vdev_clear_psta() - clear psta flag
 *
 * @vdev: handle to the objmgr vdev.
 * Return: void
 */
void dp_wrap_vdev_clear_psta(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_pdev *wpdev;
	struct dp_wrap_vdev *wvdev;

	if (vdev) {
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wrap_vdev is NULL");
			return;
		}
		wpdev = wvdev->wrap_pdev;
		if (!wpdev) {
			qwrap_err(" wrap_pdev is NULL");
			return;
		}
		wpdev->npstavaps--;
		if (wvdev->is_mpsta) {
			wpdev->mpsta_vdev = NULL;
			wpdev->mpsta_dev = NULL;
		}
		qwrap_info("clear PSTA flag vdev_id:%d pdev_id:%d mpsta:%d",
			   vdev->vdev_objmgr.vdev_id,
			   wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev
			   (vdev)), wvdev->is_mpsta);
	}
}

/**
 * dp_wrap_vdev_set_mpsta() - Set mpsta flag
 *
 * @vdev: handle to the objmgr vdev.
 * Return: void
 */
void dp_wrap_vdev_set_mpsta(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_vdev *wvdev;
	struct dp_wrap_pdev *wpdev;
	struct wlan_objmgr_pdev *pdev;

	if (vdev) {
		pdev = vdev->vdev_objmgr.wlan_pdev;
		if (!pdev) {
			qwrap_err(" pdev is NULL");
			return;
		}
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wrap_pdev is NULL");
			return;
		}
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wrap_vdev is NULL");
			return;
		}
		wvdev->is_mpsta = 1;
		wpdev->mpsta_vdev = vdev;
		qwrap_info("set MPSTA flags vdev_id:%d pdev_id:%d OMA addr:"
				QDF_MAC_ADDR_FMT "Vma addr:" QDF_MAC_ADDR_FMT,
				vdev->vdev_objmgr.vdev_id,
				wlan_objmgr_pdev_get_pdev_id(pdev),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_oma),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_vma));
	}
}

/**
 * dp_wrap_vdev_set_wrap() - set wrap flag
 *
 * @vdev: handle to the objmgr vdev.
 * Return: void
 */
void dp_wrap_vdev_set_wrap(struct wlan_objmgr_vdev *vdev)
{
	struct dp_wrap_vdev *wvdev;
	struct dp_wrap_pdev *wpdev;
	struct wlan_objmgr_pdev *pdev;

	if (vdev) {
		pdev = vdev->vdev_objmgr.wlan_pdev;
		if (!pdev) {
			qwrap_err(" pdev is NULL");
			return;
		}
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wrap_pdev is NULL");
			return;
		}
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err(" wrap_vdev is NULL");
			return;
		}
		OS_MEMZERO(wvdev, sizeof(struct dp_wrap_vdev));
		wvdev->wrap_pdev = wpdev;
		wvdev->is_wrap = 1;
		wvdev->vdev = vdev;
		wpdev->wrap_vdev = vdev;
		wpdev->nwrapvaps++;
		qwrap_info("set WRAP flags for vdev_id:%d pdev_id:%d OMA addr:"
				QDF_MAC_ADDR_FMT "Vma addr:" QDF_MAC_ADDR_FMT,
				vdev->vdev_objmgr.vdev_id,
				wlan_objmgr_pdev_get_pdev_id(pdev),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_oma),
				QDF_MAC_ADDR_REF(wvdev->wrap_dev_vma));
	}
}

/**
 * dp_wrap_vdev_clear_wrap() - clear wrap flag
 *
 * @vdev: handle to the objmgr vdev.
 * Return: void
 */
void dp_wrap_vdev_clear_wrap(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct dp_wrap_pdev *wpdev;

	if (vdev) {
		pdev = vdev->vdev_objmgr.wlan_pdev;
		if (!pdev) {
			qwrap_err(" pdev is NULL");
			return;
		}
		wpdev = dp_wrap_get_pdev_handle(pdev);
		if (!wpdev) {
			qwrap_err(" wpdev is NULL");
			return;
		}
		wpdev->wrap_vdev = NULL;
		wpdev->nwrapvaps--;
		qwrap_info("clear WRAP flags for vdev_id:%d pdev_id:%d",
			   vdev->vdev_objmgr.vdev_id,
			   wlan_objmgr_pdev_get_pdev_id(pdev));
	}
}

static inline int
dp_wrap_tx_bridge(struct wlan_objmgr_vdev *vdev, struct dp_wrap_vdev **wvdev,
		  struct sk_buff **skb) {
	/* Assuming native wifi or raw mode is not
	 * enabled in beeliner, to be revisted later
	 */
	struct ether_header *eh = (struct ether_header *)((*skb)->data);
	struct dp_wrap_pdev *wpdev;
	struct dp_wrap_vdev *wrap_vdev;

	/* Mpsta vap here, find the correct tx vap from the wrap
	 * common based on src address
	 */
	wpdev = dp_wrap_get_pdev_handle(wlan_vdev_get_pdev(vdev));
	if (qdf_unlikely(!wpdev))
		return 1;
	wrap_vdev = dp_wrap_wdev_find(&wpdev->wp_devt, eh->ether_shost);
	if (qdf_unlikely(!wrap_vdev))
		return 1;
	*wvdev = wrap_vdev;
	if (wrap_vdev) {
		if (qdf_unlikely((IEEE80211_IS_MULTICAST(eh->ether_dhost) ||
				  IEEE80211_IS_BROADCAST(eh->ether_dhost)))) {
			*skb = qdf_nbuf_unshare(*skb);
		}
		return 0;
	} else {
		/* When proxysta is not created, drop the packet. Donot send
		 * packet on mainproxysta. Return 1 here to drop the packet
		 * when psta is not yet created.
		 */
		qwrap_err("Drop pkt, PSTA is not created src mac:"
			  QDF_MAC_ADDR_FMT "vdev_id:%d", QDF_MAC_ADDR_REF
			  (eh->ether_shost), vdev->vdev_objmgr.vdev_id);
		return 1;
	}
}

int dp_wrap_tx_process(struct net_device **dev, struct wlan_objmgr_vdev *vdev,
		       struct sk_buff **skb)
{
	struct dp_wrap_vdev *wvdev = NULL;
	struct ether_header *eh;

	if (qdf_unlikely(dp_wrap_vdev_is_mpsta(vdev))) {
		if (dp_wrap_tx_bridge(vdev, &wvdev, skb))
			return 1;
		if (*(skb) == NULL) {
			qwrap_err("Drop pkt, SKB is null dev_id:%d",
				  vdev->vdev_objmgr.vdev_id);
			return 1;
		}
		eh = (struct ether_header *)((*skb)->data);
		if (qdf_likely(dp_wrap_vdev_is_psta(wvdev->vdev))) {
			if (dp_wrap_mat_tx(wvdev, (wbuf_t)*skb)) {
				qwrap_err("Drop pkt,MAT error:"QDF_MAC_ADDR_FMT
					   "vdev_id:%d", QDF_MAC_ADDR_REF(
					   eh->ether_shost),
					   vdev->vdev_objmgr.vdev_id);
				return 1;
			}
		}
		*dev = wvdev->dev;
		vdev = wvdev->vdev;
	}
	if (wlan_vdev_is_up(vdev) != QDF_STATUS_SUCCESS) {
		qwrap_err("Drop pkt, vdev is not up:"QDF_MAC_ADDR_FMT
			  "vdevid:%d", QDF_MAC_ADDR_REF(eh->ether_shost),
			  vdev->vdev_objmgr.vdev_id);
		return 1;
	}
	return 0;
}
qdf_export_symbol(dp_wrap_tx_process);

static inline
int dp_wrap_rx_bridge(struct wlan_objmgr_vdev *vdev, struct net_device **dev,
		      struct dp_wrap_vdev *wvdev, struct sk_buff *skb)
{
	/* Assuming native wifi or raw mode is not
	 * enabled in beeliner, to be revisted later
	 */
	struct ether_header *eh = (struct ether_header *)skb->data;
	struct dp_wrap_pdev *wpdev = wvdev->wrap_pdev;
	int isolation = wpdev->wp_isolation;
	int ret = 0;
	struct dp_wrap_vdev *mpsta_wvdev, *wrap_wvdev, *t_wvdev;
	struct wlan_objmgr_vdev *mpsta_vdev, *wrap_vdev;

	mpsta_vdev = wpdev->mpsta_vdev;
	wrap_vdev = wpdev->wrap_vdev;
	if (isolation == 0) {
		/* Isolatio mode,Wired and wireless clients connected
		 * to Qwrap Ap can talk through Qwrap bridge
		 */
		if ((wvdev->is_mpsta == 0) && (wvdev->is_psta) &&
		    (mpsta_vdev)) {
			if (qdf_likely(
				!(eh->ether_type == htons(ETHERTYPE_PAE)))) {
				/*get mpsta vap , for qwrap bridge learning
				 * is always through main proxy sta
				 */
				skb->dev = wpdev->mpsta_dev;
				*dev = skb->dev;
			}
			ret = 0;
		}
	} else {
		/* isolation mode enabled. Wired and wireless client
		 * connected to Qwrap AP can talk through root AP
		 */
		if (wvdev->is_psta && !wvdev->is_mpsta &&
		    wvdev->is_wired_psta) {
			/* Packets recevied through wired psta vap */
			if (qdf_likely(!(eh->ether_type == htons(ETHERTYPE_PAE)
					)) && (mpsta_vdev)) {
				/* rx packets from wired psta should go through
				 * bridge.Here qwrap bridge learning for wired
				 * proxy clients is always through mpsta
				 */
				skb->dev = wpdev->mpsta_dev;
				*dev = skb->dev;
				ret = 0;
			}
		} else if (wvdev->is_psta && !wvdev->is_mpsta &&
			  !wvdev->is_wired_psta &&
			  !(eh->ether_type == htons(ETHERTYPE_PAE)) &&
			  (wrap_vdev)) {
			/* rx unicast from wireless proxy, client
			 * should always xmit through wrap AP vap
			 */
			wrap_wvdev = dp_wrap_get_vdev_handle(wrap_vdev);
			wrap_wvdev->wlan_vdev_xmit_queue(wrap_wvdev->dev, skb);
			ret = 1;
		} else if ((wvdev->is_wrap &&
			   !(eh->ether_type == htons(ETHERTYPE_PAE))) &&
			   (mpsta_vdev)) {
			/* rx from wrap AP , since wrap not connected to
			 * in isolation , should always xmit through
			 * main proxy vap
			 */
			mpsta_wvdev = dp_wrap_get_vdev_handle(mpsta_vdev);
			mpsta_wvdev->wlan_vdev_xmit_queue(
							mpsta_wvdev->dev, skb);
			ret = 1;
		} else if (wvdev->is_mpsta &&
			   IEEE80211_IS_MULTICAST(eh->ether_dhost) &&
			   (mpsta_vdev) && (wrap_vdev)) {
			/*check oma or vma, for MPSTA both are same*/
			mpsta_wvdev = dp_wrap_get_vdev_handle(mpsta_vdev);
			if ((OS_MEMCMP(mpsta_wvdev->wrap_dev_oma,
				       eh->ether_shost, 6) == 0))  {
				/* Multicast orginated from mpsta/bridge
				 * should always xmit through wrap AP vap
				 */
				wrap_wvdev = dp_wrap_get_vdev_handle(
						wrap_vdev);
				wrap_wvdev->wlan_vdev_xmit_queue(
						wrap_wvdev->dev, skb);
				ret = 1;
			} else {
				t_wvdev = dp_wrap_wdev_vma_find(
						&wpdev->wp_devt,
						eh->ether_shost);
				if (t_wvdev) {
					if (t_wvdev->is_wired_psta) {
						/*Multicast received from wired
						 *clients , forward to wrap AP
						 */
						wrap_wvdev =
						dp_wrap_get_vdev_handle(
								wrap_vdev);
						wrap_wvdev->
							wlan_vdev_xmit_queue
							(wrap_wvdev->dev, skb);
						ret = 1;
					} else {
						/*Multicast received from
						 *wireless client,fwd to bridge
						 */
						skb->dev = wpdev->mpsta_dev;
						*dev = skb->dev;
						ret = 0;
					}
				} else  {
					qdf_nbuf_t copy;

					copy = qdf_nbuf_copy(skb);
					/*Multicast received from client behind
					 *root side forward to both wrap and
					 *bridge side
					 */
					if (copy) {
						wrap_wvdev =
						dp_wrap_get_vdev_handle(
								wrap_vdev);
						wrap_wvdev->
						wlan_vdev_xmit_queue(
							wrap_wvdev->dev, copy);
					} else {
						qdf_err("Wrap buf cpy fail");
					}
					skb->dev = wpdev->mpsta_dev;
					*dev = skb->dev;
					ret = 0;
				}
			}
		}
	}
	return ret;
}

int dp_wrap_rx_process(struct net_device **dev, struct wlan_objmgr_vdev *vdev,
		       struct sk_buff *skb, int *nwifi)
{
	struct dp_wrap_vdev *wvdev;
	int rv = 0;

	wvdev = dp_wrap_get_vdev_handle(vdev);

	if (qdf_unlikely(dp_wrap_vdev_is_psta(vdev) || dp_wrap_vdev_is_wrap(vdev))) {
		if (*nwifi) {
			transcap_nwifi_to_8023(skb);
			*nwifi = 0;
		}
		dp_wrap_mat_rx(wvdev, (wbuf_t)skb);
		rv = dp_wrap_rx_bridge(vdev, dev, wvdev, skb);
	}
	return rv;
}
qdf_export_symbol(dp_wrap_rx_process);

void
dp_wrap_vdev_set_netdev(struct wlan_objmgr_vdev *vdev, struct net_device *dev)
{
	struct dp_wrap_vdev *wvdev;
	struct dp_wrap_pdev *wpdev;
	struct wlan_objmgr_pdev *pdev;

	if (vdev) {
		wvdev = dp_wrap_get_vdev_handle(vdev);
		if (!wvdev) {
			qwrap_err("wrap_vdev is NULL");
			return;
		}
		wvdev->dev = dev;
		if (dp_wrap_vdev_is_mpsta(vdev)) {
			pdev = vdev->vdev_objmgr.wlan_pdev;
			if (!pdev) {
				qwrap_err("pdev is NULL");
				return;
			}
			wpdev = dp_wrap_get_pdev_handle(pdev);
			if (!wpdev) {
				qwrap_err(" wpdev is NULL");
				return;
			}
			wpdev->mpsta_dev = dev;
		}
	}
}

/**
 * dp_wrap_register_xmit_handler() - Register xmit handler in wrap_vdev object
 *
 * @vdev: handle to the objmgr vdev
 * @func: xmit handler function to register
 * Return: void
 */
void dp_wrap_register_xmit_handler(struct wlan_objmgr_vdev *vdev,
				    void (*wlan_vdev_xmit_queue)
				    (struct net_device *dev, wbuf_t wbuf))
{
	struct dp_wrap_vdev *wrap_vdev;

	wrap_vdev = dp_wrap_get_vdev_handle(vdev);

	if (wrap_vdev)
		wrap_vdev->wlan_vdev_xmit_queue = wlan_vdev_xmit_queue;
	else
		qwrap_err("Error in registering xmit queue");
}
#endif
#endif

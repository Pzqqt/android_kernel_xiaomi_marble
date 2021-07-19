/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

#include <cfr_defs_i.h>
#include <qdf_types.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_cfr_tgt_api.h>
#include <qdf_streamfs.h>
#include <target_if.h>
#include <target_if_direct_buf_rx_api.h>
#include <wlan_osif_priv.h>
#include <cfg_ucfg_api.h>
#include "cfr_cfg.h"
#ifdef WLAN_CFR_PM
#include "host_diag_core_event.h"
#endif

/**
 * wlan_cfr_is_ini_disabled() - Check if cfr feature is disabled
 * @pdev - the physical device object.
 *
 * Return : true if cfr is disabled, else false.
 */
static bool
wlan_cfr_is_ini_disabled(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	uint8_t cfr_disable_bitmap;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		cfr_err("psoc is null");
		return true;
	}

	cfr_disable_bitmap = cfg_get(psoc, CFG_CFR_DISABLE);

	if (cfr_disable_bitmap & (1 << wlan_objmgr_pdev_get_pdev_id(pdev))) {
		cfr_info("cfr is disabled for pdev[%d]",
			 wlan_objmgr_pdev_get_pdev_id(pdev));
		return true;
	}

	return false;
}

/**
 * wlan_cfr_get_dbr_num_entries() - Get entry number of DBR ring
 * @pdev - the physical device object.
 *
 * Return : Entry number of DBR ring.
 */
static uint32_t
wlan_cfr_get_dbr_num_entries(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_dbr_ring_caps *dbr_ring_cap;
	uint8_t num_dbr_ring_caps, cap_idx, pdev_id;
	struct target_psoc_info *tgt_psoc_info;
	uint32_t num_entries = MAX_LUT_ENTRIES;

	if (!pdev) {
		cfr_err("Invalid pdev");
		return num_entries;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		cfr_err("psoc is null");
		return num_entries;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		cfr_err("target_psoc_info is null");
		return num_entries;
	}

	num_dbr_ring_caps = target_psoc_get_num_dbr_ring_caps(tgt_psoc_info);
	dbr_ring_cap = target_psoc_get_dbr_ring_caps(tgt_psoc_info);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	for (cap_idx = 0; cap_idx < num_dbr_ring_caps; cap_idx++) {
		if (dbr_ring_cap[cap_idx].pdev_id == pdev_id &&
		    dbr_ring_cap[cap_idx].mod_id == DBR_MODULE_CFR)
			num_entries = dbr_ring_cap[cap_idx].ring_elems_min;
	}

	num_entries = QDF_MIN(num_entries, MAX_LUT_ENTRIES);
	cfr_debug("pdev id %d, num_entries %d", pdev_id, num_entries);

	return num_entries;
}

#ifdef WLAN_CFR_PM
/**
 * cfr_wakelock_init(): Create/init wake lock for CFR
 *
 * Create/init wake lock for CFR
 *
 * Return None
 */
static void cfr_wakelock_init(struct pdev_cfr *pcfr)
{
	if (!pcfr) {
		cfr_debug("NULL pa");
		return;
	}

	pcfr->is_prevent_suspend = false;
	qdf_wake_lock_create(&pcfr->wake_lock, "wlan_cfr");
	qdf_runtime_lock_init(&pcfr->runtime_lock);
}

/**
 * cfr_wakelock_deinit(): Destroy/deinit wake lock for CFR
 *
 * Destroy/deinit wake lock for CFR
 *
 * Return None
 */
static void cfr_wakelock_deinit(struct pdev_cfr *pcfr)
{
	if (!pcfr) {
		cfr_debug("NULL pa");
		return;
	}

	qdf_runtime_lock_deinit(&pcfr->runtime_lock);
	qdf_wake_lock_destroy(&pcfr->wake_lock);
}
#else
static inline void cfr_wakelock_init(struct pdev_cfr *pcfr)
{
}

static inline void cfr_wakelock_deinit(struct pdev_cfr *pcfr)
{
}
#endif

QDF_STATUS
wlan_cfr_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct psoc_cfr *cfr_sc = NULL;

	cfr_sc = (struct psoc_cfr *)qdf_mem_malloc(sizeof(struct psoc_cfr));
	if (!cfr_sc) {
		cfr_err("Failed to allocate cfr_ctx object\n");
		return QDF_STATUS_E_NOMEM;
	}

	cfr_sc->psoc_obj = psoc;

	wlan_objmgr_psoc_component_obj_attach(psoc, WLAN_UMAC_COMP_CFR,
					      (void *)cfr_sc,
					      QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct psoc_cfr *cfr_sc = NULL;

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);
	if (cfr_sc) {
		wlan_objmgr_psoc_component_obj_detach(psoc, WLAN_UMAC_COMP_CFR,
						      (void *)cfr_sc);
		qdf_mem_free(cfr_sc);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cfr *pa = NULL;
	uint32_t idx;

	if (!pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_ini_disabled(pdev)) {
		wlan_pdev_nif_feat_ext_cap_clear(pdev, WLAN_PDEV_FEXT_CFR_EN);
		return QDF_STATUS_E_NOSUPPORT;
	}

	wlan_pdev_nif_feat_ext_cap_set(pdev, WLAN_PDEV_FEXT_CFR_EN);

	pa = (struct pdev_cfr *)qdf_mem_malloc(sizeof(struct pdev_cfr));
	if (!pa) {
		cfr_err("Failed to allocate pdev_cfr object\n");
		return QDF_STATUS_E_NOMEM;
	}
	pa->pdev_obj = pdev;
	pa->lut_num = wlan_cfr_get_dbr_num_entries(pdev);
	if (!pa->lut_num) {
		cfr_err("lut num is 0");
		qdf_mem_free(pa);
		return QDF_STATUS_E_INVAL;
	}
	pa->lut = (struct look_up_table **)qdf_mem_malloc(pa->lut_num *
			sizeof(struct look_up_table *));
	if (!pa->lut) {
		cfr_err("Failed to allocate lut, lut num %d", pa->lut_num);
		qdf_mem_free(pa);
		return QDF_STATUS_E_NOMEM;
	}
	for (idx = 0; idx < pa->lut_num; idx++)
		pa->lut[idx] = (struct look_up_table *)qdf_mem_malloc(
			sizeof(struct look_up_table));

	cfr_wakelock_init(pa);
	wlan_objmgr_pdev_component_obj_attach(pdev, WLAN_UMAC_COMP_CFR,
					      (void *)pa, QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cfr *pa = NULL;
	uint32_t idx;

	if (!pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_info("cfr is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa) {
		cfr_wakelock_deinit(pa);
		wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_CFR,
						      (void *)pa);
		if (pa->lut) {
			for (idx = 0; idx < pa->lut_num; idx++)
				qdf_mem_free(pa->lut[idx]);
			qdf_mem_free(pa->lut);
		}
		qdf_mem_free(pa);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cfr *pe = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev = NULL;

	if (!peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_peer_get_vdev(peer);
	if (vdev)
		pdev = wlan_vdev_get_pdev(vdev);

	if (!pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_info("cfr is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	pe = (struct peer_cfr *)qdf_mem_malloc(sizeof(struct peer_cfr));
	if (!pe) {
		cfr_err("Failed to allocate peer_cfr object\n");
		return QDF_STATUS_E_FAILURE;
	}

	pe->peer_obj = peer;

	/* Remaining will be populated when we give CFR capture command */
	wlan_objmgr_peer_component_obj_attach(peer, WLAN_UMAC_COMP_CFR,
					      (void *)pe, QDF_STATUS_SUCCESS);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cfr *pe = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct pdev_cfr *pa = NULL;

	if (!peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_peer_get_vdev(peer);
	if (vdev)
		pdev = wlan_vdev_get_pdev(vdev);

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_info("cfr is disabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (pdev)
		pa = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							   WLAN_UMAC_COMP_CFR);

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);

	if (pa && pe) {
		if (pe->period && pe->request)
			pa->cfr_current_sta_count--;
	}

	if (pe) {
		wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_CFR,
						      (void *)pe);
		qdf_mem_free(pe);
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef CFR_USE_FIXED_FOLDER
static char *cfr_get_dev_name(struct wlan_objmgr_pdev *pdev)
{
	char *default_name = "wlan";

	return default_name;
}
#else
/**
 * cfr_get_dev_name() - Get net device name from pdev
 *  @pdev: objmgr pdev
 *
 *  Return: netdev name
 */
static char *cfr_get_dev_name(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_osif_priv *pdev_ospriv;
	struct qdf_net_if *nif;

	pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	if (!pdev_ospriv) {
		cfr_err("pdev_ospriv is NULL\n");
		return NULL;
	}

	nif = pdev_ospriv->nif;
	if (!nif) {
		cfr_err("pdev nif is NULL\n");
		return NULL;
	}

	return  qdf_net_if_get_devname(nif);
}
#endif

QDF_STATUS cfr_streamfs_init(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa = NULL;
	char *devname;
	char folder[32];

	if (!pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_info("cfr is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);

	if (pa == NULL) {
		cfr_err("pdev_cfr is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pa->is_cfr_capable) {
		cfr_err("CFR IS NOT SUPPORTED\n");
		return QDF_STATUS_E_FAILURE;
	}

	devname = cfr_get_dev_name(pdev);
	if (!devname) {
		cfr_err("devname is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	snprintf(folder, sizeof(folder), "cfr%s", devname);

	pa->dir_ptr = qdf_streamfs_create_dir((const char *)folder, NULL);

	if (!pa->dir_ptr) {
		cfr_err("Directory create failed");
		return QDF_STATUS_E_FAILURE;
	}

	pa->chan_ptr = qdf_streamfs_open("cfr_dump", pa->dir_ptr,
					 pa->subbuf_size,
					 pa->num_subbufs, NULL);

	if (!pa->chan_ptr) {
		cfr_err("Chan create failed");
		qdf_streamfs_remove_dir_recursive(pa->dir_ptr);
		pa->dir_ptr = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_remove(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa = NULL;

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_info("cfr is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa) {
		if (pa->chan_ptr) {
			qdf_streamfs_close(pa->chan_ptr);
			pa->chan_ptr = NULL;
		}

		if (pa->dir_ptr) {
			qdf_streamfs_remove_dir_recursive(pa->dir_ptr);
			pa->dir_ptr = NULL;
		}

	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_write(struct pdev_cfr *pa, const void *write_data,
			      size_t write_len)
{
	if (pa->chan_ptr) {
	/* write to channel buffer */
		qdf_streamfs_write(pa->chan_ptr, (const void *)write_data,
				   write_len);
	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_streamfs_flush(struct pdev_cfr *pa)
{
	if (pa->chan_ptr) {

	/* Flush the data write to channel buffer */
		qdf_streamfs_flush(pa->chan_ptr);
	} else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_stop_indication(struct wlan_objmgr_vdev *vdev)
{
	struct pdev_cfr *pa;
	uint32_t status;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (!pa) {
		cfr_err("pdev_cfr is NULL\n");
		return QDF_STATUS_E_INVAL;
	}

	if (pa->nl_cb.cfr_nl_cb) {
		pa->nl_cb.cfr_nl_cb(pa->nl_cb.vdev_id, pa->nl_cb.pid,
				    (const void *)CFR_STOP_STR,
				    sizeof(CFR_STOP_STR));

		return QDF_STATUS_SUCCESS;
	}

	status = cfr_streamfs_write(pa, (const void *)CFR_STOP_STR,
				    sizeof(CFR_STOP_STR));

	status = cfr_streamfs_flush(pa);
	cfr_debug("stop indication done");

	return status;
}

#ifdef WLAN_CFR_PM
QDF_STATUS cfr_prevent_suspend(struct pdev_cfr *pcfr)
{
	if (!pcfr) {
		cfr_debug("NULL pcfr");
		return QDF_STATUS_E_INVAL;
	}

	if (pcfr->is_prevent_suspend) {
		cfr_debug("acquired wake lock");
		return QDF_STATUS_E_AGAIN;
	}
	qdf_wake_lock_acquire(&pcfr->wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_CFR);
	qdf_runtime_pm_prevent_suspend(&pcfr->runtime_lock);
	pcfr->is_prevent_suspend = true;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfr_allow_suspend(struct pdev_cfr *pcfr)
{
	if (!pcfr) {
		cfr_debug("NULL pcfr");
		return QDF_STATUS_E_INVAL;
	}

	if (!pcfr->is_prevent_suspend) {
		cfr_debug("wake lock not acquired");
		return QDF_STATUS_E_INVAL;
	}
	qdf_wake_lock_release(&pcfr->wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_CFR);
	qdf_runtime_pm_allow_suspend(&pcfr->runtime_lock);
	pcfr->is_prevent_suspend = false;

	return QDF_STATUS_SUCCESS;
}
#endif

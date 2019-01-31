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

#include <cfr_defs_i.h>
#include <qdf_types.h>
#include <osif_private.h>
#include <wlan_osif_priv.h>

QDF_STATUS
wlan_cfr_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	/* CFR is per pdev; psoc doesn't contain any private object for now */
	struct psoc_cfr *cfr_sc = NULL;

	cfr_sc = (struct psoc_cfr *)qdf_mem_malloc(sizeof(struct psoc_cfr));
	if (NULL == cfr_sc) {
		cfr_err("Failed to allocate cfr_ctx object\n");
		return QDF_STATUS_E_NOMEM;
	}

	cfr_sc->psoc_obj = psoc;
	cfr_sc->is_cfr_capable = 1;

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
	if (NULL != cfr_sc) {
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

	if (NULL == pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pa = (struct pdev_cfr *)qdf_mem_malloc(sizeof(struct pdev_cfr));
	if (NULL == pa) {
		cfr_err("Failed to allocate pdev_cfr object\n");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(pa, sizeof(struct pdev_cfr));
	pa->pdev_obj = pdev;
	pa->is_cfr_capable = 1; /* derive for WMI Service ready capable */
	pa->cfr_timer_enable = 0;

	wlan_objmgr_pdev_component_obj_attach(pdev, WLAN_UMAC_COMP_CFR,
					      (void *)pa, QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_cfr *pa = NULL;

	if (NULL == pdev) {
		cfr_err("PDEV is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (NULL != pa) {
		wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_CFR,
						      (void *)pa);
		qdf_mem_free(pa);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cfr_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	struct peer_cfr *pe = NULL;

	if (NULL == peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pe = (struct peer_cfr *)qdf_mem_malloc(sizeof(struct peer_cfr));
	if (NULL == pe) {
		cfr_err("Failed to allocate peer_cfr object\n");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(pe, sizeof(struct peer_cfr));
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

	if (NULL == peer) {
		cfr_err("PEER is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (NULL != pe) {
		wlan_objmgr_peer_component_obj_detach(peer, WLAN_UMAC_COMP_CFR,
						      (void *)pe);
		qdf_mem_free(pe);
	}

	return QDF_STATUS_SUCCESS;
}

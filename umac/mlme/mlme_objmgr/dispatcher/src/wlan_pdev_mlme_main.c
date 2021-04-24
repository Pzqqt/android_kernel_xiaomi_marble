/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Implements PDEV MLME APIs
 */

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_mlme_cmn.h"
#include "include/wlan_pdev_mlme.h"
#include "wlan_pdev_mlme_main.h"
#include "wlan_pdev_mlme_api.h"
#include <wlan_utility.h>

static QDF_STATUS mlme_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev,
					       void *arg)
{
	struct pdev_mlme_obj *pdev_mlme;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev) {
		mlme_err(" PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("PSOC is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_mlme = qdf_mem_malloc(sizeof(*pdev_mlme));
	if (!pdev_mlme) {
		mlme_err(" MLME component object alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	wlan_minidump_log(pdev_mlme, sizeof(*pdev_mlme), psoc,
			  WLAN_MD_OBJMGR_PDEV_MLME, "pdev_mlme");
	pdev_mlme->pdev = pdev;

	if (mlme_pdev_ops_ext_hdl_create(pdev_mlme) != QDF_STATUS_SUCCESS)
		goto init_failed;

	wlan_objmgr_pdev_component_obj_attach(pdev, WLAN_UMAC_COMP_MLME,
					      (void *)pdev_mlme,
					      QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;

init_failed:
	wlan_minidump_remove(pdev_mlme, sizeof(*pdev_mlme), psoc,
			     WLAN_MD_OBJMGR_PDEV_MLME, "pdev_mlme");

	qdf_mem_free(pdev_mlme);

	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS mlme_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev,
						void *arg)
{
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_info(" PDEV MLME component object is NULL");
		return QDF_STATUS_SUCCESS;
	}

	mlme_pdev_ops_ext_hdl_destroy(pdev_mlme);
	wlan_objmgr_pdev_component_obj_detach(pdev, WLAN_UMAC_COMP_MLME,
					      (void *)pdev_mlme);

	wlan_minidump_remove(pdev_mlme, sizeof(*pdev_mlme),
			     wlan_pdev_get_psoc(pdev),
			     WLAN_MD_OBJMGR_PDEV_MLME, "pdev_mlme");

	qdf_mem_free(pdev_mlme);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_pdev_mlme_init(void)
{
	if (wlan_objmgr_register_pdev_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_pdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_register_pdev_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_pdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS) {
		if (wlan_objmgr_unregister_pdev_create_handler
					(WLAN_UMAC_COMP_MLME,
					 mlme_pdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_pdev_mlme_deinit(void)
{
	if (wlan_objmgr_unregister_pdev_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_pdev_obj_create_handler, NULL)
					!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_unregister_pdev_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_pdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

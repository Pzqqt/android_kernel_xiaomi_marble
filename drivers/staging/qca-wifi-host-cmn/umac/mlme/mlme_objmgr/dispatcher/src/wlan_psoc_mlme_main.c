/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implements PSOC MLME APIs
 */

#include <qdf_module.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_mlme_dbg.h>
#include <include/wlan_mlme_cmn.h>
#include <include/wlan_psoc_mlme.h>
#include <wlan_psoc_mlme_main.h>
#include <wlan_psoc_mlme_api.h>

struct psoc_mlme_obj *mlme_psoc_get_priv(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_obj *psoc_mlme;

	psoc_mlme = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							  WLAN_UMAC_COMP_MLME);
	if (!psoc_mlme) {
		mlme_err("PSOC MLME component object is NULL");
		return NULL;
	}

	return psoc_mlme;
}

qdf_export_symbol(mlme_psoc_get_priv);

static QDF_STATUS mlme_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc,
					       void *arg)
{
	struct psoc_mlme_obj *psoc_mlme;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	psoc_mlme = qdf_mem_malloc(sizeof(struct psoc_mlme_obj));
	if (!psoc_mlme) {
		mlme_err("Failed to allocate PSOS mlme Object");
		return QDF_STATUS_E_NOMEM;
	}

	psoc_mlme->psoc = psoc;

	status = mlme_psoc_ops_ext_hdl_create(psoc_mlme);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to allocate psoc ext handle");
		goto init_failed;
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       psoc_mlme,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Failed to attach psoc_ctx with psoc");
		goto init_failed;
	}

	return QDF_STATUS_SUCCESS;
init_failed:
	qdf_mem_free(psoc_mlme);

	return status;
}

static QDF_STATUS mlme_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc,
						void *arg)
{
	struct psoc_mlme_obj *psoc_mlme;

	psoc_mlme = mlme_psoc_get_priv(psoc);
	if (!psoc_mlme) {
		mlme_err("PSOC MLME component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	wlan_objmgr_psoc_component_obj_detach(psoc, WLAN_UMAC_COMP_MLME,
					      psoc_mlme);

	mlme_psoc_ops_ext_hdl_destroy(psoc_mlme);

	qdf_mem_free(psoc_mlme);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_psoc_mlme_init(void)
{
	if (wlan_objmgr_register_psoc_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_psoc_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_register_psoc_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_psoc_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS) {
		if (wlan_objmgr_unregister_psoc_create_handler
					(WLAN_UMAC_COMP_MLME,
					 mlme_psoc_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_psoc_mlme_deinit(void)
{
	if (wlan_objmgr_unregister_psoc_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_psoc_obj_create_handler, NULL)
					!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_unregister_psoc_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_psoc_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

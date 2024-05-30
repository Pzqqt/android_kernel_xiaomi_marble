/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This implementation of init/deint functions for FTM services.
 */

#include <wlan_ftm_init_deinit_api.h>
#include <wlan_ftm_ucfg_api.h>
#include <wlan_objmgr_global_obj.h>
#include "../../core/src/wlan_ftm_svc_i.h"
#include <wlan_cmn.h>
#include <qdf_module.h>

QDF_STATUS dispatcher_ftm_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_pdev_create_handler(WLAN_UMAC_COMP_FTM,
			wlan_ftm_pdev_obj_create_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		goto err_pdev_create;

	status = wlan_objmgr_register_pdev_destroy_handler(WLAN_UMAC_COMP_FTM,
			wlan_ftm_pdev_obj_destroy_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		goto err_pdev_delete;

	return QDF_STATUS_SUCCESS;

err_pdev_delete:
	wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_FTM,
			wlan_ftm_pdev_obj_create_notification, NULL);
err_pdev_create:
	return status;
}

QDF_STATUS dispatcher_ftm_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_FTM,
			wlan_ftm_pdev_obj_create_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_unregister_pdev_destroy_handler(WLAN_UMAC_COMP_FTM,
			wlan_ftm_pdev_obj_destroy_notification, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dispatcher_ftm_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	/* calling the wmi event handler registration */
	return wlan_ftm_testmode_attach(psoc);
}

QDF_STATUS dispatcher_ftm_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	/* calling the wmi event handler de-registration */
	return wlan_ftm_testmode_detach(psoc);
}

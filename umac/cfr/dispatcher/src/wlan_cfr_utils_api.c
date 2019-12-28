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

#include <wlan_cfr_utils_api.h>
#include <wlan_cfr_tgt_api.h>
#include <qdf_module.h>
#include "../../core/inc/cfr_defs_i.h"
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_pdev_obj.h>

QDF_STATUS wlan_cfr_init(void)
{
	if (wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_psoc_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_psoc_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_pdev_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_pdev_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_peer_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_peer_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_peer_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_peer_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cfr_deinit(void)
{
	if (wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_psoc_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_psoc_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_pdev_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_pdev_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_peer_create_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_peer_obj_create_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_peer_destroy_handler(WLAN_UMAC_COMP_CFR,
				wlan_cfr_peer_obj_destroy_handler, NULL)
				!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cfr_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	int status;

	/* chip specific init */
	status = tgt_cfr_init_pdev(pdev);

	if (status != QDF_STATUS_SUCCESS) {
		cfr_err("tgt_cfr_init_pdev failed with %d\n", status);
		return QDF_STATUS_SUCCESS;
	}

	/* RealyFS init */
	status = cfr_streamfs_init(pdev);

	if (status != QDF_STATUS_SUCCESS) {
		cfr_err("cfr_streamfs_init failed with %d\n", status);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cfr_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	int status = QDF_STATUS_SUCCESS;
	/*
	 * DBR does not have close as of now;
	 * but this is getting added as part for new gerrit
	 * Once we have that support we will add it.
	 */
	status = cfr_streamfs_remove(pdev);

	return status;
}

QDF_STATUS cfr_initialize_pdev(struct wlan_objmgr_pdev *pdev)
{
	int status = QDF_STATUS_SUCCESS;

	/* chip specific init */

	status = tgt_cfr_init_pdev(pdev);

	if (status != QDF_STATUS_SUCCESS)
		cfr_err("cfr_initialize_pdev status=%d\n", status);

	return status;
}
qdf_export_symbol(cfr_initialize_pdev);

QDF_STATUS cfr_deinitialize_pdev(struct wlan_objmgr_pdev *pdev)
{
	int status = QDF_STATUS_SUCCESS;

	/* chip specific deinit */

	status = tgt_cfr_deinit_pdev(pdev);

	if (status != QDF_STATUS_SUCCESS)
		cfr_err("cfr_deinitialize_pdev status=%d\n", status);

	return status;
}
qdf_export_symbol(cfr_deinitialize_pdev);

uint8_t count_set_bits(uint32_t value)
{
	uint8_t count = 0;

	while (value) {
		value &= (value - 1);
		count++;
	}

	return count;
}

qdf_export_symbol(count_set_bits);

#ifdef WLAN_ENH_CFR_ENABLE
void wlan_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf)
{
	tgt_cfr_rx_tlv_process(pdev, nbuf);
}

qdf_export_symbol(wlan_cfr_rx_tlv_process);
#endif

/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_objmgr_psoc_service_ready_api.c
 *
 * Public APIs implementation source file for accessing (ext)service ready
 * data from psoc object
 */
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_psoc_service_ready_api.h"

void
wlan_objmgr_populate_service_ready_data(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_psoc_service_ready_param *data)
{
	wlan_psoc_obj_lock(psoc);
	psoc->service_param = *data;
	wlan_psoc_obj_unlock(psoc);
}
EXPORT_SYMBOL(wlan_objmgr_populate_service_ready_data);

void
wlan_objmgr_populate_ext_service_ready_data(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_psoc_ext_service_ready_param *data)
{
	wlan_psoc_obj_lock(psoc);
	psoc->ext_service_param = *data;
	wlan_psoc_obj_unlock(psoc);
}
EXPORT_SYMBOL(wlan_objmgr_populate_ext_service_ready_data);

QDF_STATUS wlan_objmgr_ext_service_ready_chainmask_table_alloc(
		struct wlan_psoc_host_service_ext_param *service_ext_param)
{
	int i;
	uint32_t alloc_size;

	if (service_ext_param->num_chainmask_tables > 0) {
		for (i = 0; i < service_ext_param->num_chainmask_tables; i++) {
			alloc_size = (sizeof(struct wlan_psoc_host_chainmask_capabilities) *
					service_ext_param->chainmask_table[i].num_valid_chainmasks);
			service_ext_param->chainmask_table[i].cap_list = qdf_mem_alloc_outline(NULL, alloc_size);
			if (service_ext_param->chainmask_table[i].cap_list == NULL) {
				wlan_objmgr_ext_service_ready_chainmask_table_free(service_ext_param);
				return QDF_STATUS_E_NOMEM;
			}
		}
		return QDF_STATUS_SUCCESS;
	} else {
		return QDF_STATUS_E_NOSUPPORT;
	}

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_ext_service_ready_chainmask_table_alloc);

QDF_STATUS wlan_objmgr_ext_service_ready_chainmask_table_free(
		struct wlan_psoc_host_service_ext_param *service_ext_param)
{
	struct wlan_psoc_host_chainmask_table *table = NULL;
	int i;

	for (i = 0; i < service_ext_param->num_chainmask_tables; i++) {
		table =  &(service_ext_param->chainmask_table[i]);
		if (table->cap_list) {
			qdf_mem_free(table->cap_list);
			table->cap_list = NULL;
		}
	}

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(wlan_objmgr_ext_service_ready_chainmask_table_free);

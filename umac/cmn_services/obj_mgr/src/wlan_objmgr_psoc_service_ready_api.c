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
					void *data)
{
	struct wlan_objmgr_psoc_service_ready_param *service_ready_data = data;

	wlan_psoc_obj_lock(psoc);
	qdf_mem_copy(&psoc->service_param, service_ready_data,
		     sizeof(psoc->service_param));
	wlan_psoc_obj_unlock(psoc);
}

void
wlan_objmgr_populate_ext_service_ready_data(struct wlan_objmgr_psoc *psoc,
					    void *data)
{
	struct wlan_objmgr_psoc_ext_service_ready_param *ext_data = data;

	wlan_psoc_obj_lock(psoc);
	qdf_mem_copy(&psoc->ext_service_param, ext_data,
		     sizeof(psoc->ext_service_param));
	wlan_psoc_obj_unlock(psoc);
}

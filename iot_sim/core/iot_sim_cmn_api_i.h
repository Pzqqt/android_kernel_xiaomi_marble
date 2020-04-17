/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _IOT_SIM_CMN_API_I_H_
#define _IOT_SIM_CMN_API_I_H_

#include "iot_sim_defs_i.h"

/*
 * wlan_iot_sim_psoc_obj_create_handler() - handler for psoc object create
 * @psoc: reference to global psoc object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate psoc object created. Hence iot_sim_context
 * object can be created and attached to psoc component list.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE if psoc is null
 *         QDF_STATUS_E_NOMEM on failure of iot_sim object allocation
 */
QDF_STATUS wlan_iot_sim_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc,
						void *arg);

/*
 * wlan_iot_sim_psoc_obj_destroy_handler() - handler for psoc object delete
 * @psoc: reference to global psoc object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate psoc object going to be deleted.
 * Hence iot_sim_context object can be detached from psoc component list.
 * Then iot_sim_context object can be deleted.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS wlan_iot_sim_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc,
						 void *arg);

/*
 * iot_sim_get_ctx_from_pdev() - API to get iot_sim context object
 *                               from pdev
 * @pdev : Reference to psoc global object
 *
 * This API used to get iot sim context object from global psoc reference.
 * Null check should be done before invoking this inline function.
 *
 * Return : Reference to iot_sim_context object
 *
 */
static inline struct iot_sim_context *
iot_sim_get_ctx_from_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct iot_sim_context *isc = NULL;

	if (psoc) {
		isc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							    WLAN_IOT_SIM_COMP);
	}

	return isc;
}

#endif /* _IOT_SIM_CMN_API_I_H_ */

/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains various object manager related wrappers and helpers
 */

#ifndef __CFG_OBJMGR_H
#define __CFG_OBJMGR_H

#include "wlan_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"

/* Private Data */

#define cfg_psoc_get_priv(psoc) \
	wlan_objmgr_psoc_get_comp_private_obj((psoc), WLAN_UMAC_COMP_CONFIG)
#define cfg_psoc_set_priv(psoc, priv) \
	wlan_objmgr_psoc_component_obj_attach((psoc), WLAN_UMAC_COMP_CONFIG, \
					      (priv), QDF_STATUS_SUCCESS)
#define cfg_psoc_unset_priv(psoc, priv) \
	wlan_objmgr_psoc_component_obj_detach((psoc), WLAN_UMAC_COMP_CONFIG, \
					      (priv))

/* event registration */

#define cfg_psoc_register_create(callback) \
	wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_CONFIG, \
						 (callback), NULL)
#define cfg_psoc_register_destroy(callback) \
	wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_CONFIG, \
						  (callback), NULL)

/* event de-registration */

#define cfg_psoc_unregister_create(callback) \
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_CONFIG, \
						   (callback), NULL)
#define cfg_psoc_unregister_destroy(callback) \
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_CONFIG, \
						    (callback), NULL)

#endif /* __CFG_OBJMGR_H */

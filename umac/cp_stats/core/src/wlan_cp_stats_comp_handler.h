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
 * DOC: wlan_cp_stats_comp_handler.h
 *
 * This header file API declarations required to attach/detach and
 * enable/disable other UMAC component specific control plane statitics
 * to cp stats component object
 */

#ifndef __WLAN_CP_STATS_COMP_HANDLER_H__
#define __WLAN_CP_STATS_COMP_HANDLER_H__

#ifdef QCA_SUPPORT_CP_STATS
#include "wlan_cp_stats_defs.h"

/**
 * wlan_cp_stats_comp_obj_config() - attach/detach component specific stats
 * callback function
 * @obj_type: common object type
 * @cfg_state: config state either to attach of detach
 * @comp_id: cpstats component id
 * @cmn_obj: pointer to common object
 * @comp_priv_obj: pointer to component specific cp stats object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_comp_obj_config(
		enum wlan_objmgr_obj_type obj_type,
		enum wlan_cp_stats_cfg_state cfg_state,
		enum wlan_cp_stats_comp_id comp_id,
		void *cmn_obj,
		void *comp_priv_obj);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_COMP_HANDLER_H__ */

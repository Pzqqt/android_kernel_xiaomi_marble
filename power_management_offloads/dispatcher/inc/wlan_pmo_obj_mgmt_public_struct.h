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
  * DOC: Declare various struct, macros which are used for object mgmt in pmo.
  *
  * Note: This file shall not contain public API's prototype/declarations.
  *
  */


#ifndef _WLAN_PMO_OBJ_MGMT_PUBLIC_STRUCT_H_
#define _WLAN_PMO_OBJ_MGMT_PUBLIC_STRUCT_H_

#include "wlan_pmo_common_public_struct.h"
#include "wlan_pmo_arp_public_struct.h"
#include "wlan_pmo_ns_public_struct.h"
#include "wlan_pmo_gtk_public_struct.h"
#include "wlan_pmo_wow_public_struct.h"
#include "wlan_pmo_mc_addr_filtering_public_struct.h"

/**
 * struct pmo_psoc_priv_obj - psoc related data require for pmo
 * @wow: place holder for psoc wow configuration
 * @ps_config: place holder for psoc power configuration
 * @psoc_cfg: place holder for psoc configuration
 * @wow: true  wow configuration
 * @lphb_cache: lphb cache
 * @lock: spin lock for pmo psoc
 */
struct pmo_psoc_priv_obj {
	struct pmo_psoc_cfg psoc_cfg;
	struct pmo_wow wow;
	qdf_spinlock_t lock;
};

/**
 * struct wlan_pmo_ctx -offload mgr context
 * @psoc_context:     psoc context
 * @pmo_suspend_handler: suspend handler table for all componenets
 * @pmo_suspend_handler_arg: suspend handler argument sfor all componenets
 * @pmo_resume_handler: resume handler table for all componenets
 * @pmo_resume_handler_arg: resume handler argument for all componenets
 * @lock: lock for global pmo ctx
 */
struct wlan_pmo_ctx {
	pmo_psoc_suspend_handler
		pmo_suspend_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pmo_suspend_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	pmo_psoc_resume_handler
		pmo_resume_handler[WLAN_UMAC_MAX_COMPONENTS];
	void *pmo_resume_handler_arg[WLAN_UMAC_MAX_COMPONENTS];
	qdf_spinlock_t lock;
};

/**
 * struct pmo_vdev_priv_obj -vdev specific user configuration required for pmo
 * @pmo_psoc_ctx: pmo psoc ctx
 * @vdev_arp_req: place holder for arp request for vdev
 * @vdev_ns_req: place holder for ns request for vdev
 * @vdev_mc_list_req: place holder for mc addr list for vdev
 * @addr_filter_pattern: addr filter pattern for vdev
 * @vdev_gtk_params: place holder for gtk request for vdev
 * @gtk_err_enable: gtk error is enabled or not
 * @vdev_bpf_req: place holder for apf/bpf for vdev
 * @vdev_pkt_filter: place holder for vdev packet filter
 * @dtim_period: dtim period
 * @ptrn_match_enable: true when pattern match is enabled else false
 * @num_wow_default_patterns: number of wow default patterns for vdev
 * @num_wow_user_patterns: number of user wow patterns for vdev
 * @pmo_vdev_lock: spin lock for pmo vdev priv ctx
 */
struct pmo_vdev_priv_obj {
	struct pmo_psoc_priv_obj *pmo_psoc_ctx;
	struct pmo_arp_offload_params vdev_arp_req;
	struct pmo_ns_offload_params vdev_ns_req;
	struct pmo_mc_addr_list vdev_mc_list_req;
	uint8_t addr_filter_pattern;
	struct pmo_gtk_req vdev_gtk_req;
	struct pmo_gtk_rsp_req vdev_gtk_rsp_req;
	qdf_atomic_t gtk_err_enable;
	uint8_t dtim_period;
	bool ptrn_match_enable;
	uint8_t num_wow_default_patterns;
	uint8_t num_wow_user_patterns;
	qdf_spinlock_t pmo_vdev_lock;
};

#endif /* end  of _WLAN_PMO_OBJ_MGMT_PUBLIC_STRUCT_H_ */

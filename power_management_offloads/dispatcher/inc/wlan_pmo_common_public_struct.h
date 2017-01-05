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
 * DOC: Declare various struct, macros which are common for
 * various pmo related features.
  *
  * Note: This file shall not contain public API's prototype/declartions.
  *
  */

#ifndef _WLAN_PMO_COMMONP_PUBLIC_STRUCT_H_
#define _WLAN_PMO_COMMONP_PUBLIC_STRUCT_H_

#include "wlan_cmn.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_objmgr_peer_obj.h"
#include "wmi_unified.h"
#include "qdf_status.h"
#include "qdf_lock.h"

#define PMO_IPV4_ADDR_LEN         4

#define PMO_IPV4_ARP_REPLY_OFFLOAD                  0
#define PMO_IPV6_NEIGHBOR_DISCOVERY_OFFLOAD         1
#define PMO_IPV6_NS_OFFLOAD                         2
#define PMO_OFFLOAD_DISABLE                         0
#define PMO_OFFLOAD_ENABLE                          1

#define PMO_MAC_NS_OFFLOAD_SIZE               1
#define PMO_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA 16
#define PMO_MAC_IPV6_ADDR_LEN                 16
#define PMO_IPV6_ADDR_VALID                   1
#define PMO_IPV6_ADDR_UC_TYPE                 0
#define PMO_IPV6_ADDR_AC_TYPE                 1

#define PMO_80211_ADDR_LEN  6  /* size of 802.11 address */
/**
 * enum pmo_offload_type: tell offload type
 * @pmo_arp_offload: arp offload
 * @pmo_ns_offload: ns offload
 * @pmo_gtk_offload: gtk offload
 */
enum pmo_offload_type {
	pmo_arp_offload = 0,
	pmo_ns_offload,
	pmo_gtk_offload,
};

/**
 * typedef for psoc suspend handler
 */
typedef QDF_STATUS(*pmo_psoc_suspend_handler)
	(struct wlan_objmgr_psoc *psoc, void *arg);
/**
 * typedef for psoc resume handler
 */
typedef QDF_STATUS(*pmo_psoc_resume_handler)
	(struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * enum pmo_offload_trigger: trigger information
 * @pmo_apps_suspend: trigger is apps suspend
 * @pmo_apps_resume: trigger is apps resume
 * @pmo_runtime_suspend: trigger is runtime suspend
 * @pmo_runtime_resume: trigger is runtime resume
 * @pmo_ipv4_change_notify: trigger is ipv4 change handler
 * @pmo_ipv6_change_notify: trigger is ipv6 change handler
 * @pmo_ns_offload_dynamic_update: enable/disable ns offload on the fly
 * @pmo_peer_disconnect: trigger is peer disconnect
 * @pmo_mcbc_setting_dynamic_update: mcbc value update on the fly
 *
 * @pmo_offload_trigger_max: Max trigger value
 */
enum pmo_offload_trigger {
	pmo_apps_suspend = 0,
	pmo_apps_resume,
	pmo_runtime_suspend,
	pmo_runtime_resume,
	pmo_ipv4_change_notify,
	pmo_ipv6_change_notify,
	pmo_mc_list_change_notify,
	pmo_ns_offload_dynamic_update,
	pmo_peer_disconnect,
	pmo_mcbc_setting_dynamic_update,

	pmo_offload_trigger_max,
};

/**
 * struct pmo_psoc_cfg - user configuration required for pmo
 * @ptrn_match_enable_all_vdev: true when pattern match is enable for all vdev
 * @bpf_enable: true if psoc supports bpf else false
 * @arp_offload_enable: true if arp offload is supported for psoc else false
 * @ns_offload_enable_static: true if psoc supports ns offload in ini else false
 * @ns_offload_enable_dynamic: to enable / disable the ns offload using
 *    ioctl or vendor command.
 * @ssdp:  true if psoc supports if ssdp configuration in wow mode
 * @enable_mc_list: true if psoc supports mc addr list else false
 * @active_mode_offload: true if psoc supports active mode offload else false
 * @ap_arpns_support: true if psoc supports arp ns for ap mode
 * @max_wow_filters: maximum number of wow filter supported
 * @ra_ratelimit_enable: true when ra filtering ins eanbled else false
 * @ra_ratelimit_interval: ra packets interval
 * @magic_ptrn_enable: true when magic pattern is enabled else false
 * @deauth_enable: true when wake up on deauth is enabled else false
 * @disassoc_enable:  true when wake up on disassoc is enabled else false
 * @bmiss_enable: true when wake up on bmiss is enabled else false
 */
struct pmo_psoc_cfg {
	bool ptrn_match_enable_all_vdev;
	bool bpf_enable;
	bool arp_offload_enable;
	bool ns_offload_enable_static;
	bool ns_offload_enable_dynamic;
	bool ssdp;
	bool enable_mc_list;
	bool active_mode_offload;
	bool ap_arpns_support;
	uint8_t max_wow_filters;
	bool ra_ratelimit_enable;
	uint16_t ra_ratelimit_interval;
	bool magic_ptrn_enable;
	bool deauth_enable;
	bool disassoc_enable;
	bool bmiss_enable;
};

#endif /* end  of _WLAN_PMO_COMMONP_STRUCT_H_ */


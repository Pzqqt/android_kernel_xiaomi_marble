/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#include "qdf_event.h"
#include "wlan_pmo_hw_filter_public_struct.h"

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

#define PMO_WOW_REQUIRED_CREDITS 1

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
 * enum pmo_vdev_param_id: tell vdev param id
 * @pmo_vdev_param_listen_interval: vdev listen interval param id
 * @pmo_vdev_param_dtim_policy: vdev param dtim policy
 * @pmo_vdev_max_param: Max vdev param id
 */
enum pmo_vdev_param_id {
	pmo_vdev_param_listen_interval = 0,
	pmo_vdev_param_dtim_policy,
	pmo_vdev_max_param
};

/**
 * enum pmo_beacon_dtim_policy: tell vdev beacon policy
 * @pmo_ignore_dtim: fwr need to igonre dtime policy
 * @pmo_normal_dtim: fwr need to use normal dtime policy
 * @pmo_stick_dtim: fwr need to use stick dtime policy
 * @auto_dtim: fwr need to auto dtime policy
 */
enum pmo_beacon_dtim_policy {
	pmo_ignore_dtim = 0x01,
	pmo_normal_dtim = 0x02,
	pmo_stick_dtim = 0x03,
	pmo_auto_dtim = 0x04,
};

/**
 * @pmo_sta_ps_param_rx_wake_policy: Controls how frames are retrievd from AP
 *  while STA is sleeping.
 * @pmo_sta_ps_param_tx_wake_threshold: STA will go active after this many TX
 * @pmo_sta_ps_param_pspoll_count:No of PS-Poll to send before STA wakes up
 * @pmo_sta_ps_param_inactivity_time: TX/RX inactivity time in msec before
    going to sleep.
 * @pmo_sta_ps_param_uapsd: Set uapsd configuration.
 * @pmo_sta_ps_param_qpower_pspoll_count: No of PS-Poll to send before
    STA wakes up in QPower Mode.
 * @pmo_sta_ps_enable_qpower:  Enable QPower
 * @pmo_sta_ps_param_qpower_max_tx_before_wake: Number of TX frames before the
    entering the Active state
 */
enum pmo_sta_powersave_param {
	pmo_sta_ps_param_rx_wake_policy = 0,
	pmo_sta_ps_param_tx_wake_threshold = 1,
	pmo_sta_ps_param_pspoll_count = 2,
	pmo_sta_ps_param_inactivity_time = 3,
	pmo_sta_ps_param_uapsd = 4,
	pmo_sta_ps_param_qpower_pspoll_count = 5,
	pmo_sta_ps_enable_qpower = 6,
	pmo_sta_ps_param_qpower_max_tx_before_wake = 7,
};

/**
 * enum powersave_qpower_mode: QPOWER modes
 * @pmo_qpower_disabled: Qpower is disabled
 * @pmo_qpower_enabled: Qpower is enabled
 * @pmo_qpower_duty_cycling: Qpower is enabled with duty cycling
 */
enum pmo_power_save_qpower_mode {
	pmo_qpower_disabled = 0,
	pmo_qpower_enabled = 1,
	pmo_qpower_duty_cycling = 2
};

/**
 * enum powersave_qpower_mode: powersave_mode
 * @pmo_ps_not_supported: Power save is not supported
 * @pmo_ps_legacy_no_deep_sleep: Legacy pwr save enabled and deep sleep disabled
 * @pmo_ps_qpower_no_deep_sleep: QPOWER enabled and deep sleep disabled
 * @pmo_ps_legacy_deep_sleep: Legacy power save enabled and deep sleep enabled
 * @pmo_ps_qpower_deep_sleep: QPOWER enabled and deep sleep enabled
 * @pmo_ps_duty_cycling_qpower: QPOWER enabled in duty cycling mode
 */
enum pmo_powersave_mode {
	pmo_ps_not_supported = 0,
	pmo_ps_legacy_no_deep_sleep = 1,
	pmo_ps_qpower_no_deep_sleep = 2,
	pmo_ps_legacy_deep_sleep = 3,
	pmo_ps_qpower_deep_sleep = 4,
	pmo_ps_duty_cycling_qpower = 5
};

/**
 * enum wow_resume_trigger - resume trigger override setting values
 * @PMO_WOW_RESUME_TRIGGER_DEFAULT: fw to use platform default resume trigger
 * @PMO_WOW_RESUME_TRIGGER_HTC_WAKEUP: force fw to use HTC Wakeup to resume
 * @PMO_WOW_RESUME_TRIGGER_GPIO: force fw to use GPIO to resume
 * @PMO_WOW_RESUME_TRIGGER_COUNT: number of resume trigger options
 */
enum pmo_wow_resume_trigger {
	/* always first */
	PMO_WOW_RESUME_TRIGGER_DEFAULT = 0,
	PMO_WOW_RESUME_TRIGGER_HTC_WAKEUP,
	PMO_WOW_RESUME_TRIGGER_GPIO,
	/* always last */
	PMO_WOW_RESUME_TRIGGER_COUNT
};

/**
 * enum wow_interface_pause - interface pause override setting values
 * @PMO_WOW_INTERFACE_PAUSE_DEFAULT: use platform default iface pause setting
 * @PMO_WOW_INTERFACE_PAUSE_ENABLE: force interface pause setting to enabled
 * @PMO_WOW_INTERFACE_PAUSE_DISABLE: force interface pause setting to disabled
 * @PMO_WOW_INTERFACE_PAUSE_COUNT: number of interface pause options
 */
enum pmo_wow_interface_pause {
	/* always first */
	PMO_WOW_INTERFACE_PAUSE_DEFAULT = 0,
	PMO_WOW_INTERFACE_PAUSE_ENABLE,
	PMO_WOW_INTERFACE_PAUSE_DISABLE,
	/* always last */
	PMO_WOW_INTERFACE_PAUSE_COUNT
};

#define PMO_TGT_SUSPEND_COMPLETE_TIMEOUT   6000
#define PMO_WAKE_LOCK_TIMEOUT              1000
#define PMO_RESUME_TIMEOUT                 6000

/**
 * struct wow_enable_params - A collection of wow enable override parameters
 * @is_unit_test: true to notify fw this is a unit-test suspend
 * @interface_pause: used to override the interface pause indication sent to fw
 * @resume_trigger: used to force fw to use a particular resume method
 */
struct pmo_wow_enable_params {
	bool is_unit_test;
	enum pmo_wow_interface_pause interface_pause;
	enum pmo_wow_resume_trigger resume_trigger;
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
 * enum pmo_auto_pwr_detect_failure_mode_t - auto detect failure modes
 * @PMO_FW_TO_CRASH_ON_PWR_FAILURE: Don't register wow wakeup event and FW
 * crashes on power failure
 * @PMO_FW_TO_SEND_WOW_IND_ON_PWR_FAILURE: Register wow wakeup event and FW
 * sends failure event to host on power failure
 * @PMO_FW_TO_REJUVENATE_ON_PWR_FAILURE: Don't register wow wakeup event and
 * FW silently rejuvenate on power failure
 * @PMO_AUTO_PWR_FAILURE_DETECT_DISABLE: Don't register wow wakeup event and the
 * auto power failure detect feature is disabled in FW.
 */
enum pmo_auto_pwr_detect_failure_mode {
	PMO_FW_TO_CRASH_ON_PWR_FAILURE,
	PMO_FW_TO_SEND_WOW_IND_ON_PWR_FAILURE,
	PMO_FW_TO_REJUVENATE_ON_PWR_FAILURE,
	PMO_AUTO_PWR_FAILURE_DETECT_DISABLE
};

/**
 * struct pmo_psoc_cfg - user configuration required for pmo
 * @ptrn_match_enable_all_vdev: true when pattern match is enable for all vdev
 * @ptrn_id_per_vdev: true when pattern id can be same for different vdev
 * @bpf_enable: true if psoc supports bpf else false
 * @arp_offload_enable: true if arp offload is supported for psoc else false
 * @hw_filter_mode: which mode the hardware filter should use during DTIM
 * @ns_offload_enable_static: true if psoc supports ns offload in ini else false
 * @ns_offload_enable_dynamic: to enable / disable the ns offload using
 *    ioctl or vendor command.
 * @ssdp:  true if psoc supports if ssdp configuration in wow mode
 * @enable_mc_list: true if psoc supports mc addr list else false
 * @active_mode_offload: true if psoc supports active mode offload else false
 * @ap_arpns_support: true if psoc supports arp ns for ap mode
 * @d0_wow_supported: true if psoc supports D0 wow command
 * @max_wow_filters: maximum number of wow filter supported
 * @ra_ratelimit_enable: true when ra filtering ins eanbled else false
 * @ra_ratelimit_interval: ra packets interval
 * @magic_ptrn_enable: true when magic pattern is enabled else false
 * @deauth_enable: true when wake up on deauth is enabled else false
 * @disassoc_enable:  true when wake up on disassoc is enabled else false
 * @bmiss_enable: true when wake up on bmiss is enabled else false
 * @nan_enable:  true when nan is enabled else false
 * @lpass_enable: true when lpass is enabled else false
 * @sta_dynamic_dtim: station dynamic DTIM value
 * @sta_mod_dtim: station modulated DTIM value
 * @sta_max_li_mod_dtim: station max listen interval DTIM value
 * @power_save_mode: power save mode for psoc
 * @auto_power_save_fail_mode: auto detect power save failure
 */
struct pmo_psoc_cfg {
	bool ptrn_match_enable_all_vdev;
	bool ptrn_id_per_vdev;
	bool bpf_enable;
	bool arp_offload_enable;
	enum pmo_hw_filter_mode hw_filter_mode;
	bool ns_offload_enable_static;
	bool ns_offload_enable_dynamic;
	bool ssdp;
	bool enable_mc_list;
	bool active_mode_offload;
	bool ap_arpns_support;
	bool d0_wow_supported;
	uint8_t max_wow_filters;
	bool ra_ratelimit_enable;
	uint16_t ra_ratelimit_interval;
	bool magic_ptrn_enable;
	bool deauth_enable;
	bool disassoc_enable;
	bool bmiss_enable;
	bool nan_enable;
	bool lpass_enable;
	uint8_t sta_dynamic_dtim;
	uint8_t sta_mod_dtim;
	uint8_t sta_max_li_mod_dtim;
	uint8_t power_save_mode;
	enum pmo_auto_pwr_detect_failure_mode auto_power_save_fail_mode;
};

/**
 * pmo_device_caps - device capability flags (true if feature is supported)
 * @apf: Android Packet Filter (aka BPF)
 * @arp_ns_offload: APR/NS offload
 * @packet_filter: Legacy "Packet Filter"
 * @unified_wow: Firmware supports "interface pause" flag in WoW command.
 *	This allows both D0-WoW (bus up) and Non-D0-WoW (bus down) to use one
 *	unified command
 */
struct pmo_device_caps {
	bool apf;
	bool arp_ns_offload;
	bool packet_filter;
	bool unified_wow;
};

#endif /* end  of _WLAN_PMO_COMMONP_STRUCT_H_ */


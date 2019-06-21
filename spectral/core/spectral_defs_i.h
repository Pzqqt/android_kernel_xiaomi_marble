/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifndef _SPECTRAL_DEFS_I_H_
#define _SPECTRAL_DEFS_I_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <qdf_list.h>
#include <qdf_timer.h>
#include <qdf_util.h>
#include <wlan_spectral_public_structs.h>
#include <wlan_spectral_utils_api.h>
#include <spectral_ioctl.h>

#define spectral_fatal(format, args...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_err(format, args...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_warn(format, args...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_info(format, args...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_debug(format, args...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_SPECTRAL, format, ## args)

#define spectral_fatal_nofl(format, args...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_err_nofl(format, args...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_warn_nofl(format, args...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_info_nofl(format, args...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_debug_nofl(format, args...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)

#define spectral_fatal_rl(format, args...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_err_rl(format, args...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_warn_rl(format, args...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_info_rl(format, args...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_debug_rl(format, args...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_SPECTRAL, format, ## args)

#define spectral_fatal_rl_nofl(format, args...) \
	QDF_TRACE_FATAL_RL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_err_rl_nofl(format, args...) \
	QDF_TRACE_ERROR_RL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_warn_rl_nofl(format, args...) \
	QDF_TRACE_WARN_RL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_info_rl_nofl(format, args...) \
	QDF_TRACE_INFO_RL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)
#define spectral_debug_rl_nofl(format, args...) \
	QDF_TRACE_DEBUG_RL_NO_FL(QDF_MODULE_ID_SPECTRAL, format, ## args)

/**
 * struct pdev_spectral - Radio specific spectral object
 * @psptrl_pdev:          Back-pointer to struct wlan_objmgr_pdev
 * @spectral_sock:        Spectral Netlink socket for sending samples to
 *                        applications
 * @psptrl_target_handle: reference to spectral lmac object
 * @skb:                  Socket buffer for sending samples to applications
 * @spectral_pid :        Spectral port ID
 */
struct pdev_spectral {
	struct wlan_objmgr_pdev *psptrl_pdev;
	struct sock *spectral_sock;
	void *psptrl_target_handle;
	struct sk_buff *skb[SPECTRAL_MSG_TYPE_MAX];
	uint32_t spectral_pid;
};

struct wmi_spectral_cmd_ops;
/**
 * struct spectral_context - spectral global context
 * @psoc_obj:               Reference to psoc global object
 * @spectral_legacy_cbacks: Spectral legacy callbacks
 *
 * Call back functions to invoke independent of OL/DA
 * @sptrlc_ucfg_phyerr_config:     ucfg handler for phyerr
 * @sptrlc_pdev_spectral_init:     Init spectral
 * @sptrlc_pdev_spectral_deinit:   Deinit spectral
 * @sptrlc_set_spectral_config:    Set spectral configurations
 * @sptrlc_get_spectral_config:    Get spectral configurations
 * @sptrlc_start_spectral_scan:    Start spectral scan
 * @sptrlc_stop_spectral_scan:     Stop spectral scan
 * @sptrlc_is_spectral_active:     Check if spectral scan is active
 * @sptrlc_is_spectral_enabled:    Check if spectral is enabled
 * @sptrlc_set_debug_level:        Set debug level
 * @sptrlc_get_debug_level:        Get debug level
 * @sptrlc_get_spectral_capinfo:   Get spectral capability info
 * @sptrlc_get_spectral_diagstats: Get spectral diag status
 * @sptrlc_register_wmi_spectral_cmd_ops: Register wmi_spectral_cmd operations
 * @sptrlc_register_netlink_cb: Register Netlink callbacks
 * @sptrlc_use_nl_bcast: Check whether to use Netlink broadcast/unicast
 * @sptrlc_deregister_netlink_cb: De-register Netlink callbacks
 * @sptrlc_process_spectral_report: Process spectral report
 */
struct spectral_context {
	struct wlan_objmgr_psoc *psoc_obj;
	struct spectral_legacy_cbacks legacy_cbacks;
	QDF_STATUS (*sptrlc_spectral_control)
					(struct wlan_objmgr_pdev *pdev,
					 struct spectral_cp_request *sscan_req);
	int (*sptrlc_ucfg_phyerr_config)(struct wlan_objmgr_pdev *pdev,
					 void *ad);
	void * (*sptrlc_pdev_spectral_init)(struct wlan_objmgr_pdev *pdev);
	void (*sptrlc_pdev_spectral_deinit)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlc_set_spectral_config)
				(struct wlan_objmgr_pdev *pdev,
				 const uint32_t threshtype,
				 const uint32_t value,
				 const enum spectral_scan_mode smode,
				 enum spectral_cp_error_code *err);
	QDF_STATUS (*sptrlc_get_spectral_config)
					(struct wlan_objmgr_pdev *pdev,
					 struct spectral_config *sptrl_config,
					 const enum spectral_scan_mode smode);
	QDF_STATUS (*sptrlc_start_spectral_scan)
					(struct wlan_objmgr_pdev *pdev,
					 const enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);
	QDF_STATUS (*sptrlc_stop_spectral_scan)(struct wlan_objmgr_pdev *pdev,
						enum spectral_scan_mode smode);
	bool (*sptrlc_is_spectral_active)(struct wlan_objmgr_pdev *pdev,
					  enum spectral_scan_mode smode);
	bool (*sptrlc_is_spectral_enabled)(struct wlan_objmgr_pdev *pdev,
					   enum spectral_scan_mode smode);
	QDF_STATUS (*sptrlc_set_debug_level)(struct wlan_objmgr_pdev *pdev,
					     uint32_t debug_level);
	uint32_t (*sptrlc_get_debug_level)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlc_get_spectral_capinfo)(struct wlan_objmgr_pdev *pdev,
						  struct spectral_caps *scaps);
	QDF_STATUS (*sptrlc_get_spectral_diagstats)
					(struct wlan_objmgr_pdev *pdev,
					 struct spectral_diag_stats *stats);
	void (*sptrlc_register_wmi_spectral_cmd_ops)(
			struct wlan_objmgr_pdev *pdev,
			struct wmi_spectral_cmd_ops *cmd_ops);
	void (*sptrlc_register_netlink_cb)(
		struct wlan_objmgr_pdev *pdev,
		struct spectral_nl_cb *nl_cb);
	bool (*sptrlc_use_nl_bcast)(struct wlan_objmgr_pdev *pdev);
	void (*sptrlc_deregister_netlink_cb)(struct wlan_objmgr_pdev *pdev);
	int (*sptrlc_process_spectral_report)(
		struct wlan_objmgr_pdev *pdev,
		void *payload);
};

#endif				/* _SPECTRAL_DEFS_I_H_ */

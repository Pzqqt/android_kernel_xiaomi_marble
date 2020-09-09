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

/**
 * DOC: This file has main dcs structures definition.
 */

#ifndef _WLAN_DCS_H_
#define _WLAN_DCS_H_

#include <wmi_unified_param.h>
#include "wlan_dcs_tgt_api.h"
#include "wlan_dcs_ucfg_api.h"

#define dcs_debug(args ...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_DCS, ## args)
#define dcs_info(args ...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_DCS, ## args)
#define dcs_err(args ...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_DCS, ## args)

#define WLAN_DCS_MAX_PDEVS 3

#define DCS_TX_MAX_CU  30
#define MAX_DCS_TIME_RECORD 10
#define DCS_FREQ_CONTROL_TIME (5 * 60 * 1000)

/**
 * enum wlan_dcs_debug_level - dcs debug trace level
 * @DCS_DEBUG_DISABLE: disable debug trace
 * @DCS_DEBUG_CRITICAL: critical debug trace level
 * @DCS_DEBUG_VERBOSE:  verbose debug trace level
 */
enum wlan_dcs_debug_level {
	DCS_DEBUG_DISABLE = 0,
	DCS_DEBUG_CRITICAL = 1,
	DCS_DEBUG_VERBOSE = 2
};

/**
 * struct pdev_dcs_im_stats - define dcs interference mitigation
 *                            stats in pdev object
 * @prev_dcs_im_stats: previous statistics at last time
 * @user_dcs_im_stats: statistics requested from userspace
 * @im_intfr_cnt: number of times the interference is
 *                detected within detection window
 * @im_sample_cnt: sample counter
 */
struct pdev_dcs_im_stats {
	struct wlan_host_dcs_im_tgt_stats prev_dcs_im_stats;
	struct wlan_host_dcs_im_user_stats user_dcs_im_stats;
	uint8_t im_intfr_cnt;
	uint8_t im_samp_cnt;
};

/**
 * struct pdev_dcs_params - define dcs configuration parameter in pdev object
 * @dcs_enable_cfg: dcs enable from ini config
 * @dcs_enable: dcs enable from ucfg config
 * @dcs_algorithm_process: do dcs algorithm process or not
 * @dcs_debug: dcs debug trace level
 * @phy_err_penalty: phy error penalty
 * @phy_err_threshold: phy error threshold
 * @radar_err_threshold: radar error threshold
 * @coch_intfr_threshold: co-channel interference threshold
 * @user_max_cu: tx channel utilization due to AP's tx and rx
 * @intfr_detection_threshold: interference detection threshold
 * @intfr_detection_window: interference sampling window
 * @tx_err_threshold: transmission failure rate threshold
 * @user_request_count: counter of stats requested from userspace
 * @notify_user: whether to notify userspace
 */
struct pdev_dcs_params {
	uint8_t dcs_enable_cfg;
	uint8_t dcs_enable;
	bool dcs_algorithm_process;
	enum wlan_dcs_debug_level dcs_debug;
	uint32_t phy_err_penalty;
	uint32_t phy_err_threshold;
	uint32_t radar_err_threshold;
	uint32_t coch_intfr_threshold;
	uint32_t user_max_cu;
	uint32_t intfr_detection_threshold;
	uint32_t intfr_detection_window;
	uint32_t tx_err_threshold;
	uint32_t user_request_count;
	uint8_t notify_user;
};

/**
 * struct pdev_dcs_freq_ctrl_params - define dcs frequency control parameter
 *                                    in pdebv object
 * @disable_threshold_per_5mins: in five minutes, if dcs happen more than
 *                               threshold, then disable dcs for some time
 * @restart_delay: when dcs happen more than threshold in five minutes,
 *                 then start to disable dcs for restart_delay minutes
 * @timestamp: record dcs happened timestamp
 * @dcs_happened_count: dcs happened count
 * @disable_delay_process: in dcs disable delay process or not
 */
struct pdev_dcs_freq_ctrl_params {
	uint8_t disable_threshold_per_5mins;
	uint32_t restart_delay;
	unsigned long timestamp[MAX_DCS_TIME_RECORD];
	unsigned long dcs_happened_count;
	bool disable_delay_process;
};

/**
 * struct pdev_dcs_timer_args - define pdev dcs timer args
 * @psoc: psoc pointer
 * @pdev_id: pdev id
 */
struct pdev_dcs_timer_args {
	struct wlan_objmgr_psoc *psoc;
	uint32_t pdev_id;
};

/**
 * struct psoc_dcs_cbk - define dcs callback in psoc oject
 * @cbk: callback
 * @arg: arguments
 */
struct psoc_dcs_cbk {
	dcs_callback cbk;
	void *arg;
};

/**
 * struct dcs_pdev_priv_obj - define dcs pdev priv
 * @dcs_host_params: dcs host configuration parameter
 * @dcs_im_stats: dcs im statistics
 * @dcs_freq_ctrl_params: dcs frequency control parameter
 * @dcs_disable_timer: dcs disable timer
 * @dcs_timer_args: dcs disable timer args
 * @lock: lock to protect dcs pdev priv
 * @requestor_vdev_id: user request vdev id
 * @user_cb: user request callback
 */
struct dcs_pdev_priv_obj {
	struct pdev_dcs_params dcs_host_params;
	struct pdev_dcs_im_stats dcs_im_stats;
	struct pdev_dcs_freq_ctrl_params dcs_freq_ctrl_params;
	qdf_timer_t dcs_disable_timer;
	struct pdev_dcs_timer_args dcs_timer_args;
	qdf_spinlock_t lock;
	uint8_t requestor_vdev_id;
	void (*user_cb)(uint8_t vdev_id,
			struct wlan_host_dcs_im_user_stats *stats,
			int status);
};

/**
 * struct dcs_psoc_priv_obj - define dcs psoc priv
 * @dcs_pdev_priv: dcs pdev priv
 * @dcs_cbk: dcs callback
 */
struct dcs_psoc_priv_obj {
	struct dcs_pdev_priv_obj dcs_pdev_priv[WLAN_DCS_MAX_PDEVS];
	struct psoc_dcs_cbk dcs_cbk;
};

/**
 * wlan_dcs_get_pdev_private_obj() - get dcs pdev private object
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 *
 * API to retrieve the pdev private object from the psoc context
 *
 * Return: pdev private object pointer on success, NULL on error
 */
struct dcs_pdev_priv_obj *
wlan_dcs_get_pdev_private_obj(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id);

/**
 * wlan_dcs_attach() - Attach dcs handler
 * @psoc: psoc pointer
 *
 * This function gets called to register dcs FW events handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_dcs_attach(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_dcs_detach() - Detach dcs handler
 * @psoc: psoc pointer
 *
 * This function gets called to unregister dcs FW events handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_dcs_detach(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_dcs_cmd_send() - Send dcs command to target_if layer
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 * @is_host_pdev_id: pdev_id is host id or not
 *
 * The function gets called to send dcs command to FW
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wlan_dcs_cmd_send(struct wlan_objmgr_psoc *psoc,
			     uint32_t pdev_id,
			     bool is_host_pdev_id);

/**
 * wlan_dcs_process() - dcs process main entry
 * @psoc: psoc pointer
 * @event: dcs stats event pointer
 *
 * This function is the main entry to do dcs related operation
 * such as algorithm handling and dcs frequency control.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_dcs_process(struct wlan_objmgr_psoc *psoc,
			    struct dcs_stats_event *event);

/**
 * wlan_dcs_disable_timer_fn() - dcs disable timer callback
 * @dcs_timer_args: dcs timer argument pointer
 *
 * This function gets called when dcs disable timer timeout
 *
 * Return: None
 */
void wlan_dcs_disable_timer_fn(void *dcs_timer_args);

/**
 * wlan_dcs_clear() - clear dcs information
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 *
 * The function gets called to clear dcs information such as dcs
 * frequency control parameters and stop dcs disable timer
 *
 * Return: None
 */
void wlan_dcs_clear(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id);

/**
 * wlan_dcs_set_algorithm_process() - config dcs event data to do algorithm
 * process or not
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 * @dcs_algorithm_process: dcs algorithm process
 *
 * The function gets called to config dcs event data to do algorithm
 * process or not
 *
 * Return: None
 */
void wlan_dcs_set_algorithm_process(struct wlan_objmgr_psoc *psoc,
				    uint32_t pdev_id,
				    bool dcs_algorithm_process);

/*
 * wlan_dcs_pdev_obj_lock() - private API to acquire spinlock at pdev
 * @dcs_pdev: pointer to dcs pdev object
 *
 * Return: void
 */
static inline void wlan_dcs_pdev_obj_lock(struct dcs_pdev_priv_obj *dcs_pdev)
{
	qdf_spin_lock_bh(&dcs_pdev->lock);
}

/**
 * wlan_dcs_pdev_obj_unlock() - private api to release spinlock at pdev
 * @dcs_pdev: pointer to dcs pdev object
 *
 * Return: void
 */
static inline void wlan_dcs_pdev_obj_unlock(struct dcs_pdev_priv_obj *dcs_pdev)
{
	qdf_spin_unlock_bh(&dcs_pdev->lock);
}
#endif  /* _WLAN_DCS_H_ */

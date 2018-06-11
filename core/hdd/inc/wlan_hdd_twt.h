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
 * DOC : wlan_hdd_twt.h
 *
 * WLAN Host Device Driver file for TWT (Target Wake Time) support.
 *
 */

#if !defined(WLAN_HDD_TWT_H)
#define WLAN_HDD_TWT_H

#include "qdf_types.h"
#include "qdf_status.h"

struct hdd_context;
struct wma_tgt_cfg;
struct wmi_twt_enable_complete_event_param;

#ifdef WLAN_SUPPORT_TWT
/**
 * enum twt_status - TWT target state
 * @TWT_INIT: Init State
 * @TWT_DISABLED: TWT is disabled
 * @TWT_FW_TRIGGER_ENABLE_REQUESTED: FW triggered enable requested
 * @TWT_FW_TRIGGER_ENABLED: FW triggered twt enabled
 * @TWT_HOST_TRIGGER_ENABLE_REQUESTED: Host triggered TWT requested
 * @TWT_HOST_TRIGGER_ENABLED: Host triggered TWT enabled
 * @TWT_DISABLE_REQUESTED: TWT disable requested
 * @TWT_SUSPEND_REQUESTED: TWT suspend requested
 * @TWT_SUSPENDED: Successfully suspended TWT
 * @TWT_RESUME_REQUESTED: TWT Resume requested
 * @TWT_RESUMED: Successfully resumed TWT
 * @TWT_CLOSED: Deinitialized TWT feature and closed
 */
enum twt_status {
	TWT_INIT,
	TWT_DISABLED,
	TWT_FW_TRIGGER_ENABLE_REQUESTED,
	TWT_FW_TRIGGER_ENABLED,
	TWT_HOST_TRIGGER_ENABLE_REQUESTED,
	TWT_HOST_TRIGGER_ENABLED,
	TWT_DISABLE_REQUESTED,
	TWT_SUSPEND_REQUESTED,
	TWT_SUSPENDED,
	TWT_RESUME_REQUESTED,
	TWT_RESUMED,
	TWT_CLOSED,
};

/**
 * hdd_twt_print_ini_config() - Print TWT INI config items
 * @hdd_ctx: HDD Context
 *
 * Return: None
 */
void hdd_twt_print_ini_config(struct hdd_context *hdd_ctx);

/**
 * hdd_update_tgt_twt_cap() - Update TWT target capabilities
 * @hdd_ctx: HDD Context
 * @cfg: Pointer to target configuration
 *
 * Return: None
 */
void hdd_update_tgt_twt_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg);

/**
 * hdd_send_twt_enable_cmd() - Send TWT enable command to target
 * @hdd_ctx: HDD Context
 *
 * Return: None
 */
void hdd_send_twt_enable_cmd(struct hdd_context *hdd_ctx);

/**
 * hdd_twt_enable_comp_cb() - TWT enable complete event callback
 * @hdd_ctx: pointer to global HDD Context
 * @twt_event: TWT event data received from the target
 *
 * Return: None
 */
void hdd_twt_enable_comp_cb(void *hdd_ctx,
			    struct wmi_twt_enable_complete_event_param *params);

/**
 * hdd_twt_disable_comp_cb() - TWT disable complete event callback
 * @hdd_ctx: pointer to global HDD Context
 *
 * Return: None
 */
void hdd_twt_disable_comp_cb(void *hdd_ctx);

/**
 * wlan_hdd_twt_init() - Initialize TWT
 * @hdd_ctx: pointer to global HDD Context
 *
 * Initialize the TWT feature by registering the callbacks
 * with the lower layers.
 *
 * Return: None
 */
void wlan_hdd_twt_init(struct hdd_context *hdd_ctx);

/**
 * wlan_hdd_twt_deinit() - Deinitialize TWT
 * @hdd_ctx: pointer to global HDD Context
 *
 * Deinitialize the TWT feature by deregistering the
 * callbacks with the lower layers.
 *
 * Return: None
 */
void wlan_hdd_twt_deinit(struct hdd_context *hdd_ctx);

#else
static inline void hdd_twt_print_ini_config(struct hdd_context *hdd_ctx)
{
}

static inline void hdd_update_tgt_twt_cap(struct hdd_context *hdd_ctx,
					  struct wma_tgt_cfg *cfg)
{
}

static inline void hdd_send_twt_enable_cmd(struct hdd_context *hdd_ctx)
{
}

static inline void hdd_twt_enable_comp_cb(void *hdd_ctx,
			  struct wmi_twt_enable_complete_event_param *params)
{
}

static inline void hdd_twt_disable_comp_cb(void *hdd_ctx)
{
}

static inline void wlan_hdd_twt_init(struct hdd_context *hdd_ctx)
{
}

static inline void wlan_hdd_twt_deinit(struct hdd_context *hdd_ctx)
{
}

#endif
#endif /* if !defined(WLAN_HDD_TWT_H)*/

/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
#include "qca_vendor.h"
#include <net/cfg80211.h>

struct hdd_context;
struct hdd_adapter;
struct wma_tgt_cfg;
struct wmi_twt_add_dialog_param;
struct wmi_twt_del_dialog_param;
struct wmi_twt_pause_dialog_cmd_param;
struct wmi_twt_resume_dialog_cmd_param;

extern const struct nla_policy
wlan_hdd_wifi_twt_config_policy[QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX + 1];

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
 * wlan_hdd_cfg80211_wifi_twt_config() - Wifi twt configuration
 * vendor command
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT
 *
 * Return: 0 for success, negative errno for failure.
 */
int wlan_hdd_cfg80211_wifi_twt_config(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data,
				      int data_len);

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
 * hdd_send_twt_disable_cmd() - Send TWT disable command to target
 * @hdd_ctx: HDD Context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_send_twt_disable_cmd(struct hdd_context *hdd_ctx);

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

/**
 * hdd_test_config_twt_setup_session() - Process TWT setup
 * operation in the received test config vendor command and
 * send it to firmare
 * @adapter: adapter pointer
 * @tb: nl attributes
 *
 * Handles QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_SETUP
 *
 * Return: 0 for Success and negative value for failure
 */
int hdd_test_config_twt_setup_session(struct hdd_adapter *adapter,
				      struct nlattr **tb);

/**
 * hdd_test_config_twt_terminate_session() - Process TWT terminate
 * operation in the received test config vendor command and send
 * it to firmare
 * @adapter: adapter pointer
 * @tb: nl attributes
 *
 * Handles QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_TWT_TERMINATE
 *
 * Return: 0 for Success and negative value for failure
 */
int hdd_test_config_twt_terminate_session(struct hdd_adapter *adapter,
					  struct nlattr **tb);

#define FEATURE_VENDOR_SUBCMD_WIFI_CONFIG_TWT                            \
{                                                                        \
	.info.vendor_id = QCA_NL80211_VENDOR_ID,                         \
	.info.subcmd =                                                   \
		QCA_NL80211_VENDOR_SUBCMD_CONFIG_TWT,                    \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |                            \
		WIPHY_VENDOR_CMD_NEED_NETDEV |                           \
		WIPHY_VENDOR_CMD_NEED_RUNNING,                           \
	.doit = wlan_hdd_cfg80211_wifi_twt_config,                       \
	vendor_command_policy(wlan_hdd_wifi_twt_config_policy,           \
			      QCA_WLAN_VENDOR_ATTR_CONFIG_TWT_MAX)       \
},

#else
static inline void hdd_update_tgt_twt_cap(struct hdd_context *hdd_ctx,
					  struct wma_tgt_cfg *cfg)
{
}

static inline void hdd_send_twt_enable_cmd(struct hdd_context *hdd_ctx)
{
}

static inline QDF_STATUS hdd_send_twt_disable_cmd(struct hdd_context *hdd_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline void wlan_hdd_twt_init(struct hdd_context *hdd_ctx)
{
}

static inline void wlan_hdd_twt_deinit(struct hdd_context *hdd_ctx)
{
}

static inline
int hdd_test_config_twt_setup_session(struct hdd_adapter *adapter,
				      struct nlattr **tb)
{
	return -EINVAL;
}

static inline
int hdd_test_config_twt_terminate_session(struct hdd_adapter *adapter,
					  struct nlattr **tb)
{
	return -EINVAL;
}

#define FEATURE_VENDOR_SUBCMD_WIFI_CONFIG_TWT

#endif

#endif /* if !defined(WLAN_HDD_TWT_H)*/

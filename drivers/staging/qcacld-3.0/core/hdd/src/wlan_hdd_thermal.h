/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __HDD_THERMAL_H
#define __HDD_THERMAL_H
/**
 * DOC: wlan_hdd_thermal.h
 * WLAN Host Device Driver thermal mitigation include file
 */

#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>


/**
 * enum hdd_thermal_states   - The various thermal states as supported by WLAN
 * @HDD_THERMAL_STATE_NONE   - The normal working state
 * @HDD_THERMAL_STATE_LIGHT  - Intermediate states, WLAN must perform partial
 *                             mitigation
 * @HDD_THERMAL_STATE_MODERATE - Intermediate states, WLAN must perform partial
 *                               mitigation
 * @HDD_THERMAL_STATE_SEVERE - Intermediate states, WLAN must perform partial
 *                             mitigation
 * @HDD_THERMAL_STATE_CRITICAL - Intermediate states, WLAN must perform partial
 *                               mitigation
 * @HDD_THERMAL_STATE_EMERGENCY - The highest state, WLAN must enter forced
 *                                IMPS and will disconnect any active STA
 *                                connection
 */
enum hdd_thermal_states {
	HDD_THERMAL_STATE_NONE = 0,
	HDD_THERMAL_STATE_LIGHT = 1,
	HDD_THERMAL_STATE_MODERATE = 2,
	HDD_THERMAL_STATE_SEVERE = 3,
	HDD_THERMAL_STATE_CRITICAL = 4,
	HDD_THERMAL_STATE_EMERGENCY = 5,
	HDD_THERMAL_STATE_INVAL = 0xFF,
};

/*
 * thermal_monitor_id: enum of thermal client
 * @THERMAL_MONITOR_APPS: Thermal monitor client of APPS
 * @THERMAL_MONITOR_WPSS: Thermal monitor client for WPSS
 */
enum thermal_monitor_id {
	THERMAL_MONITOR_APPS = 1,
	THERMAL_MONITOR_WPSS,
};

#ifdef FW_THERMAL_THROTTLE_SUPPORT
int
wlan_hdd_cfg80211_set_thermal_mitigation_policy(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len);

/**
 * wlan_hdd_thermal_config_support() - thermal mitigation support
 *
 * Return: true if thermal mitigation support enabled otherwise false
 */
bool wlan_hdd_thermal_config_support(void);

/**
 * hdd_restore_thermal_mitigation_config - Restore the saved thermal config
 * @hdd_ctx: HDD context
 *
 * Restore the thermal mitigation config afetr SSR.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_restore_thermal_mitigation_config(struct hdd_context *hdd_ctx);

extern const struct nla_policy
	wlan_hdd_thermal_mitigation_policy
	[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX + 1];

#define FEATURE_THERMAL_VENDOR_COMMANDS                             \
{                                                                   \
	.info.vendor_id = QCA_NL80211_VENDOR_ID,                    \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD,       \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV,                        \
	.doit = wlan_hdd_cfg80211_set_thermal_mitigation_policy,    \
	vendor_command_policy(wlan_hdd_thermal_mitigation_policy,   \
			      QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX) \
},

#define FEATURE_THERMAL_VENDOR_EVENTS                               \
[QCA_NL80211_VENDOR_SUBCMD_THERMAL_INDEX] = {                       \
	.vendor_id = QCA_NL80211_VENDOR_ID,                         \
	.subcmd = QCA_NL80211_VENDOR_SUBCMD_THERMAL_EVENT,          \
},

/**
 * hdd_thermal_mitigation_register() - Register for platform specific thermal
 *                                     mitigation support
 * @hdd_ctx: Pointer to Hdd context
 * @dev: Pointer to the device
 *
 * Register to the platform specific thermal mitigation support
 * Return: None
 */
void hdd_thermal_mitigation_register(struct hdd_context *hdd_ctx,
				     struct device *dev);

/**
 * hdd_thermal_mitigation_unregister() - Unregister for platform specific
 *                                       thermal mitigation support
 * @hdd_ctx: Pointer to Hdd context
 * @dev: Pointer to the device
 *
 * Unregister to the platform specific thermal mitigation support
 * Return: None
 */
void hdd_thermal_mitigation_unregister(struct hdd_context *hdd_ctx,
				       struct device *dev);

/**
 * wlan_hdd_pld_set_thermal_mitigation() - send the suggested thermal value
 *                                         to the firmware
 * @dev: Pointer to the device
 * @state: Thermal state to set
 * @mon_id: Thermal monitor id ie.. apps or wpss
 *
 * Send the requested thermal mitigation value to the firmware * for the
 * requested thermal monitor id.
 *
 * Return: 0 for success or errno for failure.
 */
int wlan_hdd_pld_set_thermal_mitigation(struct device *dev,
					unsigned long state, int mon_id);
#ifdef FEATURE_WPSS_THERMAL_MITIGATION
/**
 * hdd_thermal_fill_clientid_priority() - fill the client id/priority
 * @hdd_ctx: pointer to hdd contex structure
 * @mon_id: Thermal monitor id ie.. apps or wpss
 * @priority_apps: Priority of the apps client to be considered
 * @priority_wpps: Priority of the wpps client to be considered
 * @params: pointer to thermal mitigation parameters
 *
 * Fill the clientid/priority for the firmwaire to consider.
 *
 * Return: none
 */
void
hdd_thermal_fill_clientid_priority(struct hdd_context *hdd_ctx, uint8_t mon_id,
				   uint8_t priority_apps, uint8_t priority_wpps,
				   struct thermal_mitigation_params *params);
#else
static inline void
hdd_thermal_fill_clientid_priority(struct hdd_context *hdd_ctx, uint8_t mon_id,
				   uint8_t priority_apps, uint8_t priority_wpps,
				   struct thermal_mitigation_params *params)
{
}
#endif

/**
 * hdd_thermal_register_callbacks() - register thermal event callback
 *  to be called by fwol thermal layer
 * @hdd_ctx: hdd context
 *
 * The callback will be invoked by fwol thermal layer when the target
 * indicate thermal throttle level changed. Host will report the new
 * level to upper layer by vendor command event.
 *
 * Return: none
 */
void hdd_thermal_register_callbacks(struct hdd_context *hdd_ctx);

/**
 * hdd_thermal_unregister_callbacks() - unregister thermal event callback
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void hdd_thermal_unregister_callbacks(struct hdd_context *hdd_ctx);
#else
#define FEATURE_THERMAL_VENDOR_COMMANDS
#define FEATURE_THERMAL_VENDOR_EVENTS

static inline bool wlan_hdd_thermal_config_support(void)
{
	return false;
}

static inline
QDF_STATUS hdd_restore_thermal_mitigation_config(struct hdd_context *hdd_ctx)
{
	return false;
}

static inline
void hdd_thermal_mitigation_register(struct hdd_context *hdd_ctx,
				     struct device *dev)
{
}

static inline
void hdd_thermal_mitigation_unregister(struct hdd_context *hdd_ctx,
				       struct device *dev)
{
}

static inline
int wlan_hdd_pld_set_thermal_mitigation(struct device *dev,
					unsigned long state, int mon_id)
{
	return 0;
}

static inline void
hdd_thermal_register_callbacks(struct hdd_context *hdd_ctx)
{
}

static inline void
hdd_thermal_unregister_callbacks(struct hdd_context *hdd_ctx)
{
}
#endif /* FEATURE_THERMAL_VENDOR_COMMANDS */

#ifdef THERMAL_STATS_SUPPORT
QDF_STATUS
hdd_send_get_thermal_stats_cmd(struct hdd_context *hdd_ctx,
			       enum thermal_stats_request_type request_type,
			       void (*callback)(void *context,
			       struct thermal_throttle_info *response),
			       void *context);
#endif /* THERMAL_STATS_SUPPORT */
#endif /* __HDD_THERMAL_H */

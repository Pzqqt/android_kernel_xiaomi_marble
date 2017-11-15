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
 * DOC: declares driver FTM functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_FTM_H_
#define _WLAN_CFG80211_FTM_H_

/**
 * enum wlan_cfg80211_ftm_attr - FTM Netlink attributes
 * @WLAN_CFG80211_FTM_ATTR_INVALID: attribute is invalid
 * @WLAN_CFG80211_FTM_ATTR_CMD: attribute type is FTM command
 * @WLAN_CFG80211_FTM_ATTR_DATA: attribute type is data
 *
 * @WLAN_CFG80211_FTM_ATTR_MAX: Max number of attributes
 */
enum wlan_cfg80211_ftm_attr {
	WLAN_CFG80211_FTM_ATTR_INVALID = 0,
	WLAN_CFG80211_FTM_ATTR_CMD = 1,
	WLAN_CFG80211_FTM_ATTR_DATA = 2,

	/* keep last */
	WLAN_CFG80211_FTM_ATTR_MAX,
};

/**
 * enum wlan_cfg80211_ftm_cmd - FTM command types
 * @WLAN_CFG80211_FTM_CMD_WLAN_FTM: command is of type FTM
 */
enum wlan_cfg80211_ftm_cmd {
	WLAN_CFG80211_FTM_CMD_WLAN_FTM = 0,
};

#define WLAN_FTM_DATA_MAX_LEN 2048

/**
 * wlan_cfg80211_ftm_testmode_cmd() - process cfg80211 testmode command
 * @pdev: pdev object
 * @data: ftm testmode command data of type void
 * @len: length of the data
 *
 * Return: 0 on success or -Eerrno otherwise
 */
int wlan_cfg80211_ftm_testmode_cmd(struct wlan_objmgr_pdev *pdev,
				void *data, uint32_t len);

/**
 * wlan_cfg80211_ftm_rx_event() - handle the received ftm event
 * @pdev: pdev object
 * @data: ftm event data
 * @len: length of the data
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS_E errno otherwise
 */
QDF_STATUS wlan_cfg80211_ftm_rx_event(struct wlan_objmgr_pdev *pdev,
					uint8_t *data, uint32_t len);

#endif

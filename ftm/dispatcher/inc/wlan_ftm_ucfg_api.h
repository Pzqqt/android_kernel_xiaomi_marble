/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: declare the ftm service data structure and apis
 */
#ifndef _WLAN_FTM_UCFG_API_H_
#define _WLAN_FTM_UCFG_API_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_cmn.h>

#define FTM_DEBUG       0

#if FTM_DEBUG
#define ftm_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_FTM, level, ## args)
#define ftm_logfl(level, format, args...) \
	ftm_log(level, FL(format), ## args)

#define ftm_alert(format, args...) \
	ftm_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define ftm_err(format, args...) \
	ftm_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define ftm_warn(format, args...) \
	ftm_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define ftm_notice(format, args...) \
	ftm_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define ftm_debug(format, args...) \
	ftm_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)
#else
#define ftm_alert(format, args...)
#define ftm_err(format, args...)
#define ftm_warn(format, args...)
#define ftm_notice(format, args...)
#define ftm_debug(format, args...)
#endif

#define FTM_IOCTL_UNIFIED_UTF_CMD		0x1000
#define FTM_IOCTL_UNIFIED_UTF_RSP		0x1001
#define FTM_CMD_MAX_BUF_LENGTH		    2048

/**
 * enum wifi_ftm_cmd_type - the enumeration of the command source per pdev
 * @WIFI_FTM_CMD_IOCTL: command from ioctl on the pdev
 * @WIFI_FTM_CMD_NL80211: command from nl80211 on the pdev
 *
 */
enum wifi_ftm_pdev_cmd_type {
	WIFI_FTM_CMD_IOCTL = 1,
	WIFI_FTM_CMD_NL80211,

	/* command should be added above */
	WIFI_FTM_CMD_UNKNOWN,
};

/**
 * struct wifi_ftm_pdev_priv_obj - wifi ftm pdev utf event info
 * @pdev: pointer to pdev
 * @data: data ptr
 * @current_seq: curent squence
 * @expected_seq: expected sequence
 * @length: length
 * @offset: offset
 * @cmd_type: command type from either ioctl or nl80211
 */
struct wifi_ftm_pdev_priv_obj {
	struct wlan_objmgr_pdev *pdev;
	uint8_t *data;
	uint8_t current_seq;
	uint8_t expected_seq;
	qdf_size_t length;
	qdf_size_t offset;
	enum wifi_ftm_pdev_cmd_type cmd_type;
};

/**
 * wlan_ftm_testmode_cmd() - handle FTM testmode command
 * @pdev: pdev pointer
 * @data: data
 * @len: data length
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS ucfg_wlan_ftm_testmode_cmd(struct wlan_objmgr_pdev *pdev,
					uint8_t *data, uint32_t len);

/**
 * wlan_ftm_testmode_rsp() - handle FTM testmode command
 * @pdev: pdev pointer
 * @data: data
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS ucfg_wlan_ftm_testmode_rsp(struct wlan_objmgr_pdev *pdev,
					uint8_t *data);

/**
 * wlan_ftm_process_utf_event() - process ftm UTF event
 * @scn_handle: scn handle
 * @event: event buffer
 * @len: event length
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wlan_ftm_process_utf_event(struct wlan_objmgr_pdev *pdev,
					uint8_t *event_buf, uint32_t len);
#endif /* _WLAN_FTM_UCFG_API_H_ */

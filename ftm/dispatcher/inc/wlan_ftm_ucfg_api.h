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
#define ftm_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_FTM, params)
#define ftm_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_FTM, params)
#define ftm_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_FTM, params)
#define ftm_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_FTM, params)
#define ftm_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_FTM, params)

#define ftm_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_FTM, params)
#define ftm_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_FTM, params)
#define ftm_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_FTM, params)
#define ftm_nofl_notice(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_FTM, params)
#define ftm_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_FTM, params)

#else
#define ftm_alert(params...)
#define ftm_err(params...)
#define ftm_warn(params...)
#define ftm_notice(params...)
#define ftm_debug(params...)

#define ftm_nofl_alert(params...)
#define ftm_nofl_err(params...)
#define ftm_nofl_warn(params...)
#define ftm_nofl_notice(params...)
#define ftm_nofl_debug(params...)
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

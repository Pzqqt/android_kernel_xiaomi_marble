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

#ifndef _FW_DBGLOG_API_H_
#define _FW_DBGLOG_API_H_

#include "target_if.h"

/**
 * fwdbg_set_log_lvl() - API to set debug log level
 * @dbg_handle: Debug module handle
 * @scn: scn handle
 * @log_lvl: value of log level
 *
 * Send wmi configuration command to set debug log level.
 *
 * Return: None
 */
void fwdbg_set_log_lvl(struct common_dbglog_handle *dbg_handle, ol_scn_t scn,
		uint32_t log_lvl);

/**
 * fwdbg_fw_handler() - Firmware handler interface
 * @dbg_handle: Debug module handle
 * @sc: soc handle
 * @data: Reference to command data
 * @datalen: length of data
 *
 * Return: 0 success
 */
int fwdbg_fw_handler(struct common_dbglog_handle *dbg_handle, ol_scn_t sc,
			uint8_t *data, uint32_t datalen);

/**
 * fwdbg_parse_debug_logs() - API to parse firmware debug logs
 * @dbg_handle: Debug module handle
 * @name: device name
 * @datap: Reference to log data
 * @len: length of data
 * @context: log context
 *
 * API parse firmware debug log messages and prints to console.
 *
 * Return: 0 success
 */
int fwdbg_parse_debug_logs(struct common_dbglog_handle *dbg_handle,
				const char *name, uint8_t *datap,
				uint16_t len, void *context);

/**
 * fwdbg_ratelimit_set() - API to set rate limit
 * @dbg_handle: Debug module handle
 * @burst_limit: burst limit
 *
 * Return: None
 */
void fwdbg_ratelimit_set(struct common_dbglog_handle *dbg_handle,
			uint32_t burst_limit);

/**
 * fwdbg_vap_log_enable() - API to Enable/Disable the logging for VAP
 * @dbg_handle: Debug module handle
 * @scn: scn handle
 * @vap_id: VAP id
 * @isenable: Enable/disable
 *
 * API allows to enable or disable debuglogs at VAP level. It encodes wmi
 * config command based on VAP id and sends wmi command to firmware to
 * enable/disable debuglog.
 *
 * Return: None
 */
void fwdbg_vap_log_enable(struct common_dbglog_handle *dbg_handle, ol_scn_t scn,
				uint16_t vap_id, bool isenable);

/**
 * fwdbg_set_timestamp_resolution - Set the resolution for time stamp
 * @dbg_handle: Debug module handle
 * @scn: scn handle
 * @tsr: time stamp resolution
 *
 * Set the resolution for time stamp in debug logs. It encodes wmi
 * config command to desired timestamp resolution and sends wmi command to
 * firmware.
 *
 * Return: None
 */
void fwdbg_set_timestamp_resolution(struct common_dbglog_handle *dbg_handle,
				ol_scn_t scn, uint16_t tsr);

/**
 * fwdbg_reporting_enable() - Enable reporting.
 * @dbg_handle: Debug module handle
 * @scn: scn handle
 * @isenable: Enable/disable
 *
 * API to enable debug information reporting. It encodes wmi config command
 * to enable reporting. If set to false then Target wont deliver any debug
 * information.
 *
 * Return: None
 */
void fwdbg_reporting_enable(struct common_dbglog_handle *dbg_handle,
				ol_scn_t scn, bool isenable);

/**
 * fwdbg_module_log_enable() - Enable/Disable logging for Module.
 * @dbg_handle: Debug module handle
 * @scn: scn handle
 * @mod_id: Module id
 * @isenable: Enable/disable
 *
 * API allows to enable or disable debuglogs per module. It encodes wmi
 * config command based on module id and sends wmi command to firmware to
 * enable/disable debuglog for that module.
 *
 * Return: None
 */
void fwdbg_module_log_enable(struct common_dbglog_handle *dbg_handle,
			ol_scn_t scn, uint32_t mod_id, bool isenable);

/**
 * fwdbg_init() - Initialize debuglog.
 * @dbg_handle: Debug module handle
 * @soc: soc handle
 *
 * It initializes debuglog print function for set of modules and
 * initializes WMI event handler for debuglog message event.
 *
 * Return: None
 */
void fwdbg_init(struct common_dbglog_handle *dbg_handle, void *soc);

/**
 * fwdbg_free() - Free debug handler.
 * @dbg_handle: Debug module handle
 * @soc: soc handle
 *
 * Return: None
 */
void fwdbg_free(struct common_dbglog_handle *dbg_handle, void *soc);

/**
 * fwdbg_set_report_size() - set the size of the report size
 * @dbg_handle: Debug module handle
 * @scn: soc handler
 * @size: Report size
 *
 * Set the debug log report size. It encodes wmi config command to
 * desired report size and sends wmi command to firmware.
 *
 * Return: None
 */
void fwdbg_set_report_size(struct common_dbglog_handle *dbg_handle,
				ol_scn_t scn, uint16_t size);

#endif /* _FW_DBGLOG_API_H_ */

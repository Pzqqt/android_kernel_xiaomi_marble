/*
 * Copyright (c) 2011, 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef _DBGLOG_HOST_H_
#define _DBGLOG_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dbglog_common.h"
#include "ol_defines.h"

/*
 * set the dbglog parser type
 */int
dbglog_parser_type_init(wmi_unified_t wmi_handle, int type);

/** dbglog_int - Registers a WMI event handle for WMI_DBGMSG_EVENT
 * @brief wmi_handle - handle to wmi module
 */
int
dbglog_init(wmi_unified_t wmi_handle);

/** dbglog_deinit - UnRegisters a WMI event handle for WMI_DBGMSG_EVENT
 * @brief wmi_handle - handle to wmi module
 */
int
dbglog_deinit(wmi_unified_t wmi_handle);

/** set the size of the report size
 * @brief wmi_handle - handle to Wmi module
 * @brief size - Report size
 */
int
dbglog_set_report_size(wmi_unified_t wmi_handle, A_UINT16 size);

/** Set the resolution for time stamp
 * @brief wmi_handle - handle to Wmi module
 * @ brief tsr - time stamp resolution
 */
int
dbglog_set_timestamp_resolution(wmi_unified_t wmi_handle,
				A_UINT16 tsr);

/** Enable reporting. If it is set to false then Traget wont deliver
 * any debug information
 */
int
dbglog_report_enable(wmi_unified_t wmi_handle, A_BOOL isenable);

/** Set the log level
 * @brief DBGLOG_INFO - Information lowest log level
 * @brief DBGLOG_WARNING
 * @brief DBGLOG_ERROR - default log level
 */
int
dbglog_set_log_lvl(wmi_unified_t wmi_handle, DBGLOG_LOG_LVL log_lvl);

/*
 * set the debug log level for a given module
 *  mod_id_lvl : the format is more user friendly.
 *    module_id =  mod_id_lvl/10;
 *    log_level =  mod_id_lvl%10;
 * example : mod_id_lvl is 153. then module id is 15 and log level is 3. this format allows
 *         user to pass a sinlge value (which is the most convenient way for most of the OSs)
 *         to be passed from user to the driver.
 */
int
dbglog_set_mod_log_lvl(wmi_unified_t wmi_handle, A_UINT32 mod_id_lvl);

/** Enable/Disable the logging for VAP */
int
dbglog_vap_log_enable(wmi_unified_t wmi_handle, A_UINT16 vap_id,
		      A_BOOL isenable);
/** Enable/Disable logging for Module */
int
dbglog_module_log_enable(wmi_unified_t wmi_handle, A_UINT32 mod_id,
			 A_BOOL isenable);

/** set vap enablie bitmap */
void
dbglog_set_vap_enable_bitmap(wmi_unified_t wmi_handle,
			     A_UINT32 vap_enable_bitmap);

/** set log level for all the modules specified in the bitmap. for all other modules
 * with 0 in the bitmap (or) outside the bitmap , the log level be reset to DBGLOG_ERR.
 */
void
dbglog_set_mod_enable_bitmap(wmi_unified_t wmi_handle,
			     A_UINT32 log_level,
			     A_UINT32 *mod_enable_bitmap,
			     A_UINT32 bitmap_len);

int
dbglog_parse_debug_logs(ol_scn_t scn, u_int8_t *datap,
					u_int32_t len);


/** Register the cnss_diag activate with the wlan driver */
int cnss_diag_activate_service(void);

#ifdef __cplusplus
}
#endif

#endif /* _DBGLOG_HOST_H_ */

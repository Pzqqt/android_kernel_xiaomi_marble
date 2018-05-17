/*
 * Copyright (c) 2011, 2014-2018 The Linux Foundation. All rights reserved.
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

#ifndef _DBGLOG_HOST_H_
#define _DBGLOG_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dbglog_common.h"
#include "wmi_unified_param.h"

#define DIAG_FWID_OFFSET            24
#define DIAG_FWID_MASK              0xFF000000  /* Bit 24-31 */

#define DIAG_TIMESTAMP_OFFSET       0
#define DIAG_TIMESTAMP_MASK         0x00FFFFFF  /* Bit 0-23 */

#define DIAG_ID_OFFSET              16
#define DIAG_ID_MASK                0xFFFF0000  /* Bit 16-31 */

#define DIAG_VDEVID_OFFSET          11
#define DIAG_VDEVID_MASK            0x0000F800  /* Bit 11-15 */
#define DIAG_VDEVID_NUM_MAX         16

#define DIAG_VDEVLEVEL_OFFSET       8
#define DIAG_VDEVLEVEL_MASK         0x00000700  /* Bit 8-10 */

#define DIAG_PAYLEN_OFFSET          0
#define DIAG_PAYLEN_MASK            0x000000FF  /* Bit 0-7 */

#define DIAG_PAYLEN_OFFSET16        0
#define DIAG_PAYLEN_MASK16          0x0000FFFF  /* Bit 0-16 */

#define DIAG_GET_TYPE(arg) \
	((arg & DIAG_FWID_MASK) >> DIAG_FWID_OFFSET)

#define DIAG_GET_TIME_STAMP(arg) \
	((arg & DIAG_TIMESTAMP_MASK) >> DIAG_TIMESTAMP_OFFSET)

#define DIAG_GET_ID(arg) \
	((arg & DIAG_ID_MASK) >> DIAG_ID_OFFSET)

#define DIAG_GET_VDEVID(arg) \
	((arg & DIAG_VDEVID_MASK) >> DIAG_VDEVID_OFFSET)

#define DIAG_GET_VDEVLEVEL(arg)	\
	((arg & DIAG_VDEVLEVEL_MASK) >> DIAG_VDEVLEVEL_OFFSET)

#define DIAG_GET_PAYLEN(arg) \
	((arg & DIAG_PAYLEN_MASK) >> DIAG_PAYLEN_OFFSET)

#define DIAG_GET_PAYLEN16(arg) \
	((arg & DIAG_PAYLEN_MASK16) >> DIAG_PAYLEN_OFFSET16)

#ifdef FEATURE_FW_LOG_PARSING
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
dbglog_set_report_size(wmi_unified_t wmi_handle, uint16_t size);

/** Set the resolution for time stamp
 * @brief wmi_handle - handle to Wmi module
 * @ brief tsr - time stamp resolution
 */
int
dbglog_set_timestamp_resolution(wmi_unified_t wmi_handle,
				uint16_t tsr);

/** Enable reporting. If it is set to false then Traget wont deliver
 * any debug information
 */
int
dbglog_report_enable(wmi_unified_t wmi_handle, A_BOOL isenable);

#ifdef CONFIG_MCL
/*
 * enum DBGLOG_LOG_LVL is not converged between WIN and MCL.
 * So this function declaration needs to be disabled from WIN side.
 */

/** Set the log level
 * @brief DBGLOG_INFO - Information lowest log level
 * @brief DBGLOG_WARNING
 * @brief DBGLOG_ERROR - default log level
 */
int
dbglog_set_log_lvl(wmi_unified_t wmi_handle, DBGLOG_LOG_LVL log_lvl);
#endif

/*
 * set the debug log level for a given module
 *  mod_id_lvl : the format is more user friendly.
 *    module_id =  mod_id_lvl/10;
 *    log_level =  mod_id_lvl%10;
 * example : mod_id_lvl is 153. then module id is 15 and log level is 3.
 *           this format allows user to pass a sinlge value
 *           (which is the most convenient way for most of the OSs)
 *           to be passed from user to the driver.
 */
int
dbglog_set_mod_log_lvl(wmi_unified_t wmi_handle, uint32_t mod_id_lvl);

/** Enable/Disable the logging for VAP */
int
dbglog_vap_log_enable(wmi_unified_t wmi_handle, uint16_t vap_id,
		      A_BOOL isenable);
/** Enable/Disable logging for Module */
int
dbglog_module_log_enable(wmi_unified_t wmi_handle, uint32_t mod_id,
			 A_BOOL isenable);

/** set vap enablie bitmap */
void
dbglog_set_vap_enable_bitmap(wmi_unified_t wmi_handle,
			     uint32_t vap_enable_bitmap);

/** set log level for all the modules specified in the bitmap.
 *  for all other modules with 0 in the bitmap (or) outside the bitmap,
 *  the log level be reset to DBGLOG_ERR.
 */
void
dbglog_set_mod_enable_bitmap(wmi_unified_t wmi_handle,
			     uint32_t log_level,
			     uint32_t *mod_enable_bitmap,
			     uint32_t bitmap_len);

int
dbglog_parse_debug_logs(ol_scn_t scn, u_int8_t *datap,
					u_int32_t len);

/**
 * cnss_diag_activate_service() - API to register CNSS diag cmd handler
 *
 * API to register the handler for the NL message received from cnss_diag
 * application.
 *
 * Return: 0
 */
int cnss_diag_activate_service(void);

/**
 * cnss_diag_deactivate_service() - API to deregister CNSS diag cmd handler
 *
 * API to deregister the handler for the NL message received from cnss_diag
 * application.
 *
 * Return: 0
 */
int cnss_diag_deactivate_service(void);

#else
static inline int
dbglog_parser_type_init(wmi_unified_t wmi_handle, int type)
{
	return A_OK;
}

static inline int
dbglog_init(wmi_unified_t wmi_handle)
{
	return A_OK;
}

static inline int
dbglog_deinit(wmi_unified_t wmi_handle)
{
	return A_OK;
}

static inline int
dbglog_report_enable(wmi_unified_t wmi_handle, A_BOOL isenable)
{
	return A_OK;
}

#ifdef CONFIG_MCL
static inline int
dbglog_set_log_lvl(wmi_unified_t wmi_handle, DBGLOG_LOG_LVL log_lvl)
{
	return A_OK;
}
#endif

static inline int cnss_diag_activate_service(void)
{
	return A_OK;
}

static inline int cnss_diag_deactivate_service(void)
{
	return A_OK;
}

static inline int
dbglog_module_log_enable(wmi_unified_t wmi_handle, uint32_t mod_id,
			 A_BOOL isenable)
{
	return A_OK;
}

static inline int
dbglog_vap_log_enable(wmi_unified_t wmi_handle, uint16_t vap_id,
		      A_BOOL isenable)
{
	return A_OK;
}

static inline int
dbglog_set_mod_log_lvl(wmi_unified_t wmi_handle, uint32_t mod_id_lvl)
{
	return A_OK;
}
#endif /* FEATURE_FW_LOG_PARSING */

#ifdef __cplusplus
}
#endif

#endif /* _DBGLOG_HOST_H_ */

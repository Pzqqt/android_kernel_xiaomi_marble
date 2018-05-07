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

#include "fw_dbglog_api.h"
#include "fw_dbglog_priv.h"

static inline struct dbglog_info *handle2info(
		struct common_dbglog_handle *dbg_handle)
{
	return (struct dbglog_info *)dbg_handle;
}

void fwdbg_set_log_lvl(struct common_dbglog_handle *dbg_handle, ol_scn_t scn,
			uint32_t log_lvl)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_set_log_lvl)
		dbg_info->ops->dbglog_set_log_lvl(scn, log_lvl);

}

int fwdbg_fw_handler(struct common_dbglog_handle *dbg_handle, ol_scn_t soc,
			uint8_t *data, uint32_t datalen)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_fw_handler)
		return dbg_info->ops->dbglog_fw_handler(soc, data, datalen);

	return 0;
}

int fwdbg_parse_debug_logs(struct common_dbglog_handle *dbg_handle,
		const char *name, uint8_t *datap, uint16_t len, void *context)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_parse_debug_logs)
		return dbg_info->ops->dbglog_parse_debug_logs(name,
				datap, len, context);

	return 0;
}

void fwdbg_ratelimit_set(struct common_dbglog_handle *dbg_handle,
		uint32_t burst_limit)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_ratelimit_set)
		dbg_info->ops->dbglog_ratelimit_set(burst_limit);

}

void fwdbg_vap_log_enable(struct common_dbglog_handle *dbg_handle, ol_scn_t scn,
				uint16_t vap_id, bool isenable)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_vap_log_enable)
		dbg_info->ops->dbglog_vap_log_enable(scn, vap_id,
					isenable);

}

void fwdbg_set_timestamp_resolution(struct common_dbglog_handle *dbg_handle,
			ol_scn_t scn, uint16_t tsr)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_set_timestamp_resolution)
		dbg_info->ops->dbglog_set_timestamp_resolution(scn, tsr);

}

void fwdbg_reporting_enable(struct common_dbglog_handle *dbg_handle,
			ol_scn_t scn, bool isenable)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_reporting_enable)
		dbg_info->ops->dbglog_reporting_enable(scn, isenable);

}

void fwdbg_module_log_enable(struct common_dbglog_handle *dbg_handle,
			ol_scn_t scn, uint32_t mod_id, bool isenable)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_module_log_enable)
		dbg_info->ops->dbglog_module_log_enable(scn, mod_id,
							isenable);

}

void fwdbg_init(struct common_dbglog_handle *dbg_handle, void *soc)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_init)
		dbg_info->ops->dbglog_init(soc);

}

void fwdbg_free(struct common_dbglog_handle *dbg_handle, void *soc)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_free)
		dbg_info->ops->dbglog_free(soc);

}

void fwdbg_set_report_size(struct common_dbglog_handle *dbg_handle,
			ol_scn_t scn, uint16_t size)
{
	struct dbglog_info *dbg_info = handle2info(dbg_handle);

	if (dbg_info->ops->dbglog_set_report_size)
		dbg_info->ops->dbglog_set_report_size(scn, size);

}


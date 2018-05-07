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

/*
 * This file contains the API definitions for the Unified Wireless
 * Module Interface (WMI).
 */
#ifndef _FW_DBGLOG_PRIV_H_
#define _FW_DBGLOG_PRIV_H_

#include <wmi_unified_api.h>

struct dbglog_ops {

void (*dbglog_set_log_lvl)(ol_scn_t scn, uint32_t log_lvl);
int (*dbglog_fw_handler)(ol_scn_t soc, uint8_t *data, uint32_t datalen);
int (*dbglog_parse_debug_logs)(const char *name,
			u_int8_t *datap, uint16_t len, void *context);
void (*dbglog_ratelimit_set)(uint32_t burst_limit);
void (*dbglog_vap_log_enable)(ol_scn_t soc, uint16_t vap_id,
				bool isenable);
void (*dbglog_set_timestamp_resolution)(ol_scn_t soc, uint16_t tsr);
void (*dbglog_reporting_enable)(ol_scn_t soc, bool isenable);
void (*dbglog_module_log_enable)(ol_scn_t scn,
				uint32_t mod_id, bool isenable);
void (*dbglog_init)(void *scn);
void (*dbglog_set_report_size)(ol_scn_t scn, uint16_t size);
void (*dbglog_free)(void *soc);

};

struct dbglog_info {
	struct dbglog_ops *ops;
};
#endif /*_FW_DBGLOG_PRIV_H_ */

/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: qal_devcfg
 * This file provides OS dependent device config related APIs
 */

#include "qdf_debugfs.h"
#include "qdf_mem.h"
#include "qdf_types.h"
#include "qdf_nbuf.h"
#include "qdf_module.h"
#include "qal_devcfg.h"
#include <net/cfg80211.h>

QDF_STATUS
qal_devcfg_send_response(qdf_nbuf_t cfgbuf)
{
	int ret;

	if (!cfgbuf)
		return QDF_STATUS_E_INVAL;

	ret = cfg80211_vendor_cmd_reply(cfgbuf);

	return qdf_status_from_os_return(ret);
}

qdf_export_symbol(qal_devcfg_send_response);

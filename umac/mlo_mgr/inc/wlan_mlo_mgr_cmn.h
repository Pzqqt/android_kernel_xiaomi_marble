/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains MLO manager public file containing common functionality
 */
#ifndef _WLAN_MLO_MGR_CMN_H_
#define _WLAN_MLO_MGR_CMN_H_

#include <qdf_types.h>
#include <qdf_trace.h>
#include "wlan_mlo_mgr_public_structs.h"

#define mlo_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_alert(format, args...) \
		QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_err(format, args...) \
		QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_warn(format, args...) \
		QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_info(format, args...) \
		QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_debug(format, args...) \
		QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_MLO, format, ## args)
/**
 * mlo_get_link_information() - get partner link information
 * @mld_addr : MLD address
 * @info: partner link information
 *
 * Return: QDF_STATUS
 */
void mlo_get_link_information(struct qdf_mac_addr *mld_addr,
			      struct mlo_link_info *info);
/**
 * is_mlo_all_links_up() - check all the link status in a MLO device
 * @ml_dev: ML device context
 *
 * Return: QDF_STATUS
 */
void is_mlo_all_links_up(struct wlan_mlo_dev_context *ml_dev);

#endif

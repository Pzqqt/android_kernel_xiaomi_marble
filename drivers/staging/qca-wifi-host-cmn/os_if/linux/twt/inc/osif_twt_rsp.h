/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC : osif_twt_rsp.h
 *
 */

#ifndef _OSIF_TWT_RSP_H_
#define _OSIF_TWT_RSP_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * osif_twt_enable_complete_cb() - callback for twt enable complete event
 * @psoc: Pointer to global psoc
 * @event: Pointer to TWT enable dialog complete event structure
 * @context: TWT enable context set during TWT enable request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_twt_enable_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_enable_complete_event_param *event,
			    void *context);
/**
 * osif_twt_disable_complete_cb() - callback for twt disable complete event
 * @psoc: Pointer to global psoc
 * @event: Pointer to TWT disable dialog complete event structure
 * @context: TWT disable context set during TWT disable request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_twt_disable_complete_cb(struct wlan_objmgr_psoc *psoc,
			     struct twt_disable_complete_event_param *event,
			     void *context);
#else
static inline QDF_STATUS
osif_twt_enable_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_enable_complete_event_param *event,
			    void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
osif_twt_disable_complete_cb(struct wlan_objmgr_psoc *psoc,
			     struct twt_disable_complete_event_param *event,
			     void *context)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _OSIF_TWT_RSP_H_ */


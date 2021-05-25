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
 * DOC: Implements CM UTF releted functionalities
 */

#ifndef WLAN_MLME_CM_UTF_H
#define WLAN_MLME_CM_UTF_H

#include <wlan_objmgr_vdev_obj.h>
#include <include/wlan_mlme_cmn.h>

#ifdef FEATURE_CM_UTF_ENABLE
void cm_utf_set_mlme_ops(struct mlme_ext_ops *ext_ops);

QDF_STATUS cm_utf_register_os_if_cb(void);

void cm_utf_attach(struct wlan_objmgr_vdev *vdev);

void cm_utf_detach(struct wlan_objmgr_vdev *vdev);
#else
static inline void cm_utf_set_mlme_ops(struct mlme_ext_ops *ext_ops)
{
}

static inline QDF_STATUS cm_utf_register_os_if_cb(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void cm_utf_attach(struct wlan_objmgr_vdev *vdev)
{
}

static inline void cm_utf_detach(struct wlan_objmgr_vdev *vdev)
{
}
#endif
#endif //WLAN_MLME_CM_UTF_H
